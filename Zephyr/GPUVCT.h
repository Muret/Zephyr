#ifndef __INCLUDE_GPUVCT_
#define __INCLUDE_GPUVCT_

#include "includes.h"
#include "RenderComponent.h"

class GPUVoxelOcctree;
class Scene;

// cone traced ssr
class GPUVCT : public RenderComponent
{
public:
	GPUVCT();

	void initialize(Scene *scene);

	virtual void pre_render();
	virtual void post_render();
	virtual void post_gbuffer_render();

	virtual bool uses_postfx() const;

private:

	GPUVoxelOcctree *octree_;
	Scene *my_scene_;
};


#endif