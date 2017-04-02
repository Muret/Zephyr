#ifndef USE_LIGHT_H_
#define USE_LIGHT_H_

#include "includes.h"

class Light
{
public:

	enum LightType
	{
		type_pointlight,
		type_directional
	};

	Light();

	void create_from_file(string name, D3DXVECTOR4 diffuse_color, D3DXVECTOR4 position, D3DXVECTOR4 direction, LightType type);

	D3DXVECTOR4 get_color() const;
	D3DXVECTOR4 get_position() const;
	D3DXVECTOR4 get_direction() const;

	LightType get_type() const
	{
		return light_type_;
	}

	string get_name() const;
private:

	D3DXVECTOR4 diffuse_color_;
	D3DXVECTOR4 position_;
	D3DXVECTOR4 direction_;

	LightType light_type_;

	std::string name_;
};


#endif