

#ifndef __INCLUDE_RENDERER_H
#define __INCLUDE_RENDERER_H

#include <vector>
#include "Mesh.h"

class RenderComponent;
class Shader;
class Scene;
class Camera;

struct FrameConstantsBuffer
{
	D3DXMATRIX view_matrix;
	D3DXMATRIX projection_matrix;
	D3DXMATRIX view_projection_matrix;

	D3DXMATRIX inv_view_matrix;
	D3DXMATRIX inv_projection_matrix;
	D3DXMATRIX inv_view_projection_matrix;

	D3DXVECTOR4 right_direction;
	D3DXVECTOR4 up_direction;
	D3DXVECTOR4 view_direction;
	D3DXVECTOR4 camera_position;

	D3DXVECTOR4 screen_texture_half_pixel_forced_mipmap;
	D3DXVECTOR4 near_far_padding2;
	D3DXVECTOR4 debug_vector;
};

struct MeshConstantsBuffer
{
	D3DXMATRIX world_matrix;
	D3DXMATRIX world_view_matrix;
	D3DXMATRIX world_view_projection_matrix;

	D3DXMATRIX inv_world_matrix;
	D3DXMATRIX inv_world_view_matrix;
	D3DXMATRIX inv_world_view_projection_matrix;

	D3DXVECTOR4 diffuse_color;
	D3DXVECTOR4 bb_min;
	D3DXVECTOR4 bb_max;
};

struct LightingConstantsBuffer
{
	D3DXVECTOR4 ws_light_position;
	D3DXVECTOR4 ss_light_position;
	D3DXVECTOR4 light_color;
};

class Renderer
{
public:
	Renderer();

	void render_frame();

	void add_renderer_component(RenderComponent *component);

	void set_scene_to_render(Scene * scene);

	void set_camera_controller(Camera *cam);

	void render_mesh(const Mesh* mesh, const Camera &cam);

	void set_mesh_primitive_topology(const Mesh* mesh);

protected:
	void begin_frame();

	void pre_render();
	virtual void gbuffer_render();
	void main_render();

	void forward_rendering_pipeline();

	void post_render();

	void end_frame();

	void validate_render_options();
	void full_deferred_rendering_pipeline();

	void set_frame_constant_values();
	void set_lighting_constant_values();
	void set_mesh_constant_values(const Mesh *mesh);

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

	//Camera stuff
	Camera *camera_;

	//uniform constants
	LightingConstantsBuffer lighting_contants_buffer_cpu;
	MeshConstantsBuffer mesh_constants_buffer_cpu;
	FrameConstantsBuffer frame_constans_buffer_cpu;

	ID3D11Buffer *lighting_constants_buffer_gpu;
	ID3D11Buffer *mesh_constants_buffer_gpu;
	ID3D11Buffer *frame_constans_buffer_gpu;

};

extern Renderer* renderer;


#endif