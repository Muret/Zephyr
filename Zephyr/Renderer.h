

#ifndef __INCLUDE_RENDERER_H
#define __INCLUDE_RENDERER_H

#include <vector>
#include "Mesh.h"
#include "TextureQuadTree.h"

class RenderComponent;
class Shader;
class Scene;
class Camera;
class TextureQuadTree;
class GPUBuffer;
class DepthTexture;

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

	D3DXVECTOR4 screen_tile_size;
	D3DXVECTOR4 screen_tile_info[128];
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
	D3DXVECTOR4 current_tile_info;
};

struct LightingConstantsBuffer
{
	D3DXVECTOR4 ws_light_position;
	D3DXVECTOR4 ss_light_position;
	D3DXVECTOR4 light_color;
	D3DXMATRIX light_view_projection_matrix;
	D3DXMATRIX light_view_projection_matrix_inv;
};

class Renderer
{
public:
	struct DrawRecord
	{
		DrawRecord(Mesh *m, const D3DXMATRIX& frm) : mesh(m), frame(frm) {}

		Mesh *mesh;
		D3DXMATRIX frame;

		vector<pair<int,int>> tiles_;
	};

	Renderer();

	void initialize_msaa_deferred_renderer();

	void initialize_tiled_shadow_renderer();

	void creaate_light_shadow_depth_texture();

	void render_frame();

	void add_renderer_component(RenderComponent *component);

	void set_scene_to_render(Scene * scene);

	void set_camera_controller(Camera *cam);

	void render_mesh(const Mesh* mesh, const Camera &cam);

	void set_mesh_primitive_topology(const Mesh* mesh);

	void light_shadow_render();

	void tiled_light_shadow_render();

	void get_light_shadow_view_proj_matrix(D3DXMATRIX &lvp);

protected:
	void begin_frame();

	void refresh_tile_resolutions();
	void pre_render();
	void tick_tilesets();
	void show_imgui();

	virtual void gbuffer_render();
	void default_gbuffer_render_loop();
	void tiled_gbuffer_render_loop();

	void main_render();

	void forward_rendering_pipeline();

	void post_render();

	void end_frame();

	void validate_render_options();
	void full_deferred_rendering_pipeline();

	void set_frame_constant_values();
	void set_lighting_constant_values();
	void set_mesh_constant_values(const DrawRecord& draw_rec);

	vector<RenderComponent*> render_components;

	Shader *default_render_shader;

	Scene *scene_to_render;

	//geometry buffer
	Texture *gbuffer_normal_texture;
	Texture *gbuffer_albedo_texture;
	Texture *gbuffer_specular_texture;

	ID3D11DepthStencilView* light_depth_texture_view_;
	ID3D11ShaderResourceView* light_depth_texture_srv_;
	ID3D11Texture2D* light_depth_texture_;

	DepthTexture* scene_depth_target_;

	GPUBuffer* per_tile_render_info_;
	GPUBuffer* cleaner_staging_buffer_;

	Texture *ds_gbuffer_texture_normal[2];
	Texture *ds_gbuffer_texture_albedo[2];
	Texture *ds_gbuffer_texture_specular[2];
	int cur_gbuffer_index_to_render_;

	Shader *gbuffer_shader;
	Shader *light_shadow_shader;
	Shader *full_deferred_diffuse_lighting_shader;
	Shader *full_deferred_diffuse_lighting_shader_per_msaa_sample;

	Texture *screen_texture;

	//render options
	bool use_postfx;
	bool do_flicker;

	//Camera stuff
	Camera *camera_;

	Texture *previous_frame_light_buffer_[2];
	TextureQuadTree* gbuffer_render_quad_tree_[2];
	map<pair<int, int>, TextureQuadTree::Tile> current_tile_data_[2];
	int current_tile_index_;
	int tile_render_data_size_;
	int *tile_render_data_;

	//uniform constants
	LightingConstantsBuffer lighting_contants_buffer_cpu;
	MeshConstantsBuffer mesh_constants_buffer_cpu;
	FrameConstantsBuffer frame_constans_buffer_cpu;

	ID3D11Buffer *lighting_constants_buffer_gpu;
	ID3D11Buffer *mesh_constants_buffer_gpu;
	ID3D11Buffer *frame_constans_buffer_gpu;

	vector<DrawRecord> cur_frame_rendered_meshes_;

	int tile_size_;
	int tile_count_x_;
	int tile_count_y_;
	float *score_per_tile_;

	int draw_one_by_one_index_;
};

extern Renderer* renderer;


#endif