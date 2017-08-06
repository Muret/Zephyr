
#include "SSR_Demo.h"

#include "Renderer.h"
#include "SSR.h"
#include "ResourceManager.h"
#include "FreeCameraController.h"

#include "GPUBasedPipeline.h"
#include "MeshGroup.h"
#include "Utilities.h"

SSRDemo::SSRDemo() : DemoBase("SSR")
{
	ssr_component = nullptr;
}

SSRDemo::~SSRDemo()
{
	delete ssr_component;
}

void SSRDemo::initialize()
{
	//ssr_component = new SSR();
	//renderer->add_renderer_component(ssr_component);

	Scene *scene = resource_manager.get_scene("ktm_bike_scene"); //new Scene("demo");
	renderer->set_scene_to_render(scene);

	Mesh *lion = scene->get_mesh("Mesh_102");

	Camera *start_camera = scene->get_camera("camera1");

	camera_controller_ = new FreeCameraController();

	//camera_controller_->set_frame(start_camera->get_frame());
	//camera_controller_->set_near(start_camera->get_near());
	//camera_controller_->set_far(start_camera->get_far());
	//camera_controller_->set_fov(start_camera->get_fov());

	renderer->set_camera_controller(camera_controller_);

	if(false)
	{
		Scene *lod_scene = resource_manager.get_scene("lod_meshes");
		if (lod_scene)
		{
			const vector<MeshGroup*> &mesh_groups = lod_scene->get_mesh_groups();
			const int total_mesh_count = 10000;
			const int instance_per_mesh_count = total_mesh_count / mesh_groups.size();
			D3DXVECTOR3 bb_min = D3DXVECTOR3(-10, 0, 0);
			D3DXVECTOR3 bb_max = D3DXVECTOR3(10, 10, -50);
			D3DXVECTOR3 bb_vec = bb_max - bb_min;

			for (int i = 0; i < mesh_groups.size(); i++)
			{
				for (int j = 0; j < instance_per_mesh_count; j++)
				{
					MeshGroup* group = new MeshGroup(mesh_groups[i]);
					D3DXVECTOR3 pos = bb_min + D3DXVECTOR3(Utilities::random_normalized_float(), Utilities::random_normalized_float(), Utilities::random_normalized_float()) * bb_vec;

					scene->add_mesh_group(group);

					D3DXMATRIX frm;
					D3DXMatrixIdentity(&frm);
					frm.m[3][0] = pos.x;
					frm.m[3][1] = pos.y; 
					frm.m[3][2] = pos.z;
					group->set_frame(frm);
				}
			}
		}
	}


}

void SSRDemo::tick(float dt)
{
	camera_controller_->tick();
}

void SSRDemo::on_key_up(char key)
{

}

