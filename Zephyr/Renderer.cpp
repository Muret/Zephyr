
#include "Renderer.h"
#include "RenderComponent.h"
#include "Mesh.h"
#include "Shader.h"
#include "Material.h"
#include "Texture.h"

Renderer *renderer = NULL;

Renderer::Renderer()
{
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

	for (int i = 2; i < 4; i++)
	{
		Mesh *mesh_to_render = meshes_to_render[i];
		mesh_to_render->get_material()->set_textures();
		mesh_to_render->set_uniform_values();

		//set buffers
		SetVertexBuffer(mesh_to_render->get_vertex_buffer(), sizeof(Mesh::Vertex));
		SetIndexBuffer(mesh_to_render->get_index_buffer());

		SetSamplerState();

		//render
		RenderIndexed(mesh_to_render->get_index_count());
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

void Renderer::add_mesh_to_render(Mesh* mesh)
{
	meshes_to_render.push_back(mesh);
}

void Renderer::add_meshes_to_render(vector<Mesh*> &mesh)
{
	meshes_to_render.insert(meshes_to_render.begin(), mesh.begin(), mesh.end());
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
		RenderIndexed(mesh_to_render->get_index_count());
	}
}

void Renderer::full_deferred_rendering_pipeline()
{
	//set gbuffer textures

	SetViewPortToDefault();

	SetBlendState(blend_state_enable_color_write);
	screen_texture->set_as_render_target(0);
	clearScreen();

	D3DXMATRIX matrix;
	D3DXMatrixIdentity(&matrix);
	render_constantsBuffer_cpu.WorldViewProjectionMatrix = matrix;
	UpdateGlobalBuffers();

	gbuffer_albedo_texture->set_srv_to_shader(shader_type_pixel,0);
	gbuffer_normal_texture->set_srv_to_shader(shader_type_pixel, 1);
	gbuffer_specular_texture->set_srv_to_shader(shader_type_pixel, 2);

	//render for diffuse lighting
	{
		full_deferred_diffuse_lighting_shader->set_shaders();
		RenderFullScreenQuad();
	}

	SetBlendState(blend_state_enable_color_write);
}

