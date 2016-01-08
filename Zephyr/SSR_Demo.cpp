
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
	vector<Mesh*> meshes;
	resource_manager.get_meshes_with_filter("sponza", meshes);
	renderer->add_meshes_to_render(meshes);

	ssr_component = new SSR();
	renderer->add_renderer_component(ssr_component);
}

void SSRDemo::tick(float dt)
{

}
