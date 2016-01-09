

#ifndef __INCLUDE_RENDERER_H
#define __INCLUDE_RENDERER_H

#include <vector>
#include "Mesh.h"

class RenderComponent;
class Shader;
class Scene;

class Renderer
{

public:

	Renderer();

	void render_frame();

	void add_renderer_component(RenderComponent *component);

	void set_scene_to_render(Scene * scene);
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

	vector<RenderComponent*> render_components;

	Shader *default_render_shader;

	Scene *scene_to_render;

	//geometry buffer
	Texture *gbuffer_normal_texture;
	Texture *gbuffer_albedo_texture;
	Texture *gbuffer_specular_texture;

	Shader *gbuffer_shader;
	Shader *full_deferred_diffuse_lighting_shader;

	Texture *screen_texture;

	//render options
	bool use_postfx;


};

extern Renderer* renderer;


#endif