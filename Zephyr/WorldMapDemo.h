#ifndef _WORLDMAP_DEMO_H
#define _WORLDMAP_DEMO_H

#include "includes.h"
#include "Demo.h"
#include "VoronoiSolver.h"
#include "World.h"

using namespace Evolution;

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
	int number_of_sites_;
	int number_of_smooth_operations_;
	float smoothing_amount_;
	
	int seed_;
	int noise_octave_count_;
	float noise_frequency_;
	float center_height_modifier_;
	float center_height_factor_;

	bool show_site_centers_;

	std::vector<VoronoiSolver::VoronoiSite> cur_sites_;
	VoronoiSolver cur_solver_;

	void create_random_world_map();
	void recolor_world_map();
	D3DXVECTOR4 get_color_of_point(float value) const;

	void read_voronoi_from_file(std::string name);

	static const int max_precision;
	static const int half_max_precision;
	static const float inv_half_max_precision;

	//Evolution
	WorldInstance *world_instance_;

};

#endif