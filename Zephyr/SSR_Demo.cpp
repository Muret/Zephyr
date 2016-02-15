
#include "SSR_Demo.h"

#include "Renderer.h"
#include "SSR.h"
#include "ResourceManager.h"
#include "FreeCameraController.h"

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
	Scene *scene = resource_manager.get_scene("cornell_centered_light_middle");
	renderer->set_scene_to_render(scene);

	Camera *camera = scene->get_camera("main_camera");

	camera_controller_ = new FreeCameraController();
	camera_controller_->set_frame(camera->get_frame());
	camera_controller_->set_near(camera->get_near());
	camera_controller_->set_far(camera->get_far());
	camera_controller_->set_fov(camera->get_fov());

	ssr_component = new SSR();
	renderer->add_renderer_component(ssr_component);
	renderer->set_camera_controller(camera_controller_);
}

void SSRDemo::tick(float dt)
{
	camera_controller_->tick();
}

void SSRDemo::on_key_up(char key)
{

}
