
#include "CatmullClark_Demo.h"
#include "CatmullClark.h"

#include "Mesh.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Camera.h"


CatmullClark_Demo::CatmullClark_Demo() : DemoBase("CatmullClark")
{
	cc_mode_ = 1;
	scene_mode_ = 0;
	wireframe_mode_ = false;
}

CatmullClark_Demo::~CatmullClark_Demo()
{
}

void CatmullClark_Demo::initialize()
{
	const string mesh_name = "smoothed_mesh";
	Scene *scene = nullptr;
	Mesh *mesh = nullptr;

	{
		scene = resource_manager.get_scene("cornell_box_catmull_clark_mesh3");
		mesh = scene->get_mesh(mesh_name);
		CatmullClark smoother(mesh, cc_mode_);
		smoother.subdivide(0);
		ico_first_vertex_buffer = mesh->get_vertices();
		ico_first_index_buffer = mesh->get_indices();
	}

	{
		scene = resource_manager.get_scene("cornell_box_catmull_clark");
		mesh = scene->get_mesh(mesh_name);
		CatmullClark smoother(mesh, cc_mode_);
		smoother.subdivide(0);
		cube_first_vertex_buffer = mesh->get_vertices();
		cube_first_index_buffer = mesh->get_indices();
	}

	import_scene();
}

void CatmullClark_Demo::import_scene()
{
	const string mesh_name = "smoothed_mesh";
	Scene *scene = nullptr;

	if (scene_mode_ == 0)
	{
		scene = resource_manager.get_scene("cornell_box_catmull_clark_mesh3");
	}
	else
	{
		scene = resource_manager.get_scene("cornell_box_catmull_clark");
	}

	renderer->set_scene_to_render(scene);


	mesh_to_edit_ = scene->get_mesh(mesh_name);
	if (cc_mode_ == 2)
	{
		mesh_to_edit_->set_color_multiplier(D3DXVECTOR4(1, 0, 0, 1));
	}
	else
	{
		mesh_to_edit_->set_color_multiplier(D3DXVECTOR4(0, 0, 1, 1));
	}
	mesh_to_edit_->set_wireframe(wireframe_mode_);
}

void CatmullClark_Demo::tick(float dt)
{

	
}

void CatmullClark_Demo::on_key_up(char key)
{
	if (key == 'M')
	{
		CatmullClark smoother(mesh_to_edit_, cc_mode_);
		smoother.subdivide(1);
	}
	else if (key == 'N')
	{
		cc_mode_ = cc_mode_ == 1 ? 2 : 1;

		if (cc_mode_ == 2)
		{
			mesh_to_edit_->set_color_multiplier(D3DXVECTOR4(1, 0, 0, 1));
		}
		else
		{
			mesh_to_edit_->set_color_multiplier(D3DXVECTOR4(0, 0, 1, 1));
		}
	}
	else if (key == 'B')
	{
		scene_mode_ = !scene_mode_;
		import_scene();
	}
	else if (key == 'V')
	{
		wireframe_mode_ = !wireframe_mode_;
		mesh_to_edit_->set_wireframe(wireframe_mode_);
	}
	else if (key == 'C')
	{
		Scene *scene = nullptr;
		Mesh *mesh = nullptr;
		const string mesh_name = "smoothed_mesh";

		{
			scene = resource_manager.get_scene("cornell_box_catmull_clark_mesh3");
			scene->get_mesh(mesh_name)->create_from_buffers(ico_first_vertex_buffer, ico_first_index_buffer);
		}

		{
			scene = resource_manager.get_scene("cornell_box_catmull_clark");
			scene->get_mesh(mesh_name)->create_from_buffers(cube_first_vertex_buffer, cube_first_index_buffer);
		}

	}
}
