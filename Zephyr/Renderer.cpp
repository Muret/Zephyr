
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

Renderer *renderer = NULL;

Renderer::Renderer()
{
	scene_to_render = nullptr;

	default_render_shader = new Shader("default_vertex", "default_pixel");

	gbuffer_normal_texture = new Texture(g_screenWidth, g_screenHeight, nullptr, DXGI_FORMAT_R8G8B8A8_UNORM);
	gbuffer_specular_texture = new Texture(g_screenWidth, g_screenHeight, nullptr, DXGI_FORMAT_R8G8B8A8_UNORM);
	gbuffer_albedo_texture = new Texture(g_screenWidth, g_screenHeight, nullptr, DXGI_FORMAT_R8G8B8A8_UNORM);

	screen_texture = new Texture(g_screenWidth, g_screenHeight, nullptr, DXGI_FORMAT_R8G8B8A8_UNORM);
	
	gbuffer_shader = new Shader("default_vertex", "gbuffer_pixel");
	full_deferred_diffuse_lighting_shader = new Shader("default_vertex", "full_deferred_diffuse_lighting");
	
	use_postfx = false;
}

void Renderer::render_frame()
{
	if (scene_to_render == nullptr)
	{
		return;
	}

	begin_frame();

	pre_render();

	gbuffer_render();

	main_render();

	post_render();

	end_frame();
}

void Renderer::pre_render()
{
	for (int i = 0; i < render_components.size(); i++)
	{
		render_components[i]->pre_render();
	}
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
	gbuffer_shader->set_shaders();
	SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	const vector<Mesh*> &meshes_to_render = scene_to_render->get_meshes();

	for (int i = 0; i < meshes_to_render.size(); i++)
	{
		Mesh *mesh_to_render = meshes_to_render[i];
		mesh_to_render->get_material()->set_textures();
		mesh_to_render->set_uniform_values();

		//set buffers
		SetVertexBuffer(mesh_to_render->get_vertex_buffer(), sizeof(Mesh::Vertex));
		SetIndexBuffer(mesh_to_render->get_index_buffer());

		SetSamplerState();

		//render
		int tri_to_render = mesh_to_render->get_index_count();// min(mesh_to_render->get_index_count(), Utilities::get_debug_vector().x * 3 * 5);
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

	full_deferred_rendering_pipeline();
}

void Renderer::post_render()
{
	{
		SetRenderViews(GetDefaultRenderTargetView(), GetDefaultDepthStencilView(), 0);
		screen_texture->set_srv_to_shader(shader_type_pixel, 3);
	}
	
	for (int i = 0; i < render_components.size(); i++)
	{
		render_components[i]->post_render();
	}

	{
		D3DXVECTOR4 pos(0.8, 0.8, 0, 0);
		D3DXVECTOR4 scale(0.1, 0.1, 1, 1);
		OutputTextureToScreen(gbuffer_normal_texture, pos, scale);

		pos.y = 0.55;
		OutputTextureToScreen(gbuffer_albedo_texture, pos, scale);

		pos.y = 0.30;
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
		mesh_to_render->set_uniform_values();

		//set buffers
		SetVertexBuffer(mesh_to_render->get_vertex_buffer(), sizeof(Mesh::Vertex));
		SetIndexBuffer(mesh_to_render->get_index_buffer());

		SetSamplerState();

		//render
		int tri_to_render = mesh_to_render->get_index_count();// min(mesh_to_render->get_index_count(), Utilities::get_debug_vector().x);
		RenderIndexed(tri_to_render);
	}
}

void Renderer::full_deferred_rendering_pipeline()
{
	const vector<Light*> &lights_to_render = scene_to_render->get_lights();
	if (lights_to_render.size() > 0)
	{
		Light* first_light = lights_to_render[0];
		lighting_InfoBuffer_cpu.light_color = first_light->get_color();
		lighting_InfoBuffer_cpu.ws_light_position = first_light->get_position();

		D3DXMATRIX viewProjection = demo_camera.get_view_projection_matrix();
		D3DXVECTOR4 ss_light_p;
		D3DXVec4Transform(&ss_light_p, &lighting_InfoBuffer_cpu.ws_light_position, &viewProjection);
		ss_light_p = ss_light_p / ss_light_p.w;
		ss_light_p.x = ss_light_p.x * 0.5 + 0.5;
		ss_light_p.y = ss_light_p.y * 0.5 + 0.5;

		lighting_InfoBuffer_cpu.ss_light_position = ss_light_p;

		UpdateGlobalBuffers();
	}

	SetViewPortToDefault();

	SetBlendState(blend_state_enable_color_write);
	screen_texture->set_as_render_target(0);
	clearScreen();

	D3DXMATRIX matrix;
	D3DXMatrixIdentity(&matrix);
	render_constantsBuffer_cpu.WorldViewProjectionMatrix = matrix;
	UpdateGlobalBuffers();

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

void Renderer::set_scene_to_render(Scene * scene)
{
	scene_to_render = scene;
}

