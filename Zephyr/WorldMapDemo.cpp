
#include "WorldMapDemo.h"
#include "FreeCameraController.h"
#include "GUI.h"
#include "Mesh.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include "VoronoiSolver.h"

WorldMapDemo::WorldMapDemo() : DemoBase("WorldMapDemo")
{
	camera_controller_ = nullptr;
	last_voronoi_mesh_ = nullptr;
	seed_ = 0;
	number_of_smooth_operations_ = 0;
	smoothing_amount_ = 0.5f;
}

WorldMapDemo::~WorldMapDemo()
{
	SAFE_DELETE(camera_controller_);
	SAFE_DELETE(last_voronoi_mesh_);
	SAFE_DELETE(scene_);
}

void WorldMapDemo::initialize()
{
	camera_controller_ = new FreeCameraController();
	camera_controller_->set_directions(D3DXVECTOR4(0, 0, -1, 0), D3DXVECTOR4(0, 1, 0, 0), D3DXVECTOR4(1, 0, 0, 0));
	camera_controller_->set_position(D3DXVECTOR4(0, 0, 5, 1));

	scene_ = new Scene("World Map");
	renderer->set_scene_to_render(scene_);
	renderer->set_camera_controller(camera_controller_);

	number_of_sites_ = 10;
	create_random_world_map();
}

void WorldMapDemo::tick(float dt)
{
	camera_controller_->tick();

	ImGui::Begin("World Map Demo");
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	
	bool recaulculate = false;

	recaulculate = recaulculate || ImGui::InputInt("Seed", &seed_);
	recaulculate = recaulculate || ImGui::InputInt("Number Of Smooths", &number_of_smooth_operations_);
	recaulculate = recaulculate || ImGui::InputInt("Number Of Sites", &number_of_sites_);
	recaulculate = recaulculate || ImGui::InputFloat("Smoothing Amount", &smoothing_amount_);
	recaulculate = recaulculate || ImGui::Button("Press to create random world map");
	
	if (recaulculate && number_of_sites_ > 0)
	{
		create_random_world_map();
	}
	
	ImGui::End();
}

void WorldMapDemo::on_key_up(char key)
{
}

void WorldMapDemo::create_random_world_map()
{
	const int max_precision = RAND_MAX;
	const int half_max_precision = max_precision / 2;

	SAFE_DELETE(last_voronoi_mesh_);
	scene_->clear_meshes();

	std::vector<VoronoiSite> sites;

	float bounding_box = 10;
	Mesh *original_mesh = resource_manager.get_mesh("basicSphere");

	srand(seed_);
	for (int i = 0; i < number_of_sites_; i++)
	{
		int x_int = (rand() % half_max_precision) * 2;
		x_int -= half_max_precision;

		int y_int = (rand() % half_max_precision) * 2;
		y_int -= half_max_precision;

		float x = ((float)x_int / (float)half_max_precision);
		float y = ((float)y_int / (float)half_max_precision);

		float r = (float(rand() % 1000) / 1000.0f);
		float g = (float(rand() % 1000) / 1000.0f);
		float b = (float(rand() % 1000) / 1000.0f);

		VoronoiSite new_site;
		new_site.point = D3DXVECTOR2(x_int, y_int);
		new_site.color = D3DXVECTOR3(r, g, b);
		sites.push_back(new_site);
	}

	D3DXMATRIX voronoi_mesh_frame;
	D3DXMatrixIdentity(&voronoi_mesh_frame);
	voronoi_mesh_frame.m[0][0] = (1.0f / (float)half_max_precision);
	voronoi_mesh_frame.m[1][1] = (1.0f / (float)half_max_precision);
	voronoi_mesh_frame.m[2][2] = (1.0f / (float)half_max_precision);

	VoronoiSolver new_solver;
	new_solver.calculate(sites);
	for (int i = 0; i < number_of_smooth_operations_; i++)
	{
		sites.clear();
		new_solver.increment_uniformity(sites);
		new_solver.calculate(sites);
	}

	for (int i = 0; i < number_of_sites_; i++)
	{
		Mesh *new_mesh = new Mesh(original_mesh);

		int x_int = sites[i].point.x;
		int y_int = sites[i].point.y;

		float x = ((float)x_int / (float)half_max_precision);
		float y = ((float)y_int / (float)half_max_precision);

		D3DXMATRIX new_frame;
		D3DXMatrixIdentity(&new_frame);
		new_frame.m[0][0] = 0.005f;
		new_frame.m[1][1] = 0.005f;
		new_frame.m[2][2] = 0.005f;
		new_frame.m[3][0] = /*(int)*/x;
		new_frame.m[3][1] = /*(int)*/y;
		new_frame.m[3][2] = /*(int)*/0;

		new_mesh->set_frame(new_frame);
		scene_->add_mesh(new_mesh);
	}

	last_voronoi_mesh_ = new_solver.get_edge_line_mesh(smoothing_amount_);
	last_voronoi_mesh_->set_frame(voronoi_mesh_frame);
	scene_->add_mesh(last_voronoi_mesh_);
	//last_voronoi_mesh_->set_wireframe(true);

	

}
