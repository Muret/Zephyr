#ifndef _WORLDMAP_DEMO_H
#define _WORLDMAP_DEMO_H

#include "includes.h"
#include "Demo.h"

class FreeCameraController;
class Scene;

class WorldMapDemo : public DemoBase
{
public:
	WorldMapDemo();
	virtual ~WorldMapDemo() override;

	virtual void initialize() override;
	virtual void tick(float dt)  override;

	virtual void on_key_up(char key) override;

private:
	FreeCameraController *camera_controller_;
	Mesh *last_voronoi_mesh_;
	Scene *scene_;
	int seed_;
	int number_of_sites_;
	int number_of_smooth_operations_;

	void create_random_world_map();
};

#endif