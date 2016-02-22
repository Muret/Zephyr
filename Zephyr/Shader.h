
#ifndef __INCLUDE_SHADER_H
#define __INCLUDE_SHADER_H

#include "d11.h"

class Shader
{
public:
	Shader();
	Shader(const char* vertex_path, const char* pixel_path, const char* geometry_path = nullptr);

	static Shader* create_compute_shader(string path);

	void set_shaders();

private:

	ID3D11VertexShader* vertex_shader;
	ID3D11PixelShader* pixel_shader;
	ID3D11GeometryShader* geometry_shader;

	ID3D11ComputeShader* compute_shader;
};




#endif