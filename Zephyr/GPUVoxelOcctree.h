#ifndef INCLUDE_GPUVOXELCONSTRUCTOR_
#define INCLUDE_GPUVOXELCONSTRUCTOR_

#include "includes.h"

class Scene;
class Texture;
class GPUBuffer;
class Shader;

class GPUVoxelOcctree
{
public:

	struct NodeAllocationRequest
	{
		D3DXVECTOR3 position;
		D3DXVECTOR3 albedo;
		D3DXVECTOR3 normal;
		D3DXVECTOR3 ID;
	};

	struct Node
	{
		int construction_mutex;
		int brick_index;

		int child_node_set_index;
		float padding;
	};

	struct NodeSet
	{
		Node nodes[8];
	};

	struct VCTConstants
	{
		D3DXVECTOR4 g_scene_max;
		D3DXVECTOR4 g_scene_min;
		D3DXVECTOR4 g_inverse_scene_length;
		D3DXVECTOR4 g_grid_resolution_xyz_iteration_count_w;
	};


	GPUVoxelOcctree(const D3DXVECTOR3 &resolution);
	~GPUVoxelOcctree();

	void construct(Scene *scene);

private:

	void update_constant_buffer();

	Texture *x_render_texture_;
	Texture *y_render_texture_;
	Texture *z_render_texture_;

	D3DXVECTOR3 resolution_;

	VCTConstants uniform_buffer_cpu_;

	GPUBuffer *node_allocate_requests_buffer_;
	GPUBuffer *node_allocate_requests_buffer_ping_pong_;
	GPUBuffer *nodes_buffer_;
	GPUBuffer *uniform_constants_buffer_;

	Shader *deferred_cs_shader_;
	Shader *construction_shader_;
	Scene *my_scene_;
	
};

#endif