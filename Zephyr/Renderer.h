

#ifndef __INCLUDE_RENDERER_H
#define __INCLUDE_RENDERER_H

#include <vector>
#include "Mesh.h"

class RenderComponent;
class Shader;

class Renderer
{

public:

	Renderer();

	void render_frame();

	void add_mesh_to_render(Mesh* mesh);
	void add_meshes_to_render(vector<Mesh*> &mesh);
	void add_renderer_component(RenderComponent *component);

private:
	void begin_frame();

	void pre_render();
	void gbuffer_render();
	void main_render();

	void forward_rendering_pipeline();

	void post_render();

	void end_frame();

	void validate_render_options();
	void full_deferred_rendering_pipeline();
	vector<Mesh*> meshes_to_render;
	vector<RenderComponent*> render_components;

	Shader *default_render_shader;

	//geomtry buffer
	Texture *gbuffer_normal_texture;
	Texture *gbuffer_albedo_texture;
	Texture *gbuffer_specular_texture;

	Shader *gbuffer_shader;
	Shader *full_deferred_diffuse_lighting_shader;

	Texture *screen_texture;

	//render opsitons
	bool use_postfx;


};

extern Renderer* renderer;


#endif