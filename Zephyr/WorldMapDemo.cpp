
#include "WorldMapDemo.h"
#include "FreeCameraController.h"
#include "GUI.h"
#include "Mesh.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include "Noise.h"

const int WorldMapDemo::max_precision = RAND_MAX;
const int WorldMapDemo::half_max_precision = max_precision / 2;
const float WorldMapDemo::inv_half_max_precision = 1.0f / half_max_precision;

WorldMapDemo::WorldMapDemo() : DemoBase("WorldMapDemo")
{
	camera_controller_ = nullptr;
	last_voronoi_mesh_ = nullptr;
	seed_ = 0;
	number_of_smooth_operations_ = 4;
	smoothing_amount_ = 0.5f;
	noise_octave_count_ = 5;
	center_height_modifier_ = 1.0f;
	show_site_centers_ = false;
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
	camera_controller_->set_position(D3DXVECTOR4(0, 0, 3, 1));

	scene_ = new Scene("World Map");
	renderer->set_scene_to_render(scene_);
	renderer->set_camera_controller(camera_controller_);

	number_of_sites_ = 7000;
	//create_random_world_map();

	//read_voronoi_from_file("voronoi.bin");
}

void WorldMapDemo::tick(float dt)
{
	camera_controller_->tick();

	ImGui::Begin("World Map Demo");
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	
	bool recaulculate = false;
	bool recolor = false;

	ImGui::Columns(3, "Mixed");

	recaulculate = recaulculate || ImGui::InputInt("Number Of Smooths", &number_of_smooth_operations_);
	recaulculate = recaulculate || ImGui::InputInt("Number Of Sites", &number_of_sites_);
	recaulculate = recaulculate || ImGui::InputFloat("Smoothing Amount", &smoothing_amount_);

	ImGui::NextColumn();

	recolor = recolor || ImGui::Checkbox("Show Site Centers", &show_site_centers_);
	recolor = recolor || ImGui::InputInt("Noise Octaves", &noise_octave_count_);
	recolor = recolor || ImGui::InputInt("Seed", &seed_);
	recolor = recolor || ImGui::InputFloat("Noise Frequency", &noise_frequency_);
	recolor = recolor || ImGui::InputFloat("Center Height Modifier", &center_height_modifier_);
	recolor = recolor || ImGui::InputFloat("Center Height Factor", &center_height_factor_);
	
	ImGui::NextColumn();

	if (ImGui::Button("Create Voronoi"))
	{
		create_random_world_map();
	}

	if (ImGui::Button("Save Voronoi"))
	{
		cur_solver_.write_to_file("voronoi.bin");
	}

	if (ImGui::Button("Load Voronoi"))
	{
		read_voronoi_from_file("voronoi.bin");
	}



	if (recolor)
	{
		recolor_world_map();
	}

	ImGui::End();
}

void WorldMapDemo::on_key_up(char key)
{
}

void WorldMapDemo::create_random_world_map()
{
	float bounding_box = 10;

	srand(seed_);
	for (int i = 0; i < number_of_sites_; i++)
	{
		int x_int = (rand() % half_max_precision) * 2;
		x_int -= half_max_precision;
	
		int y_int = (rand() % half_max_precision) * 2;
		y_int -= half_max_precision;
	
		float x = (float)x_int * inv_half_max_precision;
		float y = (float)y_int * inv_half_max_precision;
	
		VoronoiSolver::VoronoiSite new_site;
		new_site.point = D3DXVECTOR2(x_int, y_int);
		new_site.color = D3DXVECTOR3(0, 0, 0);
		cur_sites_.push_back(new_site);
	}
	
	cur_solver_.calculate(cur_sites_);
	for (int i = 0; i < number_of_smooth_operations_; i++)
	{
		cur_sites_.clear();
		cur_solver_.increment_uniformity(cur_sites_);
		cur_solver_.calculate(cur_sites_);
	}

	recolor_world_map();
}

void WorldMapDemo::recolor_world_map()
{
	SAFE_DELETE(last_voronoi_mesh_);
	scene_->clear_meshes();

	NoiseInstance::NoiseGeneratorParams noise_params;
	noise_params.seed_ = seed_;
	noise_params.number_of_octaves_ = noise_octave_count_;
	noise_params.noise_frequency = noise_frequency_;

	NoiseInstance noise_gen;
	noise_gen.build_2d_heightmap(D3DXVECTOR2(-1,-1), D3DXVECTOR2(1,1), D3DXVECTOR2(2048, 1024), noise_params);

	std::vector<D3DXVECTOR3> new_colors;
	for (int i = 0; i < number_of_sites_; i++)
	{
		D3DXVECTOR2 current_position = cur_sites_[i].point * inv_half_max_precision;
		float distance_to_center = abs(D3DXVec2Dot(&current_position, &current_position));

		float height = noise_gen.get_heightmap_value(current_position);

		D3DXVECTOR4 color = get_color_of_point(height);
		new_colors.push_back(D3DXVECTOR3(color.x, color.y, color.z));

	}

	cur_solver_.set_site_colors(new_colors);

	if (show_site_centers_)
	{
		Mesh *original_mesh = resource_manager.get_mesh("basicSphere");
		for (int i = 0; i < number_of_sites_; i++)
		{
			Mesh *new_mesh = new Mesh(original_mesh);

			int x_int = cur_sites_[i].point.x;
			int y_int = cur_sites_[i].point.y;

			float x = (float)x_int * inv_half_max_precision;
			float y = (float)y_int * inv_half_max_precision;

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
	}

	D3DXMATRIX voronoi_mesh_frame;
	D3DXMatrixIdentity(&voronoi_mesh_frame);
	voronoi_mesh_frame.m[0][0] = (1.0f / (float)half_max_precision);
	voronoi_mesh_frame.m[1][1] = (1.0f / (float)half_max_precision);
	voronoi_mesh_frame.m[2][2] = (1.0f / (float)half_max_precision);

	last_voronoi_mesh_ = cur_solver_.get_triangulated_voronoi_mesh();
	//last_voronoi_mesh_ = cur_solver_.get_edge_line_mesh(smoothing_amount_);
	last_voronoi_mesh_->set_frame(voronoi_mesh_frame);
	scene_->add_mesh(last_voronoi_mesh_);
	//last_voronoi_mesh_->set_wireframe(true);
}

D3DXVECTOR4 WorldMapDemo::get_color_of_point(float value) const
{
	struct ColorPoint
	{
		ColorPoint(float s, float e, D3DXVECTOR3 c)
		{
			start = s;
			end = e;
			color = c;
		}

		float start;
		float end;
		D3DXVECTOR3 color;
	};

	static std::vector<ColorPoint> points;
	points.push_back(ColorPoint(-1, -0.25, D3DXVECTOR3(0, 0, 128) / 255.0f));
	points.push_back(ColorPoint(-0.25, 0, D3DXVECTOR3(0, 255, 255) / 255.0f));
	points.push_back(ColorPoint(0, 0.0625, D3DXVECTOR3(0, 128, 255) / 255.0f));
	points.push_back(ColorPoint(0.0625, 0.125, D3DXVECTOR3(240, 240, 64) / 255.0f));
	points.push_back(ColorPoint(0.125, 0.3750, D3DXVECTOR3(32, 160, 0) / 255.0f));
	points.push_back(ColorPoint(0.3750, 0.750, D3DXVECTOR3(224, 224, 0) / 255.0f));
	points.push_back(ColorPoint(0.750, 1, D3DXVECTOR3(128, 128, 128) / 255.0f));
	points.push_back(ColorPoint(1, 10, D3DXVECTOR3(255, 255, 255) / 255.0f));

	for (int i = 0; i < points.size() - 1; i++)
	{
		if (points[i].start <= value && points[i].end > value)
		{
			float blend = (value - points[i].start) / (points[i].end - points[i].start);
			return points[i].color * (1.0f - blend) + points[i + 1].color * blend;
		}
	}

	return points.back().color;
}

void WorldMapDemo::read_voronoi_from_file(std::string name)
{
	SAFE_DELETE(last_voronoi_mesh_);
	scene_->clear_meshes();

	std::vector<VoronoiSolver::VoronoiCellInfo> sites;
	cur_solver_.read_from_file(name, sites);

	NoiseInstance::NoiseGeneratorParams noise_params;
	noise_params.seed_ = seed_;
	noise_params.number_of_octaves_ = noise_octave_count_;
	noise_params.noise_frequency = noise_frequency_;

	NoiseInstance noise_gen;
	noise_gen.build_2d_heightmap(D3DXVECTOR2(-1, -1), D3DXVECTOR2(1, 1), D3DXVECTOR2(2048, 1024), noise_params);

	std::vector<D3DXVECTOR3> new_colors;
	for (int i = 0; i < sites.size(); i++)
	{
		float height = noise_gen.get_heightmap_value(sites[i].center * inv_half_max_precision);

		D3DXVECTOR4 color = get_color_of_point(height);
		new_colors.push_back(D3DXVECTOR3(color.x, color.y, color.z));
	}

	cur_solver_.set_site_colors(new_colors);

	D3DXMATRIX voronoi_mesh_frame;
	D3DXMatrixIdentity(&voronoi_mesh_frame);
	voronoi_mesh_frame.m[0][0] = (1.0f / (float)half_max_precision);
	voronoi_mesh_frame.m[1][1] = (1.0f / (float)half_max_precision);
	voronoi_mesh_frame.m[2][2] = (1.0f / (float)half_max_precision);

	last_voronoi_mesh_ = cur_solver_.get_triangulated_voronoi_mesh(sites);
	last_voronoi_mesh_->set_frame(voronoi_mesh_frame);
	scene_->add_mesh(last_voronoi_mesh_);

}

