#ifndef INCLUDE_RESOURCE_MANAGER_
#define INCLUDE_RESOURCE_MANAGER_

#include "includes.h"

#include "BoundingBox.h"

class Texture;
class Mesh;
class Light;
class Scene;
class Camera;

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
	void add_light(Light *new_light);
	void add_camera(Camera *new_camera);

	const vector<Mesh*> get_meshes() const;
	const vector<Light*> get_lights() const;

	Scene *create_copy() const;
	string get_name() const;
	
	Mesh* get_mesh(string name) const;
	Camera* get_camera(string name) const;

	const BoundingBox& get_bb() const;

private:
	vector<Mesh*> meshes_;
	vector<Light*> lights_;
	vector<Camera*> cameras_;

	BoundingBox bb_;

	string name_;
};


extern ResourceManager resource_manager;


#endif