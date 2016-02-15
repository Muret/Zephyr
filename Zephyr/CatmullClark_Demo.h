#ifndef _CATMULL_CLARK_DEMO_H
#define _CATMULL_CLARK_DEMO_H

#include "includes.h"
#include "Demo.h"
#include "Mesh.h"

class CatmullClark_Demo : public DemoBase
{
public:
	CatmullClark_Demo();
	virtual ~CatmullClark_Demo() override;

	virtual void initialize() override;

	void import_scene();

	virtual void tick(float dt)  override;
	virtual void on_key_up(char key) override;
private:

	Mesh *mesh_to_edit_;
	int cc_mode_;
	int scene_mode_;
	bool wireframe_mode_;

	std::vector<Mesh::Vertex>  cube_first_vertex_buffer;
	std::vector<int>  cube_first_index_buffer;

	std::vector<Mesh::Vertex>  ico_first_vertex_buffer;
	std::vector<int>  ico_first_index_buffer;


};

#endif