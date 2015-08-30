
#include "Mesh.h"
#include "TextureLoader.h"
#include "Demo.h"
#include "Texture.h"
#include "Material.h"

#include <assert.h>
#include "Camera.h"

#include "Renderer.h"
#include "SSR.h"
#include "d11.h"

std::vector<Mesh*> demo_meshes;
Camera demo_camera;

void init_demo_scene()
{
	string fbx_path("..\\Resources\\Oilbarell\\oilbarrel.fbx");
	FBXImporter importer(fbx_path, demo_meshes);

	renderer->add_meshes_to_render(demo_meshes);

	SSR *ssr_render_component = new SSR();
	renderer->add_renderer_component(ssr_render_component);

	lighting_InfoBuffer_cpu.light_direction = D3DXVECTOR4(0, 0, -1, 0);
	lighting_InfoBuffer_cpu.light_color = D3DXVECTOR4(1, 1, 1, 1);
}

