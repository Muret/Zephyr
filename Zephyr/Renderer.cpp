
#include "Renderer.h"
#include "RenderComponent.h"
#include "Mesh.h"
#include "Shader.h"
#include "Material.h"
#include "Texture.h"
#include "Utilities.h"
#include "ResourceManager.h"
#include "Light.h"
#include "d11.h"
#include "Camera.h"
#include "GUI.h"

Renderer *renderer = NULL;

Renderer::Renderer()
{
	scene_to_render = nullptr;

	default_render_shader = new Shader("default_vertex", "default_pixel");

	gbuffer_normal_texture = new Texture  (D3DXVECTOR3( g_screenWidth, g_screenHeight,1), nullptr, DXGI_FORMAT_R8G8B8A8_UNORM , 0);
	gbuffer_specular_texture = new Texture(D3DXVECTOR3( g_screenWidth, g_screenHeight,1), nullptr, DXGI_FORMAT_R8G8B8A8_UNORM , 0);
	gbuffer_albedo_texture = new Texture  (D3DXVECTOR3( g_screenWidth, g_screenHeight,1), nullptr, DXGI_FORMAT_R8G8B8A8_UNORM , 0);

	screen_texture = new Texture(D3DXVECTOR3(g_screenWidth, g_screenHeight, 1), nullptr, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	
	gbuffer_shader = new Shader("default_vertex", "gbuffer_pixel");
	full_deferred_diffuse_lighting_shader = new Shader("direct_vertex_position", "full_deferred_diffuse_lighting");
	
	use_postfx = false;
	camera_ = nullptr;

	lighting_constants_buffer_gpu = CreateConstantBuffer(sizeof(LightingConstantsBuffer));
	mesh_constants_buffer_gpu = CreateConstantBuffer(sizeof(MeshConstantsBuffer));
	frame_constans_buffer_gpu = CreateConstantBuffer(sizeof(FrameConstantsBuffer));

	SetConstantBufferToSlot(0, frame_constans_buffer_gpu);
	SetConstantBufferToSlot(1, mesh_constants_buffer_gpu);
	SetConstantBufferToSlot(2, lighting_constants_buffer_gpu);
}

void Renderer::render_frame()
{
	begin_frame();
	
	if (scene_to_render != nullptr)
	{
		pre_render();

		gbuffer_render();

		main_render();

		post_render();
	}

	gui.render_frame();

	end_frame();
}

void Renderer::pre_render()
{
	for (int i = 0; i < render_components.size(); i++)
	{
		render_components[i]->pre_render();
	}

	set_frame_constant_values();
}

void Renderer::gbuffer_render()
{
	invalidate_srv(shaderType::shader_type_pixel);

	gbuffer_albedo_texture->set_as_render_target(0);
	gbuffer_normal_texture->set_as_render_target(1);
	gbuffer_specular_texture->set_as_render_target(2);
	clearScreen(D3DXVECTOR4(0, 0, 0, 0));
	ClearRenderView(D3DXVECTOR4(0, 0, 0, 0), 1);
	ClearRenderView(D3DXVECTOR4(0, 0, 0, 0),2);

	SetDepthState(depth_state_enable_test_enable_write);
	
	SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	const vector<Mesh*> &meshes_to_render = scene_to_render->get_meshes();

	for (int i = 0; i < meshes_to_render.size(); i++)
	{
		Mesh *mesh_to_render = meshes_to_render[i];

		if (mesh_to_render->get_material())
		{
			mesh_to_render->get_material()->set_textures();
		}
		set_mesh_constant_values(mesh_to_render);

		Shader *shader_to_set = mesh_to_render->get_material() ? mesh_to_render->get_material()->get_enforced_gbuffer_shader() : nullptr;
		if (shader_to_set == nullptr)
		{
			shader_to_set = gbuffer_shader;
		}

		shader_to_set->set_shaders();

		//set buffers
		SetVertexBuffer(mesh_to_render->get_vertex_buffer(), sizeof(Mesh::Vertex));
		SetIndexBuffer(mesh_to_render->get_index_buffer());

		if (mesh_to_render->is_wireframe())
		{
			SetRasterState(raster_state_wireframe_mode);
		}
		else
		{
			SetRasterState(raster_state_fill_mode);
		}

		SetSamplerState();

		//render
		int tri_to_render = mesh_to_render->get_index_count();
		RenderIndexed(tri_to_render);
	}

	SetRenderTargetView(nullptr, 0);
	SetRenderTargetView(nullptr, 1);
	SetRenderTargetView(nullptr, 2);

	for (int i = 0; i < render_components.size(); i++)
	{
		render_components[i]->post_gbuffer_render();
	}
}

void Renderer::main_render()
{
	//forward_rendering_pipeline();

	SetRasterState(raster_state_fill_mode);
	full_deferred_rendering_pipeline();
}

void Renderer::post_render()
{
	SetRasterState(raster_state_fill_mode);
	SetRenderViews(GetDefaultRenderTargetView(), GetDefaultDepthStencilView(), 0);
	screen_texture->set_srv_to_shader(shader_type_pixel, 3);
	
	for (int i = 0; i < render_components.size(); i++)
	{
		render_components[i]->post_render();
	}

	{
		D3DXVECTOR4 pos(0.89, 0.01, 0, 0);
		D3DXVECTOR4 scale(0.1, 0.1, 1, 1);
		OutputTextureToScreen(gbuffer_normal_texture, pos, scale);
	
		pos.y += 0.11;
		OutputTextureToScreen(gbuffer_albedo_texture, pos, scale);
	
		pos.y += 0.11;
		OutputTextureToScreen(gbuffer_specular_texture, pos, scale);
	}
}

void Renderer::begin_frame()
{
	SetViewPortToDefault();

	clearScreen(D3DXVECTOR4(0,0,0,0));

	validate_render_options();
}

void Renderer::end_frame()
{
	EndScene();
}

void Renderer::add_renderer_component(RenderComponent *component)
{
	render_components.push_back(component);
}

void Renderer::validate_render_options()
{
	for (int i = 0; i < render_components.size(); i++)
	{
		use_postfx |= render_components[i]->uses_postfx();
	}
}

void Renderer::forward_rendering_pipeline()
{
	//set states
	{
		SetViewPortToDefault();

		if (use_postfx)
		{
			SetRenderViews(screen_texture->get_rt(), GetDefaultDepthStencilView(), 0);
		}
		else
		{
			SetRenderViews(GetDefaultRenderTargetView(), GetDefaultDepthStencilView(), 0);
		}

		clearScreen(D3DXVECTOR4(0, 0, 0, 0));
		SetDepthState(depth_state_enable_test_enable_write);
		default_render_shader->set_shaders();
		SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	const vector<Mesh*> &meshes_to_render = scene_to_render->get_meshes();
	for (int i = 0; i < meshes_to_render.size(); i++)
	{
		Mesh *mesh_to_render = meshes_to_render[i];
		mesh_to_render->get_material()->set_textures();
		set_mesh_constant_values(mesh_to_render);

		//set buffers
		SetVertexBuffer(mesh_to_render->get_vertex_buffer(), sizeof(Mesh::Vertex));
		SetIndexBuffer(mesh_to_render->get_index_buffer());

		SetSamplerState();

		if (mesh_to_render->is_wireframe())
		{
			SetRasterState(raster_state_wireframe_mode);
		}
		else
		{
			SetRasterState(raster_state_fill_mode);
		}

		//render
		int tri_to_render = mesh_to_render->get_index_count();
		RenderIndexed(tri_to_render);
	}
}

void Renderer::full_deferred_rendering_pipeline()
{
	SetViewPortToDefault();

	SetBlendState(blend_state_enable_color_write);
	if (use_postfx)
	{
		SetRenderViews(screen_texture->get_rt(), GetDefaultDepthStencilView(), 0);
	}
	else
	{
		SetRenderViews(GetDefaultRenderTargetView(), GetDefaultDepthStencilView(), 0);
	}
	clearScreen();

	set_lighting_constant_values();

	//set gbuffer textures
	gbuffer_albedo_texture->set_srv_to_shader(shader_type_pixel, 0);
	gbuffer_normal_texture->set_srv_to_shader(shader_type_pixel, 1);
	gbuffer_specular_texture->set_srv_to_shader(shader_type_pixel, 2);

	//render for diffuse lighting
	{
		full_deferred_diffuse_lighting_shader->set_shaders();
		RenderFullScreenQuad();
	}

	SetBlendState(blend_state_enable_color_write);
}

void Renderer::set_frame_constant_values()
{
	D3DXMATRIX cur_frame_view_matrix = camera_->get_view_matrix();
	D3DXMATRIX cur_frame_projection_matrix = camera_->get_projection_matrix();
	D3DXMATRIX cur_frame_view_projection_matrix = camera_->get_view_projection_matrix();

	D3DXMATRIX cur_frame_inv_view_matrix = camera_->get_inv_view_matrix();
	D3DXMATRIX cur_frame_inv_projection_matrix = camera_->get_inv_projection_matrix();
	D3DXMATRIX cur_frame_inv_view_projection_matrix = camera_->get_inv_view_projection_matrix();

	D3DXMatrixTranspose(&cur_frame_view_matrix, &cur_frame_view_matrix);
	D3DXMatrixTranspose(&cur_frame_projection_matrix, &cur_frame_projection_matrix);
	D3DXMatrixTranspose(&cur_frame_view_projection_matrix, &cur_frame_view_projection_matrix);

	D3DXMatrixTranspose(&cur_frame_inv_view_matrix, &cur_frame_inv_view_matrix);
	D3DXMatrixTranspose(&cur_frame_inv_projection_matrix, &cur_frame_inv_projection_matrix);
	D3DXMatrixTranspose(&cur_frame_inv_view_projection_matrix, &cur_frame_inv_view_projection_matrix);

	frame_constans_buffer_cpu.view_matrix = cur_frame_view_matrix;
	frame_constans_buffer_cpu.projection_matrix = cur_frame_projection_matrix;
	frame_constans_buffer_cpu.view_projection_matrix = cur_frame_view_projection_matrix;

	frame_constans_buffer_cpu.inv_view_matrix = cur_frame_inv_view_matrix;
	frame_constans_buffer_cpu.inv_projection_matrix = cur_frame_inv_projection_matrix;
	frame_constans_buffer_cpu.inv_view_projection_matrix = cur_frame_inv_view_projection_matrix;
	
	frame_constans_buffer_cpu.right_direction = camera_->get_right_vector();
	frame_constans_buffer_cpu.up_direction = camera_->get_up_vector();
	frame_constans_buffer_cpu.view_direction = camera_->get_forward_vector();
	frame_constans_buffer_cpu.camera_position = camera_->get_position();

	frame_constans_buffer_cpu.near_far_padding2 = D3DXVECTOR4(camera_->get_near(), camera_->get_far(), 0, 0);
	frame_constans_buffer_cpu.screen_texture_half_pixel_forced_mipmap = D3DXVECTOR4((1.0f / float(g_screenWidth)), (1.0f / float(g_screenHeight)), -1, 0);
	frame_constans_buffer_cpu.debug_vector = Utilities::get_debug_vector();

	UpdateBuffer(&frame_constans_buffer_cpu, sizeof(FrameConstantsBuffer), frame_constans_buffer_gpu);
}

void Renderer::set_lighting_constant_values()
{
	const vector<Light*> &lights_to_render = scene_to_render->get_lights();
	if (lights_to_render.size() > 0)
	{
		Light* first_light = lights_to_render[0];
		lighting_contants_buffer_cpu.light_color = first_light->get_color();
		lighting_contants_buffer_cpu.ws_light_position = first_light->get_position();

		D3DXMATRIX viewProjection = camera_->get_view_projection_matrix();
		D3DXVECTOR4 ss_light_p;
		D3DXVec4Transform(&ss_light_p, &lighting_contants_buffer_cpu.ws_light_position, &viewProjection);
		ss_light_p = ss_light_p / ss_light_p.w;
		ss_light_p.x = ss_light_p.x * 0.5 + 0.5;
		ss_light_p.y = ss_light_p.y * 0.5 + 0.5;

		lighting_contants_buffer_cpu.ss_light_position = ss_light_p;

		UpdateBuffer(&lighting_contants_buffer_cpu, sizeof(LightingConstantsBuffer), lighting_constants_buffer_gpu);
	}
}

void Renderer::set_mesh_constant_values(const Mesh *mesh)
{
	D3DXMATRIX world_matrix = mesh->get_frame();
	D3DXMATRIX world_view_matrix = world_matrix * camera_->get_view_matrix();
	D3DXMATRIX world_view_projection_matrix = world_matrix * camera_->get_view_projection_matrix();

	float determinant;
	D3DXMATRIX inv_world_matrix;
	D3DXMATRIX inv_world_view_matrix;
	D3DXMATRIX inv_world_view_projection_matrix;

	D3DXMatrixInverse(&inv_world_matrix, &determinant, &world_matrix);
	D3DXMatrixInverse(&inv_world_view_matrix, &determinant, &world_view_matrix);
	D3DXMatrixInverse(&inv_world_view_projection_matrix, &determinant, &world_view_projection_matrix);

	D3DXMatrixTranspose(&world_matrix, &world_matrix);
	D3DXMatrixTranspose(&world_view_matrix, &world_view_matrix);
	D3DXMatrixTranspose(&world_view_projection_matrix, &world_view_projection_matrix);

	D3DXMatrixTranspose(&inv_world_matrix, &inv_world_matrix);
	D3DXMatrixTranspose(&inv_world_matrix, &inv_world_matrix);
	D3DXMatrixTranspose(&inv_world_view_projection_matrix, &inv_world_view_projection_matrix);

	mesh_constants_buffer_cpu.world_matrix = world_matrix;
	mesh_constants_buffer_cpu.world_view_matrix = world_view_matrix;
	mesh_constants_buffer_cpu.world_view_projection_matrix = world_view_projection_matrix;

	mesh_constants_buffer_cpu.inv_world_matrix = inv_world_matrix;
	mesh_constants_buffer_cpu.inv_world_view_matrix = inv_world_view_matrix;
	mesh_constants_buffer_cpu.inv_world_view_projection_matrix = inv_world_view_projection_matrix;

	mesh_constants_buffer_cpu.diffuse_color = mesh->get_material() ? mesh->get_material()->get_diffuse_color() : D3DXVECTOR4(1,1,1,1);
	mesh_constants_buffer_cpu.bb_min = mesh->get_bb().get_min();
	mesh_constants_buffer_cpu.bb_max = mesh->get_bb().get_max();

	UpdateBuffer(&mesh_constants_buffer_cpu, sizeof(MeshConstantsBuffer), mesh_constants_buffer_gpu);
}

void Renderer::set_scene_to_render(Scene * scene)
{
	scene_to_render = scene;
}

void Renderer::set_camera_controller(Camera *cam)
{
	camera_ = cam;
}

void Renderer::render_mesh(const Mesh * mesh, const Camera &cam)
{
	//TODO_MURAT00 : seperate camera from renderer
	Camera *old_camera = camera_;
	camera_ = (Camera*)&cam;

	SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	set_frame_constant_values();
	set_mesh_constant_values(mesh);

	mesh->get_material()->set_textures();

	//set buffers
	SetVertexBuffer(mesh->get_vertex_buffer(), sizeof(Mesh::Vertex));
	SetIndexBuffer(mesh->get_index_buffer());

	RenderIndexed(mesh->get_index_count());

	camera_ = old_camera;
}

