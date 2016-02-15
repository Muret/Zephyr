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

	struct NodeAllocationRequestEntry
	{
		D3DXVECTOR3 position;
		D3DXVECTOR3 albedo;
		D3DXVECTOR3 normal;
		D3DXVECTOR3 ID;
	};

	GPUVoxelOcctree(const D3DXVECTOR3 &resolution);
	~GPUVoxelOcctree();

	void construct(Scene *scene);

private:

	Texture *x_render_texture_;
	Texture *y_render_texture_;
	Texture *z_render_texture_;

	D3DXVECTOR3 resolution_;

	GPUBuffer *node_allocate_requests_buffer_;
	Shader *construction_shader_;
	
};

#endif