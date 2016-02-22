
#include "GPUVoxelOcctree.h"
#include "ResourceManager.h"
#include "BoundingBox.h"
#include "Camera.h"
#include "Texture.h"
#include "GPUBuffer.h"
#include "Shader.h"
#include "Renderer.h"

GPUVoxelOcctree::GPUVoxelOcctree(const D3DXVECTOR3 &resolution)
{
	resolution_ = resolution;
	my_scene_ = nullptr;

	x_render_texture_ = new Texture(resolution.z, resolution.y, NULL, DXGI_FORMAT_R32G32B32A32_FLOAT);
	y_render_texture_ = new Texture(resolution.x, resolution.z, NULL, DXGI_FORMAT_R32G32B32A32_FLOAT);
	z_render_texture_ = new Texture(resolution.x, resolution.y, NULL, DXGI_FORMAT_R32G32B32A32_FLOAT);

	const int max_requests = resolution.x * resolution.y;
	node_allocate_requests_buffer_ = new GPUBuffer(sizeof(NodeAllocationRequest), max_requests, NULL, 
		GPUBuffer::CreationFlags::structured_buffer | GPUBuffer::CreationFlags::has_atomic_counter);

	node_allocate_requests_buffer_ping_pong_ = new GPUBuffer(sizeof(NodeAllocationRequest), max_requests, NULL,
		GPUBuffer::CreationFlags::structured_buffer | GPUBuffer::CreationFlags::has_atomic_counter);

	void *nodes_buffer_data = new NodeSet[1024 * 1024];
	memset(nodes_buffer_data, 0, 1024 * 1024 * sizeof(NodeSet));
	nodes_buffer_ = new GPUBuffer(sizeof(NodeSet), 1024 * 1024, NULL, GPUBuffer::CreationFlags::structured_buffer | GPUBuffer::CreationFlags::has_atomic_counter);

	construction_shader_ = new Shader("default_vertex", "voxel_construction_p");
	deferred_cs_shader_ = Shader::create_compute_shader("voxel_filler_cs");

	uniform_constants_buffer_ = new GPUBuffer(sizeof(VCTConstants), 1, nullptr, GPUBuffer::CreationFlags::cpu_write_acces | GPUBuffer::CreationFlags::constant_buffer);
}

GPUVoxelOcctree::~GPUVoxelOcctree()
{
}

void GPUVoxelOcctree::construct(Scene * scene)
{
	my_scene_ = scene;

	update_constant_buffer();

	const BoundingBox &bb = scene->get_bb();

	construction_shader_->set_shaders();
	nodes_buffer_->set_as_uav(0, shaderType::shader_type_pixel);
	node_allocate_requests_buffer_->set_as_uav(1, shaderType::shader_type_pixel, 0);

	SetDepthStencilView(nullptr);

	SetViewPort(0, 0, resolution_.x, resolution_.x);

	//render from -x
	{
		D3DXVECTOR4 cam_pos = D3DXVECTOR4(bb.get_min().x - 5, (bb.get_min().y + bb.get_max().y) * 0.5, (bb.get_min().z + bb.get_max().z) * 0.5, 1);

		Camera ortho_cam;
		ortho_cam.set_position(cam_pos);
		ortho_cam.set_directions(D3DXVECTOR4(1, 0, 0, 0), D3DXVECTOR4(0, 1, 0, 0), D3DXVECTOR4(0, 0, 1, 0));
		ortho_cam.set_ortho_params(bb.get_min().z, bb.get_max().z, bb.get_max().y, bb.get_min().y, 0.01, 10 + (bb.get_max().x - bb.get_min().x));
		ortho_cam.set_is_ortho(true);
		ortho_cam.validate_cur_frame_cache();

		x_render_texture_->set_as_render_target(0);

		const vector<Mesh*> &meshes = my_scene_->get_meshes();
		for (int i = 0; i < meshes.size(); i++)
		{
			renderer->render_mesh(meshes[i], ortho_cam);
		}

		GPUBuffer *input_buffer = node_allocate_requests_buffer_;
		GPUBuffer *output_buffer = node_allocate_requests_buffer_ping_pong_;

		ResetUAVToPixelShader();
		ID3D11UnorderedAccessView *reseter = nullptr;

		while (1)
		{
			int count = input_buffer->get_current_element_count();

			if (count == 0)
			{
				break;
			}

			int number_of_groups = ceil(float(count) / 256.0f);
			deferred_cs_shader_->set_shaders();

			nodes_buffer_->set_as_uav(4, shaderType::shader_type_compute);
			setCShaderUAVResources(&reseter, 1, 5);
			input_buffer->set_as_uav(6, shaderType::shader_type_compute);
			output_buffer->set_as_uav(5, shaderType::shader_type_compute, 0);

			DispatchComputeShader(number_of_groups, 1, 1);

			GPUBuffer *temp = input_buffer;
			input_buffer = output_buffer;
			output_buffer = temp;
		}

		setCShaderUAVResources(&reseter, 1, 6);
		setCShaderUAVResources(&reseter, 1, 5);

	}

	construction_shader_->set_shaders();
	nodes_buffer_->set_as_uav(0, shaderType::shader_type_pixel);
	node_allocate_requests_buffer_->set_as_uav(1, shaderType::shader_type_pixel, 0);

	//render from -y
	{
		D3DXVECTOR4 cam_pos = D3DXVECTOR4((bb.get_min().x + bb.get_max().x) * 0.5, bb.get_min().y - 5, (bb.get_min().z + bb.get_max().z) * 0.5, 1);

		Camera ortho_cam;
		ortho_cam.set_position(cam_pos);
		ortho_cam.set_directions(D3DXVECTOR4(0, 1, 0, 0), D3DXVECTOR4(0, 0 , 1, 0), D3DXVECTOR4(1, 0, 0, 0));
		ortho_cam.set_ortho_params(bb.get_min().x, bb.get_max().x, bb.get_max().z, bb.get_min().z, 0.01, 10 + (bb.get_max().y - bb.get_min().y));
		ortho_cam.set_is_ortho(true);
		ortho_cam.validate_cur_frame_cache();

		y_render_texture_->set_as_render_target(0);

		const vector<Mesh*> &meshes = my_scene_->get_meshes();
		for (int i = 0; i < meshes.size(); i++)
		{
			renderer->render_mesh(meshes[i], ortho_cam);
		}

	}

	construction_shader_->set_shaders();
	nodes_buffer_->set_as_uav(0, shaderType::shader_type_pixel);
	node_allocate_requests_buffer_->set_as_uav(1, shaderType::shader_type_pixel, 0);

	//render from -z
	{
		D3DXVECTOR4 cam_pos = D3DXVECTOR4((bb.get_min().x + bb.get_max().x) * 0.5, (bb.get_min().y + bb.get_max().y) * 0.5, bb.get_min().z - 5, 1);

		Camera ortho_cam;
		ortho_cam.set_position(cam_pos);
		ortho_cam.set_directions(D3DXVECTOR4(0, 0, 1, 0), D3DXVECTOR4(0, 1, 0, 0), D3DXVECTOR4(-1, 0, 0, 0));
		ortho_cam.set_ortho_params(bb.get_min().x, bb.get_max().x, bb.get_max().y, bb.get_min().y, 0.01, 10 + (bb.get_max().z - bb.get_min().z));
		ortho_cam.set_is_ortho(true);
		ortho_cam.validate_cur_frame_cache();

		z_render_texture_->set_as_render_target(0);

		const vector<Mesh*> &meshes = my_scene_->get_meshes();
		for (int i = 0; i < meshes.size(); i++)
		{
			renderer->render_mesh(meshes[i], ortho_cam);
		}
	}

	

}

void GPUVoxelOcctree::update_constant_buffer()
{
	const BoundingBox &scene_bb = my_scene_->get_bb();

	D3DXVECTOR4 length = scene_bb.get_max() - scene_bb.get_min();

	uniform_buffer_cpu_.g_grid_resolution_xyz_iteration_count_w = D3DXVECTOR4(resolution_.x, resolution_.y, resolution_.z, log2(resolution_.x));
	uniform_buffer_cpu_.g_inverse_scene_length = D3DXVECTOR4(1.0f / length.x, 1.0f / length.y, 1.0f / length.z, 1);;
	uniform_buffer_cpu_.g_scene_max = scene_bb.get_max();
	uniform_buffer_cpu_.g_scene_min = scene_bb.get_min();

	uniform_constants_buffer_->update_data(&uniform_buffer_cpu_, sizeof(VCTConstants));
	uniform_constants_buffer_->set_as_constant_buffer(3);
}
