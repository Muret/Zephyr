
#include "Shader.h"
#include "d11.h"

Shader::Shader()
{
	vertex_shader		= nullptr;
	pixel_shader		= nullptr;
	geometry_shader		= nullptr;
	compute_shader		= nullptr;
}

Shader::Shader(const char* vertex_path, const char* pixel_path, const char* geometry_path /*= nullptr*/)
{
	vertex_shader = CreateVertexShader(vertex_path);
	pixel_shader = CreatePixelShader(pixel_path);

	if (geometry_path)
	{
		geometry_shader = CreateGeometryShader(geometry_path);
	}
	else
	{
		geometry_shader = nullptr;
	}

	compute_shader = nullptr;
}

Shader * Shader::create_compute_shader(string path)
{
	Shader *new_shader = new Shader();
	new_shader->compute_shader = CreateComputeShader(path.c_str());
	return new_shader;
}

void Shader::set_shaders()
{
	SetShaders(vertex_shader, geometry_shader, pixel_shader);
	SetComputeShader(compute_shader);
}