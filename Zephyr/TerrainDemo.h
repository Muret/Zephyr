#ifndef _TERRAIN_DEMO_H
#define _TERRAIN_DEMO_H

#include "includes.h"
#include "Demo.h"

class SSR;
class FreeCameraController;

class TerrainDemo : public DemoBase
{
public:
	TerrainDemo();
	virtual ~TerrainDemo() override;

	virtual void initialize() override;
	virtual void tick(float dt)  override;

	virtual void on_key_up(char key) override;

	void create_terrain_mesh();

private:
	FreeCameraController *camera_controller_;
	Mesh *terrain_mesh_;
};

#endif