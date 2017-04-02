
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
#include "GPUBuffer.h"
#include "DepthTexture.h"

//#define USE_TILED_RENDERING
//#define USE_TILED_LIGHT_SHADOW_RENDERING
#define USE_MSAA_DEFERRED_RENDERER

#define LIGHT_SHADOW_RESOLUTION 512
#define LIGHT_SHADOW_ATLAS_SIZE (LIGHT_SHADOW_RESOLUTION * 32)

Renderer *renderer = NULL;

Renderer::Renderer()
{
	scene_to_render = nullptr;

	default_render_shader = new Shader("default_vertex", "default_pixel");

	int gbuffer_sample_count = 1;

#ifdef USE_MSAA_DEFERRED_RENDERER
	gbuffer_sample_count = 4;
#endif

	gbuffer_normal_texture = new Texture  (D3DXVECTOR3( g_screenWidth, g_screenHeight,1), nullptr, DXGI_FORMAT_R32G32B32A32_FLOAT , 0, gbuffer_sample_count);
	gbuffer_specular_texture = new Texture(D3DXVECTOR3( g_screenWidth, g_screenHeight,1), nullptr, DXGI_FORMAT_R8G8B8A8_UNORM , 0, gbuffer_sample_count);
	gbuffer_albedo_texture = new Texture  (D3DXVECTOR3( g_screenWidth, g_screenHeight,1), nullptr, DXGI_FORMAT_R8G8B8A8_UNORM , 0, gbuffer_sample_count);

	scene_depth_target_ = new DepthTexture(D3DXVECTOR2(g_screenWidth, g_screenHeight), nullptr, DXGI_FORMAT_D24_UNORM_S8_UINT, gbuffer_sample_count);

	cur_gbuffer_index_to_render_ = 0;

	ds_gbuffer_texture_normal	[0] = new Texture(D3DXVECTOR3(g_screenWidth * 2, g_screenHeight * 2, 1), nullptr, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 1);
	ds_gbuffer_texture_albedo	[0] = new Texture(D3DXVECTOR3(g_screenWidth * 2, g_screenHeight * 2, 1), nullptr, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 1);
	ds_gbuffer_texture_specular	[0] = new Texture(D3DXVECTOR3(g_screenWidth * 2, g_screenHeight * 2, 1), nullptr, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 1);
	
	ds_gbuffer_texture_normal	[1] = new Texture(D3DXVECTOR3(g_screenWidth * 2, g_screenHeight * 2, 1), nullptr, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 1);
	ds_gbuffer_texture_albedo	[1] = new Texture(D3DXVECTOR3(g_screenWidth * 2, g_screenHeight * 2, 1), nullptr, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 1);
	ds_gbuffer_texture_specular	[1] = new Texture(D3DXVECTOR3(g_screenWidth * 2, g_screenHeight * 2, 1), nullptr, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 1);

	screen_texture = new Texture(D3DXVECTOR3(g_screenWidth, g_screenHeight, 1), nullptr, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 1);
	
#ifdef USE_TILED_RENDERING
	gbuffer_shader = new Shader("tiled_gbuffer_vertex", "tiled_gbuffer_pixel");
#else
	gbuffer_shader = new Shader("gbuffer_vertex", "gbuffer_pixel");
#endif

	full_deferred_diffuse_lighting_shader = new Shader("direct_vertex_position", "full_deferred_diffuse_lighting");

#ifdef USE_TILED_LIGHT_SHADOW_RENDERING
	light_shadow_shader = new Shader("tiled_gbuffer_vertex", "tiled_gbuffer_pixel");
#else
	light_shadow_shader = new Shader("gbuffer_vertex", "");
#endif	
	
	use_postfx = false;
	camera_ = nullptr;

	lighting_constants_buffer_gpu = CreateConstantBuffer(sizeof(LightingConstantsBuffer));
	mesh_constants_buffer_gpu = CreateConstantBuffer(sizeof(MeshConstantsBuffer));
	frame_constans_buffer_gpu = CreateConstantBuffer(sizeof(FrameConstantsBuffer));

	SetConstantBufferToSlot(0, frame_constans_buffer_gpu);
	SetConstantBufferToSlot(1, mesh_constants_buffer_gpu);
	SetConstantBufferToSlot(2, lighting_constants_buffer_gpu);

	draw_one_by_one_index_ = 1000;

	creaate_light_shadow_depth_texture();

#ifdef USE_TILED_LIGHT_SHADOW_RENDERING
	initialize_tiled_shadow_renderer();
#endif

#ifdef USE_MSAA_DEFERRED_RENDERER
	initialize_msaa_deferred_renderer();
#endif

}

void Renderer::initialize_msaa_deferred_renderer()
{
	full_deferred_diffuse_lighting_shader_per_msaa_sample = new Shader("direct_vertex_position", "full_deferred_diffuse_lighting_per_msaa_sample");
}

void Renderer::initialize_tiled_shadow_renderer()
{
	//tile info
	tile_size_ = 16;
	gbuffer_render_quad_tree_[0] = new TextureQuadTree(LIGHT_SHADOW_ATLAS_SIZE, tile_size_ * 8);
	gbuffer_render_quad_tree_[1] = new TextureQuadTree(LIGHT_SHADOW_ATLAS_SIZE, tile_size_ * 8);

	tile_count_x_ = LIGHT_SHADOW_RESOLUTION / tile_size_;
	tile_count_y_ = LIGHT_SHADOW_RESOLUTION / tile_size_;

	score_per_tile_ = new float[tile_count_x_ * tile_count_y_];

	for (int i = 0; i < tile_count_y_; i++)
	{
		for (int j = 0; j < tile_count_x_; j++)
		{
			score_per_tile_[j + i * tile_count_x_] = 6;
		}
	}

	tile_render_data_size_ = tile_count_x_ * tile_count_y_ * 2;

	refresh_tile_resolutions();

	current_tile_index_ = 0;

	tile_render_data_ = new int[tile_render_data_size_];

	memset(tile_render_data_, 0, sizeof(int) * tile_render_data_size_);
	per_tile_render_info_ = new GPUBuffer(sizeof(int) * 2, tile_count_x_ * tile_count_y_, tile_render_data_, (UINT)::CreationFlags::structured_buffer);
	cleaner_staging_buffer_ = new GPUBuffer(sizeof(int) * 2, tile_count_x_ * tile_count_y_, tile_render_data_, (UINT)::CreationFlags::staging);

	previous_frame_light_buffer_[0] = new Texture(D3DXVECTOR3(g_screenWidth, g_screenHeight, 1), nullptr, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 1);
	previous_frame_light_buffer_[1] = new Texture(D3DXVECTOR3(g_screenWidth, g_screenHeight, 1), nullptr, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 1);

	do_flicker = false;
}

void Renderer::creaate_light_shadow_depth_texture()
{
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = LIGHT_SHADOW_RESOLUTION;
	depthBufferDesc.Height = LIGHT_SHADOW_RESOLUTION;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create the texture for the depth buffer using the filled out description.
	HRESULT result = g_device->CreateTexture2D(&depthBufferDesc, NULL, &light_depth_texture_);
	light_depth_texture_srv_ = CreateTextureResourceView(light_depth_texture_, DXGI_FORMAT_R32_FLOAT, 0, 1, D3D_SRV_DIMENSION_TEXTURE2D);

	// Set up the depth stencil view description.
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	result = g_device->CreateDepthStencilView(light_depth_texture_, &depthStencilViewDesc, &light_depth_texture_view_);
}

void Renderer::render_frame()
{
	begin_frame();
	
#ifdef USE_TILED_LIGHT_SHADOW_RENDERING
	show_imgui();
#endif

	refresh_tile_resolutions();

	if (scene_to_render != nullptr)
	{
		pre_render();

#ifdef USE_TILED_LIGHT_SHADOW_RENDERING
		tiled_light_shadow_render();
#else
		light_shadow_render();
#endif

		gbuffer_render();

		main_render();

		post_render();
	}

	//Sleep(100);

	current_tile_index_ = (current_tile_index_ + 1) % 2;

	gui.render_frame();

	end_frame();
}

void Renderer::refresh_tile_resolutions()
{
	//for (int j = 0; j < tile_count_y_; j++)
	//{
	//	for (int i = 0; i < tile_count_x_; i++)
	//	{
	//		int current_tile_index = i + j * tile_count_x_;
	//		float current_score = score_per_tile_[current_tile_index];
	//		int size = pow(2, current_score);
	//
	//		pair<int, int> tile_index = make_pair(i, j);
	//		auto prev_it = current_tile_data_[current_tile_index_].find(tile_index);
	//		if (prev_it != current_tile_data_[current_tile_index_].end())
	//		{
	//			gbuffer_render_quad_tree_[current_tile_index_]->return_tile(current_tile_data_[current_tile_index_][tile_index]);
	//			current_tile_data_[current_tile_index_].erase(prev_it);
	//		}
	//	}
	//}

	for (int j = 0; j < tile_count_y_; j++)
	{
		for (int i = 0; i < tile_count_x_; i++)
		{
			int current_tile_index = i + j * tile_count_x_;
			float cur_score = do_flicker ? score_per_tile_[current_tile_index] - current_tile_index_ : score_per_tile_[current_tile_index];
			float current_score = max(cur_score, 0);
			int size = pow(2, current_score);

			pair<int, int> tile_index = make_pair(i, j);

			auto it = current_tile_data_[current_tile_index_].find(tile_index);
			if (it != current_tile_data_[current_tile_index_].end() && it->second.size != size)
			{
				gbuffer_render_quad_tree_[current_tile_index_]->return_tile(current_tile_data_[current_tile_index_][tile_index]);
				current_tile_data_[current_tile_index_].erase(it);
			}

			if (current_tile_data_[current_tile_index_].find(tile_index) == current_tile_data_[current_tile_index_].end())
			{
				TextureQuadTree::Tile new_tile = gbuffer_render_quad_tree_[current_tile_index_]->get_tile(size);
				if (new_tile.size != 0)
				{
					current_tile_data_[current_tile_index_][tile_index] = new_tile;

					int current_index = i + j * tile_count_x_;
					frame_constans_buffer_cpu.screen_tile_info[current_index] = D3DXVECTOR4(new_tile.start.x, new_tile.start.y,
						new_tile.normalized_size.x, new_tile.normalized_size.y);
				}
				else
				{
					int a = 5;
				}
			}
		}
	}
}

void Renderer::pre_render()
{
	for (int i = 0; i < render_components.size(); i++)
	{
		render_components[i]->pre_render();
	}

#ifdef USE_TILED_LIGHT_SHADOW_RENDERING
	tick_tilesets();
#endif

	set_frame_constant_values();
}

void Renderer::tick_tilesets()
{
	int mesh_count = cur_frame_rendered_meshes_.size();
	if (draw_one_by_one_index_ >= 0)
	{
		mesh_count = min(mesh_count, draw_one_by_one_index_);
	}

	D3DXMATRIX view_proj_matrix;
	get_light_shadow_view_proj_matrix(view_proj_matrix);

	for (int i = 0; i < mesh_count && i < draw_one_by_one_index_; i++)
	{
		if (i == 0)
		{
			continue;
		}

		DrawRecord &cur_draw_record = cur_frame_rendered_meshes_[i];
		const D3DXMATRIX &frame = cur_frame_rendered_meshes_[i].frame;
		Mesh *cur_mesh = cur_frame_rendered_meshes_[i].mesh;
		const BoundingBox &mesh_bb = cur_mesh->get_bb();

		D3DXVECTOR4 bb_radius = (mesh_bb.get_max() - mesh_bb.get_min()) * 0.5f;

		D3DXVECTOR2 min_screen_point = D3DXVECTOR2(1e6, 1e6);
		D3DXVECTOR2 max_screen_point = D3DXVECTOR2(-1e6, -1e6);
		D3DXVECTOR4 mesh_location = D3DXVECTOR4(frame.m[3][0], frame.m[3][1], frame.m[3][2], 1);

		for (int i = 0; i < 8; i++)
		{
			int first_axis = (i % 2);
			int second_axis = (i >> 1) % 2;
			int third_axis = (i >> 2) % 2;

			first_axis = first_axis * 2 - 1;
			second_axis = second_axis * 2 - 1;
			third_axis = third_axis * 2 - 1;

			D3DXVECTOR4 current_displacement = D3DXVECTOR4(first_axis * bb_radius.x, second_axis * bb_radius.y, third_axis * bb_radius.z, 0);
			D3DXVECTOR4 current_bb_point = mesh_location + current_displacement;
			D3DXVECTOR4 ss_position;
			D3DXVec4Transform(&ss_position, &current_bb_point, &view_proj_matrix);

			ss_position /= ss_position.w;

			ss_position.x = max(ss_position.x, -1);
			ss_position.y = max(ss_position.y, -1);

			ss_position.x = min(ss_position.x, 1);
			ss_position.y = min(ss_position.y, 1);

			if (ss_position.x < min_screen_point.x)
			{
				min_screen_point.x = ss_position.x;
			}

			if (ss_position.x > max_screen_point.x)
			{
				max_screen_point.x = ss_position.x;
			}

			if (ss_position.y < min_screen_point.y)
			{
				min_screen_point.y = ss_position.y;
			}

			if (ss_position.y > max_screen_point.y)
			{
				max_screen_point.y = ss_position.y;
			}
		}

		min_screen_point = min_screen_point * 0.5 + D3DXVECTOR2(0.5, 0.5);
		max_screen_point = max_screen_point * 0.5 + D3DXVECTOR2(0.5, 0.5);

		min_screen_point.y = 1.0f - min_screen_point.y;
		max_screen_point.y = 1.0f - max_screen_point.y;

		float temp = max_screen_point.y;
		max_screen_point.y = min_screen_point.y;
		min_screen_point.y = temp;

		D3DXVECTOR4 distance_vector = mesh_location - camera_->get_position();

		float distance_to_mesh = sqrt(D3DXVec4Dot(&distance_vector, &distance_vector));
		float grid_multiplier = pow(2, distance_to_mesh / 0.5f);

		int min_x = min_screen_point.x * (tile_count_x_);
		int min_y = min_screen_point.y * (tile_count_y_);

		int max_x = min(max_screen_point.x * (tile_count_x_), tile_count_x_ - 1);
		int max_y = min(max_screen_point.y * (tile_count_y_), tile_count_y_ - 1);

		for (int i = min_x; i <= max_x; i++)
		{
			for (int j = min_y; j <= max_y; j++)
			{
				int current_tile_index = i + j * tile_count_x_;

				cur_draw_record.tiles_.push_back(make_pair(i, j));
			}
		}
	}

	frame_constans_buffer_cpu.screen_tile_size.x = tile_count_x_;
	frame_constans_buffer_cpu.screen_tile_size.y = tile_count_y_;

	frame_constans_buffer_cpu.screen_tile_size.z = 1.0f / tile_count_x_;
	frame_constans_buffer_cpu.screen_tile_size.w = 1.0f / tile_count_y_;

	for (int j = 0; j < tile_count_y_; j++)
	{
		for (int i = 0; i < tile_count_x_; i++)
		{
			pair<int, int> tile_index = make_pair(i, j);
			auto it = current_tile_data_[current_tile_index_].find(tile_index);
			if (it != current_tile_data_[current_tile_index_].end())
			{
				const TextureQuadTree::Tile &new_tile = it->second;

				int current_index = i + j * tile_count_x_;
				frame_constans_buffer_cpu.screen_tile_info[current_index] = D3DXVECTOR4(new_tile.start.x, new_tile.start.y,
					new_tile.normalized_size.x, new_tile.normalized_size.y);
			}
		}
	}

	for (int j = 0; j < tile_count_y_; j++)
	{
		for (int i = 0; i < tile_count_x_; i++)
		{
			pair<int, int> tile_index = make_pair(i, j);
			auto it = current_tile_data_[(current_tile_index_ + 1) % 2].find(tile_index);
			if (it != current_tile_data_[(current_tile_index_ + 1) % 2].end())
			{
				const TextureQuadTree::Tile &new_tile = it->second;

				int current_index = i + j * tile_count_x_;
				frame_constans_buffer_cpu.screen_tile_info[current_index + 64] = D3DXVECTOR4(new_tile.start.x, new_tile.start.y,
					new_tile.normalized_size.x, new_tile.normalized_size.y);
			}
		}
	}
}

void Renderer::show_imgui()
{
	ImGui::Begin("Tile Render Feedback");
	ImGui::Columns(tile_count_x_);

	bool any_change = false;

	for (int i = 0; i < tile_count_x_; i++)
	{
		for (int j = 0; j < tile_count_y_; j++)
		{
			int index = (tile_count_y_ * j + i) * 2;
			char data[256];
			sprintf(data, "##feedbackname%d%d", i, j);

			float ratio = float(tile_render_data_[index + 1]) > 0 ? float(tile_render_data_[index + 1]) / float(tile_render_data_[index]) : 0;
			ImGui::Text("%.2f - %d - %d", ratio, tile_render_data_[index] , tile_render_data_[index + 1]);
			//any_change |= ImGui::InputFloat(data, &score_per_tile_[i + j * tile_count_y_]);
		}

		ImGui::NextColumn();
	}

	ImGui::End();

	ImGui::Begin("Tile Resolutions");
	bool do_change = ImGui::Button("Do Adaptation");
	ImGui::SameLine();
	ImGui::Checkbox("Do Flicker", &do_flicker);

	ImGui::Columns(tile_count_x_);

	for (int i = 0; i < tile_count_x_; i++)
	{
		for (int j = 0; j < tile_count_y_; j++)
		{
			char data[256];
			sprintf(data, "##resname%d%d", i, j);
			ImGui::InputFloat(data, &score_per_tile_[i + j * tile_count_y_]);
		}

		ImGui::NextColumn();
	}

	ImGui::End();

	if (any_change)
	{
		refresh_tile_resolutions();
	}

	if (do_change)
	{
		for (int i = 0; i < tile_count_x_; i++)
		{
			for (int j = 0; j < tile_count_y_; j++)
			{
				int index = (tile_count_y_ * j + i) * 2;

				float ratio = float(tile_render_data_[index + 1]) > 0 ? float(tile_render_data_[index + 1]) / float(tile_render_data_[index]) : 0;
				if (ratio < 0.01 && score_per_tile_[i + j * tile_count_y_] > 0)
				{
					score_per_tile_[i + j * tile_count_y_] -= 1;
				}

				if (ratio > 0.02 && score_per_tile_[i + j * tile_count_y_] < 7)
				{
					score_per_tile_[i + j * tile_count_y_] += 1;
				}
			}
		}
	}
}

void Renderer::gbuffer_render()
{
	invalidate_srv(shaderType::pixel);

	//gbuffer_render_quad_tree_->get_atlas_texture()->set_srv_to_shader(shaderType::pixel, 0);
	
	scene_depth_target_->set_as_depth_stencil_view();

#ifdef USE_TILED_RENDERING
	gbuffer_render_quad_tree_->get_atlas_texture()->set_as_render_target(0);
#else
	gbuffer_albedo_texture->set_as_render_target(0);
	gbuffer_normal_texture->set_as_render_target(1);
	gbuffer_specular_texture->set_as_render_target(2);
#endif

	//gbuffer_render_quad_tree_->get_atlas_texture()->set_as_render_target(0);
	//gbuffer_specular_texture->set_as_render_target(2);

	clearScreen(D3DXVECTOR4(0, 0, 0, 0));
#ifndef USE_TILED_RENDERING
	ClearRenderView(D3DXVECTOR4(0, 0, 0, 0), 1);
	ClearRenderView(D3DXVECTOR4(0, 0, 0, 0) ,2);
#endif

	SetDepthState(true, true, device_comparison_func::less_equal, false, false, device_comparison_func::always, device_stencil_op::zero, 0);
	
	SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

#ifdef USE_TILED_RENDERING
	tiled_gbuffer_render_loop();
#else
	default_gbuffer_render_loop();
#endif

	SetRenderTargetView(nullptr, 0);
	SetRenderTargetView(nullptr, 1);
	SetRenderTargetView(nullptr, 2);

	for (int i = 0; i < render_components.size(); i++)
	{
		render_components[i]->post_gbuffer_render();
	}
}

void Renderer::default_gbuffer_render_loop()
{
	int total_mesh_drawn = 0;

	gbuffer_albedo_texture->set_as_render_target(0);
	gbuffer_normal_texture->set_as_render_target(1);
	gbuffer_specular_texture->set_as_render_target(2);

	clearScreen();

	set_frame_constant_values();

	for (int i = 0; i < cur_frame_rendered_meshes_.size() && i < draw_one_by_one_index_; i++)
	{
		Mesh *mesh_to_render = cur_frame_rendered_meshes_[i].mesh;
		DrawRecord &record = cur_frame_rendered_meshes_[i];

		PIX_EVENT(mesh_to_render->get_name().c_str());

		if (mesh_to_render->get_material())
		{
			mesh_to_render->get_material()->set_textures();
		}

		set_mesh_constant_values(cur_frame_rendered_meshes_[i]);

		Shader *shader_to_set = mesh_to_render->get_material() ? mesh_to_render->get_material()->get_enforced_gbuffer_shader() : nullptr;
		if (shader_to_set == nullptr)
		{
			shader_to_set = gbuffer_shader;
		}

		shader_to_set->set_shaders();

		//set buffers
		SetVertexBuffer(mesh_to_render->get_vertex_buffer(), sizeof(Mesh::Vertex));
		SetIndexBuffer(mesh_to_render->get_index_buffer());

		set_mesh_primitive_topology(mesh_to_render);

		if (mesh_to_render->is_wireframe())
		{
			SetRasterState(raster_state_wireframe_mode);
		}
		else
		{
			SetRasterState(raster_state_fill_mode);
		}

		SetViewPort(0, 0, g_screenWidth, g_screenHeight);

		SetSamplerState();

		//render
		int tri_to_render = mesh_to_render->get_index_count();
		RenderIndexed(tri_to_render);

		total_mesh_drawn++;
	}

	SetRenderTargetView(NULL, 0);
	SetRenderTargetView(NULL, 1);
	SetRenderTargetView(NULL, 2);
}

void Renderer::tiled_gbuffer_render_loop()
{
	int total_mesh_drawn = 0;

	for (int i = 0; i < cur_frame_rendered_meshes_.size(); i++)
	{
		Mesh *mesh_to_render = cur_frame_rendered_meshes_[i].mesh;
		DrawRecord &record = cur_frame_rendered_meshes_[i];

		for (int j = 0; j < record.tiles_.size(); j++)
		{
			if (draw_one_by_one_index_ >= 0 && total_mesh_drawn >= draw_one_by_one_index_)
			{
				break;
			}

			std::pair<int, int> cur_tile = record.tiles_[j];
			TextureQuadTree::Tile current_tile = current_tile_data_[current_tile_index_][cur_tile];

			if (mesh_to_render->get_material())
			{
				mesh_to_render->get_material()->set_textures();
			}

			mesh_constants_buffer_cpu.current_tile_info.x = cur_tile.first;
			mesh_constants_buffer_cpu.current_tile_info.y = cur_tile.second;

			set_mesh_constant_values(cur_frame_rendered_meshes_[i]);

			Shader *shader_to_set = mesh_to_render->get_material() ? mesh_to_render->get_material()->get_enforced_gbuffer_shader() : nullptr;
			if (shader_to_set == nullptr)
			{
				shader_to_set = gbuffer_shader;
			}

			shader_to_set->set_shaders();

			//set buffers
			SetVertexBuffer(mesh_to_render->get_vertex_buffer(), sizeof(Mesh::Vertex));
			SetIndexBuffer(mesh_to_render->get_index_buffer());

			set_mesh_primitive_topology(mesh_to_render);

			if (mesh_to_render->is_wireframe())
			{
				SetRasterState(raster_state_wireframe_mode);
			}
			else
			{
				SetRasterState(raster_state_fill_mode);
			}

			D3DXVECTOR2 viewport_start = D3DXVECTOR2(current_tile.start.x * 4096 - cur_tile.first * current_tile.size,
				current_tile.start.y * 4096 - cur_tile.second * current_tile.size);

			D3DXVECTOR2 viewport_end = viewport_start + D3DXVECTOR2(current_tile.size * tile_count_x_, current_tile.size * tile_count_y_);

			//SetScissorTest(current_tile.start.x * g_screenWidth, current_tile.start.y * g_screenHeight,
			//	current_tile.normalized_size.x * g_screenWidth, current_tile.normalized_size.y * g_screenHeight);
			//SetViewPort(viewport_start.x, viewport_start.y, viewport_end.x - viewport_start.x, viewport_end.y - viewport_start.y);
			SetViewPort(0, 0, 4096, 4096);

			SetSamplerState();

			//render
			int tri_to_render = mesh_to_render->get_index_count();
			RenderIndexed(tri_to_render);

			total_mesh_drawn++;
		}
	}
}

void Renderer::main_render()
{
	//forward_rendering_pipeline();

	SetViewPortToDefault();
	SetRasterState(raster_state_fill_mode);
	full_deferred_rendering_pipeline();
}

void Renderer::post_render()
{
	SetViewPortToDefault();

	SetRasterState(raster_state_fill_mode);
	SetRenderViews(GetDefaultRenderTargetView(), GetDefaultDepthStencilView(), 0);
	screen_texture->set_srv_to_shader(shaderType::pixel, 3);
	
	for (int i = 0; i < render_components.size(); i++)
	{
		render_components[i]->post_render();
	}

	{
		D3DXVECTOR4 pos(0.01, 0.89, 0, 0);
		D3DXVECTOR4 scale(0.1, 0.1, 1, 1);
#ifndef USE_MSAA_DEFERRED_RENDERER
		OutputTextureToScreen(gbuffer_normal_texture, pos, scale);
	
		pos.y -= 0.11;
		OutputTextureToScreen(gbuffer_albedo_texture, pos, scale);
		
		pos.y -= 0.11;
		OutputTextureToScreen(gbuffer_specular_texture, pos, scale);
#endif

#ifdef USE_TILED_LIGHT_SHADOW_RENDERING
		pos.y -= 0.33;
		OutputTextureToScreen(gbuffer_render_quad_tree_[current_tile_index_]->depth_atlas_texture_srv_, pos, scale);
#endif
	}
}

void Renderer::begin_frame()
{
	SetViewPortToDefault();

	clearScreen(D3DXVECTOR4(0,0,0,0));

	validate_render_options();

	ImGui::Begin("Renderer Stats");
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::Text("Camera pos %f %f %f", camera_->get_position().x, camera_->get_position().y, camera_->get_position().z);
	ImGui::Text("Camera gaze %f %f %f", camera_->get_forward_vector().x, camera_->get_forward_vector().y, camera_->get_forward_vector().z);
	ImGui::Text("Camera up %f %f %f", camera_->get_up_vector().x, camera_->get_up_vector().y, camera_->get_up_vector().z);
	ImGui::Text("Camera right %f %f %f", camera_->get_right_vector().x, camera_->get_right_vector().y, camera_->get_right_vector().z);
	ImGui::Text("Number of meshes %d", scene_to_render->get_mesh_count());
	ImGui::InputInt("Draw one by one %d", &draw_one_by_one_index_);

	ImGui::End();

	cur_frame_rendered_meshes_.clear();
	scene_to_render->get_meshes_to_render(camera_, cur_frame_rendered_meshes_);

}

void Renderer::end_frame()
{
	cur_gbuffer_index_to_render_ = (cur_gbuffer_index_to_render_ + 1) % 2;

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
		SetDepthState(true, true, device_comparison_func::less_equal, false, false, device_comparison_func::always, device_stencil_op::zero, 0);
		default_render_shader->set_shaders();
		SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	for (int i = 0; i < cur_frame_rendered_meshes_.size(); i++)
	{
		Mesh *mesh_to_render = cur_frame_rendered_meshes_[i].mesh;
		mesh_to_render->get_material()->set_textures();
		set_mesh_constant_values(cur_frame_rendered_meshes_[i]);

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
#ifdef USE_MSAA_DEFERRED_RENDERER
	//find complex pixels, write stencil for those
	//use 2 passes in full deferred lighting..
#endif

	SetViewPortToDefault();
	SetDepthStencilView(nullptr);

	SetBlendState(blend_state_enable_color_write);
	SetDepthState(false, false, device_comparison_func::always, false, false, device_comparison_func::always, device_stencil_op::zero, 0);

	if (use_postfx)
	{
		SetRenderViews(screen_texture->get_rt(), nullptr, 0);
	}
	else
	{
		SetRenderViews(GetDefaultRenderTargetView(), nullptr, 0);
	}

	set_lighting_constant_values();
	SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//set gbuffer textures
#ifdef USE_TILED_RENDERING
	gbuffer_render_quad_tree_->get_atlas_texture()->set_srv_to_shader(shaderType::pixel, 1);
#else
	gbuffer_albedo_texture->set_srv_to_shader(shaderType::pixel, 0);
	gbuffer_normal_texture->set_srv_to_shader(shaderType::pixel, 1);
	gbuffer_specular_texture->set_srv_to_shader(shaderType::pixel, 2);
#endif

#ifdef USE_TILED_LIGHT_SHADOW_RENDERING
	SetSRV(&gbuffer_render_quad_tree_[current_tile_index_]->depth_atlas_texture_srv_, 1, shaderType::pixel, 5);
	SetSRV(&gbuffer_render_quad_tree_[(current_tile_index_ + 1) % 2]->depth_atlas_texture_srv_, 1, shaderType::pixel, 6);
	per_tile_render_info_->set_as_uav(4, shaderType::pixel);
	previous_frame_light_buffer_[current_tile_index_]->set_as_render_target(1);
	//previous_frame_light_buffer_[(current_tile_index_ + 1) % 2]->set_srv_to_shader(shaderType::pixel, 6);
#else
	SetSRV(&light_depth_texture_srv_, 1, shaderType::pixel, 5);
#endif

	clearScreen();

	//render for diffuse lighting
	{
#ifdef USE_MSAA_DEFERRED_RENDERER
		full_deferred_diffuse_lighting_shader_per_msaa_sample->set_shaders();
#else
		full_deferred_diffuse_lighting_shader->set_shaders();
#endif
		RenderFullScreenQuad();
	}

	SetSRV(NULL, shaderType::pixel, 0);
	SetSRV(NULL, shaderType::pixel, 1);
	SetSRV(NULL, shaderType::pixel, 2);
	SetSRV(NULL, shaderType::pixel, 3);
	SetSRV(NULL, shaderType::pixel, 4);
	SetSRV(NULL, shaderType::pixel, 5);
	SetSRV(NULL, shaderType::pixel, 6);

	SetRenderTargetView(nullptr, 1);

	SetUAVToPixelShader(NULL, 4);

	SetBlendState(blend_state_enable_color_write);

#ifdef USE_TILED_LIGHT_SHADOW_RENDERING
	per_tile_render_info_->get_data(tile_render_data_, tile_render_data_size_ * sizeof(int));
	per_tile_render_info_->copy_contents(cleaner_staging_buffer_);
#endif
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
		lighting_contants_buffer_cpu.ws_light_position = first_light->get_type() == Light::type_directional ? first_light->get_direction() : first_light->get_position();

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

void Renderer::set_mesh_constant_values(const DrawRecord& draw_rec)
{
	D3DXMATRIX world_matrix = draw_rec.frame;
	D3DXMATRIX world_view_matrix = world_matrix * camera_->get_view_matrix();
	D3DXMATRIX cur_frame_projection_matrix = camera_->get_projection_matrix();

	D3DXMATRIX world_view_projection_matrix = world_view_matrix * cur_frame_projection_matrix;

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

	mesh_constants_buffer_cpu.diffuse_color = draw_rec.mesh->get_material() ? draw_rec.mesh->get_material()->get_diffuse_color() : D3DXVECTOR4(1,1,1,1);
	mesh_constants_buffer_cpu.bb_min = draw_rec.mesh->get_bb().get_min();
	mesh_constants_buffer_cpu.bb_max = draw_rec.mesh->get_bb().get_max();

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

	set_mesh_primitive_topology(mesh);

	set_frame_constant_values();
	set_mesh_constant_values(DrawRecord((Mesh*)mesh, D3DXMATRIX()));

	mesh->get_material()->set_textures();

	//set buffers
	SetVertexBuffer(mesh->get_vertex_buffer(), sizeof(Mesh::Vertex));
	SetIndexBuffer(mesh->get_index_buffer());

	RenderIndexed(mesh->get_index_count());

	camera_ = old_camera;
}

void Renderer::set_mesh_primitive_topology(const Mesh * mesh)
{
	switch (mesh->get_mesh_type())
	{
	case Mesh::MeshType::triangle_mesh:
		SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		break;
	case Mesh::MeshType::line_mesh:
		SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		break;
	}
}

void Renderer::light_shadow_render()
{
	if (scene_to_render->get_lights().size() > 0)
	{
		SetDepthState(true, true, device_comparison_func::less_equal, false, false, device_comparison_func::always, device_stencil_op::zero, 0);

		Light* first_light = scene_to_render->get_lights()[0];
		D3DXVECTOR3 light_position = first_light->get_position();

		D3DXMATRIX light_view_matrix;
		D3DXMATRIX light_projection_matrix;

		if (first_light->get_type() == Light::type_pointlight)
		{
			D3DXVECTOR3 gaze_vector(0, -1, 0);
			D3DXVECTOR3 up_vector(0, 0, -1);
			D3DXVECTOR3 right_vector(1, 0, 0);
			D3DXVECTOR3 light_lookat = light_position + gaze_vector;

			D3DXMatrixLookAtRH(&light_view_matrix, &light_position, &light_lookat, &up_vector);
			D3DXMatrixPerspectiveFovRH(&light_projection_matrix, (90.0f / 180.0f) * PI, 1, 0.01f, 1000.0f);
		}
		else
		{
			D3DXVECTOR3 right_vector(1, 0, 0);
			D3DXVECTOR3 gaze_vector = first_light->get_direction();
			D3DXVECTOR3 up_vector;

			D3DXVec3Cross(&up_vector, &right_vector, &gaze_vector);
			D3DXVec3Cross(&right_vector, &gaze_vector, &up_vector);

			D3DXVECTOR3 light_lookat = scene_to_render->get_bb().get_max();
			D3DXVECTOR3 light_dir(first_light->get_direction().x, first_light->get_direction().y, first_light->get_direction().z);
			D3DXVECTOR3 light_position_to_give = light_lookat - light_dir * 500;

			D3DXMatrixLookAtRH(&light_view_matrix, &light_position, &light_lookat, &up_vector);

			vector<D3DXVECTOR4> bb_points_in_light_space;
			scene_to_render->get_bb().get_points(bb_points_in_light_space);

			for (int i = 0; i < bb_points_in_light_space.size(); i++)
			{
				D3DXVec4Transform(&bb_points_in_light_space[i], &bb_points_in_light_space[i], &light_view_matrix);
			}

			BoundingBox scene_bb_in_light_space;
			
			for (int i = 0; i < bb_points_in_light_space.size(); i++)
			{
				scene_bb_in_light_space.enlarge_bb_with_point(bb_points_in_light_space[i]);
			}

			D3DXVECTOR4 light_max = scene_bb_in_light_space.get_max();
			D3DXVECTOR4 light_min = scene_bb_in_light_space.get_min();

			D3DXMatrixOrthoRH(&light_projection_matrix, (light_max.x - light_min.x) * 3, (light_max.y - light_min.y) * 3, 0.01f, 1000.0f);
		}


		D3DXMATRIX light_view_projection_matrix = light_view_matrix * light_projection_matrix;

		SetRenderTargetView(nullptr, 0);
		SetRenderTargetView(nullptr, 1);
		SetRenderTargetView(nullptr, 2);

		SetDepthStencilView(light_depth_texture_view_);
		clearScreen();

		for (int i = 0; i < cur_frame_rendered_meshes_.size() && i < draw_one_by_one_index_; i++)
		{
			Mesh *mesh_to_render = cur_frame_rendered_meshes_[i].mesh;
			DrawRecord &record = cur_frame_rendered_meshes_[i];

			D3DXMATRIX mesh_world_view_proj_matrix = record.frame * light_view_projection_matrix;
			D3DXMatrixTranspose(&mesh_world_view_proj_matrix, &mesh_world_view_proj_matrix);
			mesh_constants_buffer_cpu.world_view_projection_matrix = mesh_world_view_proj_matrix;
			UpdateBuffer(&mesh_constants_buffer_cpu, sizeof(MeshConstantsBuffer), mesh_constants_buffer_gpu);
			
			light_shadow_shader->set_shaders();

			//set buffers
			SetVertexBuffer(mesh_to_render->get_vertex_buffer(), sizeof(Mesh::Vertex));
			SetIndexBuffer(mesh_to_render->get_index_buffer());

			set_mesh_primitive_topology(mesh_to_render);
			SetRasterState(raster_state_fill_mode);

			SetViewPort(0, 0, LIGHT_SHADOW_RESOLUTION, LIGHT_SHADOW_RESOLUTION);

			SetSamplerState();

			//render
			int tri_to_render = mesh_to_render->get_index_count();
			RenderIndexed(tri_to_render);
		}

		float determinant;
		D3DXMATRIX light_view_projection_matrix_inv;
		D3DXMatrixInverse(&light_view_projection_matrix_inv, &determinant, &light_view_projection_matrix);

		D3DXMatrixTranspose(&light_view_projection_matrix_inv, &light_view_projection_matrix_inv);
		D3DXMatrixTranspose(&light_view_projection_matrix, &light_view_projection_matrix);

		lighting_contants_buffer_cpu.light_view_projection_matrix = light_view_projection_matrix;
		lighting_contants_buffer_cpu.light_view_projection_matrix_inv = light_view_projection_matrix_inv;

		UpdateBuffer(&lighting_contants_buffer_cpu, sizeof(LightingConstantsBuffer), lighting_constants_buffer_gpu);
	}

}

void Renderer::tiled_light_shadow_render()
{
	if (scene_to_render->get_lights().size() > 0)
	{
		SetDepthState(true, true, device_comparison_func::less_equal, false, false, device_comparison_func::always, device_stencil_op::zero, 0);

		Light* first_light = scene_to_render->get_lights()[0];
		D3DXVECTOR3 light_position = first_light->get_position();

		D3DXVECTOR3 gaze_vector(0, -1, 0);
		D3DXVECTOR3 up_vector(0, 0, -1);
		D3DXVECTOR3 right_vector(1, 0, 0);
		D3DXVECTOR3 light_lookat = light_position + gaze_vector;

		D3DXMATRIX light_view_matrix;
		D3DXMATRIX light_projection_matrix;

		D3DXMatrixLookAtRH(&light_view_matrix, &light_position, &light_lookat, &up_vector);
		D3DXMatrixPerspectiveFovRH(&light_projection_matrix, (90.0f / 180.0f) * PI, 1, 0.01f, 100.0f);

		D3DXMATRIX light_view_projection_matrix = light_view_matrix * light_projection_matrix;

		SetRenderTargetView(nullptr, 0);
		SetRenderTargetView(nullptr, 1);
		SetRenderTargetView(nullptr, 2);

		SetViewPort(0, 0, LIGHT_SHADOW_ATLAS_SIZE, LIGHT_SHADOW_ATLAS_SIZE);
		SetDepthStencilView(gbuffer_render_quad_tree_[current_tile_index_]->depth_atlas_texture_view_);
		clearScreen();

		for (int i = 0; i < cur_frame_rendered_meshes_.size() && i < draw_one_by_one_index_; i++)
		{
			Mesh *mesh_to_render = cur_frame_rendered_meshes_[i].mesh;
			DrawRecord &record = cur_frame_rendered_meshes_[i];

			for (int j = 0; j < record.tiles_.size(); j++)
			{
				std::pair<int, int> cur_tile = record.tiles_[j];
				TextureQuadTree::Tile current_tile = current_tile_data_[current_tile_index_][cur_tile];

				if (mesh_to_render->get_material())
				{
					mesh_to_render->get_material()->set_textures();
				}

				mesh_constants_buffer_cpu.current_tile_info.x = cur_tile.first;
				mesh_constants_buffer_cpu.current_tile_info.y = cur_tile.second;

				D3DXMATRIX mesh_world_view_proj_matrix = record.frame * light_view_projection_matrix;
				D3DXMatrixTranspose(&mesh_world_view_proj_matrix, &mesh_world_view_proj_matrix);
				mesh_constants_buffer_cpu.world_view_projection_matrix = mesh_world_view_proj_matrix;
				UpdateBuffer(&mesh_constants_buffer_cpu, sizeof(MeshConstantsBuffer), mesh_constants_buffer_gpu);

				light_shadow_shader->set_shaders();

				//set buffers
				SetVertexBuffer(mesh_to_render->get_vertex_buffer(), sizeof(Mesh::Vertex));
				SetIndexBuffer(mesh_to_render->get_index_buffer());

				set_mesh_primitive_topology(mesh_to_render);
				SetRasterState(raster_state_fill_mode);


				SetSamplerState();

				//render
				int tri_to_render = mesh_to_render->get_index_count();
				RenderIndexed(tri_to_render);
			}
		}

		float determinant;
		D3DXMATRIX light_view_projection_matrix_inv;
		D3DXMatrixInverse(&light_view_projection_matrix_inv, &determinant, &light_view_projection_matrix);

		D3DXMatrixTranspose(&light_view_projection_matrix_inv, &light_view_projection_matrix_inv);
		D3DXMatrixTranspose(&light_view_projection_matrix, &light_view_projection_matrix);

		lighting_contants_buffer_cpu.light_view_projection_matrix = light_view_projection_matrix;
		lighting_contants_buffer_cpu.light_view_projection_matrix_inv = light_view_projection_matrix_inv;

		UpdateBuffer(&lighting_contants_buffer_cpu, sizeof(LightingConstantsBuffer), lighting_constants_buffer_gpu);
	}

}

void Renderer::get_light_shadow_view_proj_matrix(D3DXMATRIX & lvp)
{
	if (scene_to_render->get_lights().size() > 0)
	{
		Light* first_light = scene_to_render->get_lights()[0];
		D3DXVECTOR3 light_position = first_light->get_position();

		D3DXVECTOR3 gaze_vector(0, -1, 0);
		D3DXVECTOR3 up_vector(0, 0, -1);
		D3DXVECTOR3 right_vector(1, 0, 0);
		D3DXVECTOR3 light_lookat = light_position + gaze_vector;

		D3DXMATRIX light_view_matrix;
		D3DXMATRIX light_projection_matrix;

		D3DXMatrixLookAtRH(&light_view_matrix, &light_position, &light_lookat, &up_vector);
		D3DXMatrixPerspectiveFovRH(&light_projection_matrix, (90.0f / 180.0f) * PI, 1, 0.01f, 100.0f);

		lvp = light_view_matrix * light_projection_matrix;
	}
}

