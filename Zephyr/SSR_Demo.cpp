
#include "SSR_Demo.h"

#include "Renderer.h"
#include "SSR.h"
#include "ResourceManager.h"

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

	ssr_component = new SSR();
	renderer->add_renderer_component(ssr_component);
}

void SSRDemo::tick(float dt)
{

}
