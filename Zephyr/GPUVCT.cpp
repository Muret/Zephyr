
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
	octree_ = new GPUVoxelOcctree(D3DXVECTOR3(4, 4, 4));
}

void GPUVCT::pre_render()
{
	static int i = 0; 
	if (i == 1)
	{
		octree_->construct(my_scene_);
	}
	i++;
}

void GPUVCT::post_render()
{
}

void GPUVCT::post_gbuffer_render()
{
	//octree_->construct(my_scene_);
}

bool GPUVCT::uses_postfx() const
{
	return false;
}
