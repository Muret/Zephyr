
#ifndef __INCLUDE_SHADER_H
#define __INCLUDE_SHADER_H

#include "d11.h"

class Shader
{
public:
	Shader(const char* vertex_path, const char* pixel_path, const char* geometry_path = nullptr);

	void set_shaders();

private:

	ID3D11VertexShader* vertex_shader;
	ID3D11PixelShader* pixel_shader;
	ID3D11GeometryShader* geometry_shader;
};




#endif