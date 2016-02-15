
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

DemoBase::DemoBase(string name)
{
	name_ = name;
}

DemoBase::~DemoBase()
{

}
