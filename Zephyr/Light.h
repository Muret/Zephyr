#ifndef USE_LIGHT_H_
#define USE_LIGHT_H_

#include "includes.h"

class Light
{
public:

	Light();

	void create_from_file(string name, D3DXVECTOR4 diffuse_color, D3DXVECTOR4 position);

	D3DXVECTOR4 get_color() const;
	D3DXVECTOR4 get_position() const;

	string get_name() const;
private:

	D3DXVECTOR4 diffuse_color_;
	D3DXVECTOR4 position_;

	std::string name_;
};


#endif