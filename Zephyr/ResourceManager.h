#ifndef INCLUDE_RESOURCE_MANAGER_
#define INCLUDE_RESOURCE_MANAGER_

#include "includes.h"

class Texture;
class Mesh;

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
	void get_meshes_with_filter(string name, vector<Mesh*> &meshes) const;
	
private:

	vector<Texture*> textures_;
	map<string, Texture*> texture_accesor_map_;
	vector<string> texture_extenstions_;

	vector<Mesh*> meshes_;
	map<string, Mesh*> mesh_accesor_map_;
	vector<string> mesh_extenstions_;
	map<string, vector<Mesh*>> mesh_filter_data_;
};

extern ResourceManager resource_manager;


#endif