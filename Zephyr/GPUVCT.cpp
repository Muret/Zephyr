
#include "GPUVCT.h"
#include "GPUVoxelOcctree.h"

GPUVCT::GPUVCT()
{
	octree_ = nullptr;
	my_scene_ = nullptr;

}

void GPUVCT::initialize(Scene * scene)
{
	my_scene_ = scene;;
	octree_ = new GPUVoxelOcctree(D3DXVECTOR3(16, 16, 16));
	octree_->construct(scene);
}

void GPUVCT::pre_render()
{
}

void GPUVCT::post_render()
{
}

void GPUVCT::post_gbuffer_render()
{
	octree_->construct(my_scene_);
}

bool GPUVCT::uses_postfx() const
{
	return false;
}
