#include "includes.h"

#include "Light.h"

Light::Light() : diffuse_color_(0, 0, 0, 0), position_(0, 0, 0, 0), name_("")
{
}

void Light::create_from_file(string name, D3DXVECTOR4 diffuse_color, D3DXVECTOR4 position, D3DXVECTOR4 direction, LightType type)
{
	name_ = name;
	diffuse_color_ = diffuse_color;
	position_ = position;
	direction_ = direction;
	light_type_ = type;
}

D3DXVECTOR4 Light::get_color() const
{
	return diffuse_color_;
}

D3DXVECTOR4 Light::get_position() const
{
	return position_;
}

D3DXVECTOR4 Light::get_direction() const
{
	return direction_;
}

std::string Light::get_name() const
{
	return name_;
}
