
#include "VCT_Demo.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include "FreeCameraController.h"
#include "GPUVCT.h"

VCTDemo::VCTDemo() : DemoBase("Voxel Cone Tracing")
{
	vct_ = nullptr;
	camera_controller_ = nullptr;
}

VCTDemo::~VCTDemo()
{

}

void VCTDemo::initialize()
{
	Scene *scene = resource_manager.get_scene("stanford_bunny");
	renderer->set_scene_to_render(scene); 

	Camera *camera = scene->get_camera("main_camera");

	camera_controller_ = new FreeCameraController();
	camera_controller_->set_frame(camera->get_frame());
	camera_controller_->set_near(camera->get_near());
	camera_controller_->set_far(camera->get_far());
	camera_controller_->set_fov(camera->get_fov());

	vct_ = new GPUVCT();
	vct_->initialize(scene);

	renderer->add_renderer_component(vct_);
	renderer->set_camera_controller(camera_controller_);
}

void VCTDemo::tick(float dt)
{
	camera_controller_->tick();
}

void VCTDemo::on_key_up(char key)
{

}
