
#include "GPUVoxelOcctree.h"
#include "ResourceManager.h"
#include "BoundingBox.h"
#include "Camera.h"
#include "Texture.h"
#include "GPUBuffer.h"
#include "Shader.h"
#include "Renderer.h"
#include "Utilities.h"

GPUVoxelOcctree::GPUVoxelOcctree(const D3DXVECTOR3 &resolution)
{
	resolution_ = resolution;
	my_scene_ = nullptr;

	x_render_texture_ = new Texture( D3DXVECTOR3(resolution.z, resolution.y, 1), NULL, DXGI_FORMAT_R32G32B32A32_FLOAT, 0);
	y_render_texture_ = new Texture( D3DXVECTOR3(resolution.x, resolution.z, 1), NULL, DXGI_FORMAT_R32G32B32A32_FLOAT, 0);
	z_render_texture_ = new Texture( D3DXVECTOR3(resolution.x, resolution.y, 1), NULL, DXGI_FORMAT_R32G32B32A32_FLOAT, 0);

	const int max_requests = resolution.x * resolution.y;
	node_allocate_requests_buffer_ = new GPUBuffer(sizeof(NodeAllocationRequest), max_requests, NULL, 
		CreationFlags::structured_buffer | CreationFlags::has_atomic_counter);

	node_allocate_requests_buffer_ping_pong_ = new GPUBuffer(sizeof(NodeAllocationRequest), max_requests, NULL,
		CreationFlags::structured_buffer | CreationFlags::has_atomic_counter);

	void *nodes_buffer_data = new NodeSet[1024 * 1024];
	memset(nodes_buffer_data, 0, 1024 * 1024 * sizeof(NodeSet));
	nodes_buffer_ = new GPUBuffer(sizeof(NodeSet), 1024 * 1024, NULL, CreationFlags::structured_buffer | CreationFlags::has_atomic_counter);

	construction_shader_ = new Shader("default_vertex", "voxel_construction_p");
	deferred_cs_shader_ = Shader::create_compute_shader("voxel_filler_cs");

	uniform_constants_buffer_ = new GPUBuffer(sizeof(VCTConstants), 1, nullptr, CreationFlags::cpu_write_acces | CreationFlags::constant_buffer);

	int max_bricks = 1024;
	int pixel_count = max_bricks * 3 * 3 * 1.5 * max_bricks;
	float *initial_data = new float[pixel_count * 4];
	memset(initial_data, 0 , pixel_count * 4 * sizeof(float));
	leaf_bricks_3d = new Texture(D3DXVECTOR3( 3 * 4 , 3 * 4 , 1.5 * max_bricks), initial_data, DXGI_FORMAT_R32G32B32A32_FLOAT , (UINT)CreationFlags::structured_buffer);
	delete[] initial_data;
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
	nodes_buffer_->set_as_uav(4, shaderType::pixel, 0);
	node_allocate_requests_buffer_->set_as_uav(5, shaderType::pixel, 0);
	leaf_bricks_3d->set_as_uav(7, shaderType::pixel, 0);

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

		build_octree_from_side_aux(ortho_cam);
	}

	construction_shader_->set_shaders();
	nodes_buffer_->set_as_uav(4, shaderType::pixel);
	node_allocate_requests_buffer_->set_as_uav(5, shaderType::pixel, 0);
	leaf_bricks_3d->set_as_uav(7, shaderType::pixel);

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

		//build_octree_from_side_aux(ortho_cam);
	}

	construction_shader_->set_shaders();
	nodes_buffer_->set_as_uav(4, shaderType::pixel);
	node_allocate_requests_buffer_->set_as_uav(5, shaderType::pixel, 0);
	leaf_bricks_3d->set_as_uav(7, shaderType::pixel);

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

		//build_octree_from_side_aux(ortho_cam);
	}

	create_debug_render_mesh();
}

void GPUVoxelOcctree::build_octree_from_side_aux(const Camera &cam)
{
	const vector<Mesh*> &meshes = my_scene_->get_meshes();
	for (int i = 0; i < meshes.size(); i++)
	{
		renderer->render_mesh(meshes[i], cam);
	}

	GPUBuffer *input_buffer = node_allocate_requests_buffer_;
	GPUBuffer *output_buffer = node_allocate_requests_buffer_ping_pong_;
	ID3D11UnorderedAccessView *reseter = nullptr;

	ResetUAVToPixelShader();

	while (false)
	{
		int count = input_buffer->get_current_element_count();
		int node_count = nodes_buffer_->get_current_element_count();

		NodeSet node_set;
		nodes_buffer_->get_data(&node_set, sizeof(NodeSet));
		int number_of_texture_bricks = node_set.nodes[1].construction_mutex;

		if (count == 0)
		{
			break;
		}
		 
		int number_of_groups = ceil(float(count) / 256.0f);
		deferred_cs_shader_->set_shaders();

		nodes_buffer_->set_as_uav(4, shaderType::compute);
		setCShaderUAVResources(&reseter, 1, 5);
		input_buffer->set_as_uav(6, shaderType::compute);
		output_buffer->set_as_uav(5, shaderType::compute, 0);
		leaf_bricks_3d->set_as_uav(7, shaderType::compute);

		DispatchComputeShader(number_of_groups, 1, 1);

		GPUBuffer *temp = input_buffer;
		input_buffer = output_buffer;
		output_buffer = temp;
	}

	setCShaderUAVResources(&reseter, 1, 4);
	setCShaderUAVResources(&reseter, 1, 6);
	setCShaderUAVResources(&reseter, 1, 5);
	setCShaderUAVResources(&reseter, 1, 7);
}

void GPUVoxelOcctree::update_constant_buffer()
{
	const BoundingBox &scene_bb = my_scene_->get_bb();
	const float max_number_of_leaves = 1024;

	D3DXVECTOR4 length = scene_bb.get_max() - scene_bb.get_min();

	uniform_buffer_cpu_.g_grid_resolution_xyz_iteration_count_w = D3DXVECTOR4(resolution_.x, resolution_.y, resolution_.z, log2(resolution_.x));
	uniform_buffer_cpu_.g_inverse_scene_length = D3DXVECTOR4(1.0f / length.x, 1.0f / length.y, 1.0f / length.z, 1);;
	uniform_buffer_cpu_.g_scene_max = scene_bb.get_max();
	uniform_buffer_cpu_.g_scene_min = scene_bb.get_min();
	
	D3DXVECTOR3 leaf_dimension_mult = resolution_ * 1.5f;
	uniform_buffer_cpu_.g_inverse_leaf_dimension = D3DXVECTOR4(max_number_of_leaves / leaf_dimension_mult.x, 1.0f / leaf_dimension_mult.y, 1.0f / leaf_dimension_mult.z, 1);;

	uniform_constants_buffer_->update_data(&uniform_buffer_cpu_, sizeof(VCTConstants));
	uniform_constants_buffer_->set_as_constant_buffer(3);
}

void GPUVoxelOcctree::create_debug_render_mesh()
{
	//get node information
	int total_node_count = nodes_buffer_->get_current_element_count() + 1;
	NodeSet *node_sets = new NodeSet[total_node_count];
	nodes_buffer_->get_data(node_sets, total_node_count * sizeof(NodeSet));

	int number_of_cells = node_sets[0].nodes[1].construction_mutex;

	std::function<void(NodeSet *, Node*, vector<pair<D3DXVECTOR3, int>>&, const D3DXVECTOR3&, const D3DXVECTOR3&)> collect_tree_leaf_brick_indexes;

	collect_tree_leaf_brick_indexes = [&collect_tree_leaf_brick_indexes](NodeSet *root_node, Node *cur_node, vector<pair<D3DXVECTOR3, int>>& output, const D3DXVECTOR3& cur_middle, const D3DXVECTOR3& cur_length)
	{
		int child_index = cur_node->child_node_set_index;
		NodeSet *child_set = &root_node[child_index];
		for (int i = 0; i < 8; i++)
		{
			Node *cur_child_node = &child_set->nodes[i];
			if (cur_child_node->construction_mutex == 0)
			{
				continue;
			}

			float x_index = ((i & 0x1) > 0) * 2.0f - 1.0f;
			float y_index = ((i & 0x2) > 0) * 2.0f - 1.0f;
			float z_index = ((i & 0x4) > 0) * 2.0f - 1.0f;

			D3DXVECTOR3 offset = cur_length * 0.5f;
			offset.x *= x_index;
			offset.y *= y_index;
			offset.z *= z_index;
			D3DXVECTOR3 next_middle = cur_middle + offset;

			ZEPHYR_ASSERT(child_index != 0);

			if (cur_child_node->is_leaf_index)
			{
				output.push_back(make_pair(next_middle, cur_child_node->child_node_set_index));
			}
			else
			{
				collect_tree_leaf_brick_indexes(root_node, cur_child_node, output, next_middle, cur_length * 0.5f);
			}
		}
	};

	vector<pair<D3DXVECTOR3, int>> output;
	D3DXVECTOR3 cur_middle = (my_scene_->get_bb().get_max() + my_scene_->get_bb().get_min()) * 0.5f;
	D3DXVECTOR3 cur_length = (my_scene_->get_bb().get_max() - my_scene_->get_bb().get_min());

	collect_tree_leaf_brick_indexes(&node_sets[0] , &node_sets[0].nodes[0], output, cur_middle, cur_length * 0.5f);

	float *texture_data = new float[3 * 3 * 3 * output.size() * 4];
	leaf_bricks_3d->get_data(texture_data, 3 * 3 * 3 * output.size() * 4 * sizeof(float));

	vector<Mesh::Vertex> vertices;
	vector<int> indices;
	D3DXVECTOR3 half_length = D3DXVECTOR3(cur_length.x / resolution_.x, cur_length.y / resolution_.y, cur_length.z / resolution_.z) * 0.5f;
	for (int i = 0; i < output.size(); i++)
	{
		int cur_brick_index = output[i].second;
		int cur_brick_offset = (4 * 3 * 3 * 3 * cur_brick_index);
		for (int j = 0; j < 8; j++)
		{
			D3DXVECTOR3 inside_index;
			inside_index.x = ((j & 0x1) > 0);
			inside_index.y = ((j & 0x2) > 0);
			inside_index.z = ((j & 0x4) > 0);

			D3DXVECTOR3 color = sample_brick(texture_data + cur_brick_offset, inside_index);
			float length = D3DXVec3Length(&color);

			if (length > 1e-6)
			{
				inside_index = inside_index * 2.0f - D3DXVECTOR3(1.0f, 1.0f, 1.0f);
				D3DXVECTOR3 position = output[i].first + half_length * inside_index;

				Mesh::add_cube_mesh(vertices, indices, position, half_length, D3DXVECTOR4(color,1));
			}
			else
			{
				int a = 5;
			}
		}
	}

	if (vertices.size() > 0)
	{
		Mesh *new_mesh = new Mesh();
		new_mesh->create_from_buffers(vertices, indices);

		my_scene_->clear_meshes();
		my_scene_->add_mesh(new_mesh);
	}
}

D3DXVECTOR3 GPUVoxelOcctree::sample_brick(float * brick_set, const D3DXVECTOR3 & inside_index) const
{
	std::function<int(const D3DXVECTOR3&)> get_linear_index = [&get_linear_index](const D3DXVECTOR3& index)
	{
		return 4 * (index.x + index.y * 3 * 4 + index.z * 3 * 3 * 16);
	};

	std::function<D3DXVECTOR4(float *, int)> sample_aux = [&sample_aux](float *data, int linear_index)
	{
		return D3DXVECTOR4(data[linear_index], data[linear_index + 1], data[linear_index + 2], data[linear_index + 3]);
	};

	ZEPHYR_ASSERT(inside_index.x >= 0 && inside_index.x <= 1);
	ZEPHYR_ASSERT(inside_index.y >= 0 && inside_index.y <= 1);
	ZEPHYR_ASSERT(inside_index.z >= 0 && inside_index.z <= 1);

	D3DXVECTOR4 result = D3DXVECTOR4(0, 0, 0, 0);

	D3DXVECTOR4 u000 = sample_aux(brick_set, get_linear_index(inside_index + D3DXVECTOR3(0, 0, 0)));
	D3DXVECTOR4 u001 = sample_aux(brick_set, get_linear_index(inside_index + D3DXVECTOR3(0, 0, 1)));
	D3DXVECTOR4 u010 = sample_aux(brick_set, get_linear_index(inside_index + D3DXVECTOR3(0, 1, 0)));
	D3DXVECTOR4 u011 = sample_aux(brick_set, get_linear_index(inside_index + D3DXVECTOR3(0, 1, 1)));
	D3DXVECTOR4 u100 = sample_aux(brick_set, get_linear_index(inside_index + D3DXVECTOR3(1, 0, 0)));
	D3DXVECTOR4 u101 = sample_aux(brick_set, get_linear_index(inside_index + D3DXVECTOR3(1, 0, 1)));
	D3DXVECTOR4 u110 = sample_aux(brick_set, get_linear_index(inside_index + D3DXVECTOR3(1, 1, 0)));
	D3DXVECTOR4 u111 = sample_aux(brick_set, get_linear_index(inside_index + D3DXVECTOR3(1, 1, 1)));

	bool is_cell_occupied = true;
	is_cell_occupied = is_cell_occupied && u000.w > 0;
	is_cell_occupied = is_cell_occupied && u001.w > 0;
	is_cell_occupied = is_cell_occupied && u010.w > 0;
	is_cell_occupied = is_cell_occupied && u011.w > 0;
	is_cell_occupied = is_cell_occupied && u100.w > 0;
	is_cell_occupied = is_cell_occupied && u101.w > 0;
	is_cell_occupied = is_cell_occupied && u110.w > 0;
	is_cell_occupied = is_cell_occupied && u111.w > 0;

	if (is_cell_occupied)
	{
		result += u000;
		result += u001;
		result += u010;
		result += u011;
		result += u100;
		result += u101;
		result += u110;
		result += u111;
	}

	return result * 0.125f;

}


