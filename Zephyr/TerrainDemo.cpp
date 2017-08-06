
#include "TerrainDemo.h"

#include "Renderer.h"
#include "ResourceManager.h"
#include "FreeCameraController.h"

#include "Utilities.h"

TerrainDemo::TerrainDemo() : DemoBase("TerrainDemo")
{
	terrain_mesh_ = nullptr;
}

TerrainDemo::~TerrainDemo()
{
}

void TerrainDemo::initialize()
{
	Scene *scene = new Scene("terrain_demo");
	renderer->set_scene_to_render(scene);

	camera_controller_ = new FreeCameraController();
	camera_controller_->set_directions(D3DXVECTOR4(0, 0, -1, 0), D3DXVECTOR4(0, 1, 0, 0), D3DXVECTOR4(1, 0, 0, 0));
	camera_controller_->set_position(D3DXVECTOR4(0,0.5,0,0));

	renderer->set_camera_controller(camera_controller_);

	create_terrain_mesh();
}

void TerrainDemo::tick(float dt)
{
	camera_controller_->tick();
}

void TerrainDemo::on_key_up(char key)
{

}

void TerrainDemo::create_terrain_mesh()
{
	Mesh *mesh = new Mesh();

	vector<Mesh::Vertex> vertices;
	vector<int> indices;
	
	int tesselation_factor = 16;
	for (int i = 0; i < tesselation_factor; i++)
	{
		for (int i = 0; i < tesselation_factor; i++)
		{

		}
	}

}

