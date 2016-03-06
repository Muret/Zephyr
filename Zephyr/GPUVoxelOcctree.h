#ifndef INCLUDE_GPUVOXELCONSTRUCTOR_
#define INCLUDE_GPUVOXELCONSTRUCTOR_

#include "includes.h"
#include "Camera.h"

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
		int is_leaf_index;

		int child_node_set_index;
		int is_constructed;
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
		D3DXVECTOR4 g_inverse_leaf_dimension;
	};


	GPUVoxelOcctree(const D3DXVECTOR3 &resolution);
	~GPUVoxelOcctree();

	void construct(Scene *scene);
	void build_octree_from_side_aux(const Camera &cam);

private:

	void update_constant_buffer();
	void create_debug_render_mesh();

	D3DXVECTOR3 sample_brick(float *brick_set, const D3DXVECTOR3 &inside_index) const;

	Texture *x_render_texture_;
	Texture *y_render_texture_;
	Texture *z_render_texture_;

	Texture *leaf_bricks_3d;

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