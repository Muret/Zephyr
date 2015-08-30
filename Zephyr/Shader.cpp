
#include "Shader.h"
#include "d11.h"

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
}

void Shader::set_shaders()
{
	SetShaders(vertex_shader, geometry_shader, pixel_shader);
}