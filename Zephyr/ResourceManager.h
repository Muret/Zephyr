#ifndef INCLUDE_RESOURCE_MANAGER_
#define INCLUDE_RESOURCE_MANAGER_

#include "includes.h"

#include "BoundingBox.h"
#include "Renderer.h"

class Texture;
class Mesh;
class Light;
class Scene;
class Camera;
class MeshGroup;

class ResourceManager
{
public:

	ResourceManager();
	~ResourceManager();

	void init_resources(string resource_foler);

	void init_textures(string resource_foler);
	void init_meshes(string resource_foler);
	
	Texture* get_texture(string name) const;
	Mesh* get_mesh(string name) const;
	Scene* get_scene(string name) const;
	
	void get_meshes_with_filter(string name, vector<Mesh*> &meshes) const;
	void add_scene(Scene *new_scene);

private:

	vector<Texture*> textures_;
	map<string, Texture*> texture_accesor_map_;
	vector<string> texture_extenstions_;

	vector<Mesh*> meshes_;
	map<string, Mesh*> mesh_accesor_map_;

	vector<MeshGroup*> mesh_groups_;
	map<string, MeshGroup*> mesh_groups_via_name_;

	vector<string> mesh_extenstions_;
	map<string, vector<Mesh*>> mesh_filter_data_;

	map<string, Scene* > scenes_;
};

class Scene
{
public:
	Scene(string name);
	~Scene();

	void add_mesh(Mesh *new_mesh);
	void add_mesh_group(MeshGroup *new_mesh_group);
	void add_light(Light *new_light);
	void add_camera(Camera *new_camera);

	void get_meshes_to_render(const Camera *cam, vector<Renderer::DrawRecord> &meshes) const;

	const vector<Mesh*> get_meshes() const;
	const vector<MeshGroup*> get_mesh_groups() const;
	const vector<Light*> get_lights() const;

	Scene *create_copy() const;
	string get_name() const;
	
	Mesh* get_mesh(string name) const;
	Camera* get_camera(string name) const;

	const BoundingBox& get_bb() const;

	void clear_meshes();

	int get_mesh_count() const
	{
		return meshes_.size() + mesh_groups_.size();
	}

private:
	vector<Mesh*> meshes_;
	vector<MeshGroup*> mesh_groups_;
	vector<Light*> lights_;
	vector<Camera*> cameras_;

	BoundingBox bb_;

	string name_;
};


extern ResourceManager resource_manager;


#endif