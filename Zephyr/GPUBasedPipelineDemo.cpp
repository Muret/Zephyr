
#include "GPUBasedPipelineDemo.h"

#include "Renderer.h"
#include "SSR.h"
#include "ResourceManager.h"
#include "FreeCameraController.h"

#include "GPUBasedPipeline.h"


GPUBasedPipelineDemo::GPUBasedPipelineDemo() : DemoBase("GPUBasedPipeline")
{
	renderer_ = nullptr;
}

GPUBasedPipelineDemo::~GPUBasedPipelineDemo()
{
	delete renderer_;
}

void GPUBasedPipelineDemo::initialize()
{
	init_state();
}

void GPUBasedPipelineDemo::tick(float dt)
{
	camera_controller_->tick();

	static float f = 0.0f;
	ImGui::Text("Hello, world!");
	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
	ImGui::ColorEdit3("clear color", (float*)&clear_col);
	if (ImGui::Button("Test Window")) show_test_window ^= 1;
	if (ImGui::Button("Another Window")) show_another_window ^= 1;
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}

void GPUBasedPipelineDemo::on_key_up(char key)
{

}

void GPUBasedPipelineDemo::init_state()
{
	Scene *scene = resource_manager.get_scene("tengri_test");
	renderer->set_scene_to_render(scene);

	GPUBasedPipeline::GPUBasedRenderer *t_renderer = new GPUBasedPipeline::GPUBasedRenderer();
	t_renderer->set_scene(scene);

	Camera *camera = scene->get_camera("main_camera");

	camera_controller_ = new FreeCameraController();

	if (camera != nullptr)
	{
		camera_controller_->set_frame(camera->get_frame());
		camera_controller_->set_near(camera->get_near());
		camera_controller_->set_far(camera->get_far());
		camera_controller_->set_fov(camera->get_fov());
	}

	renderer->set_camera_controller(camera_controller_);

	show_test_window = true;
	show_another_window = false;
	clear_col = ImColor(114, 144, 154);

}
