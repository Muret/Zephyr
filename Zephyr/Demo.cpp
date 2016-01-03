
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

DemoBase::DemoBase(string name)
{
	name_ = name;
}

DemoBase::~DemoBase()
{

}
