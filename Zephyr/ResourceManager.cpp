
#include "ResourceManager.h"
#include "Utilities.h"
#include "TextureLoader.h"
#include "FBXSceneImporter.h"
#include "Mesh.h"
#include "Camera.h"

ResourceManager::ResourceManager()
{
	texture_extenstions_.push_back("png");
	texture_extenstions_.push_back("jpg");
	texture_extenstions_.push_back("tga");

	mesh_extenstions_.push_back("fbx");
}

ResourceManager::~ResourceManager()
{

}

void ResourceManager::init_resources(string resource_folder)
{
	init_textures(resource_folder);
	init_meshes(resource_folder);
	
	vector<string> folders;
	Utilities::get_folders_under_folder(resource_folder, folders);

	for (int i = 0; i < folders.size(); i++)
	{
		init_resources(resource_folder + "/" + folders[i]);
	}
}

void ResourceManager::init_textures(string resource_folder)
{
	vector<string> names;
	Utilities::get_files_under_folder(resource_folder, names, texture_extenstions_);

	for (int i = 0; i < names.size(); i++)
	{
		TextureLoader loader(resource_folder + "/" + names[i]);
		Texture *new_texture = loader.create_texture_from_file();

		textures_.push_back(new_texture);
		texture_accesor_map_[names[i]] = new_texture;
	}

}

Texture* ResourceManager::get_texture(string name) const
{
	auto it = texture_accesor_map_.find(name);
	if (it == texture_accesor_map_.end())
	{
		return nullptr;
	}
	else
	{
		return it->second;
	}

}

void ResourceManager::init_meshes(string resource_folder)
{
	vector<string> names;
	Utilities::get_files_under_folder(resource_folder, names, mesh_extenstions_);

	for (int i = 0; i < names.size(); i++)
	{
		string fbx_path(resource_folder + "/" + names[i]);
		FBXSceneImporter importer(fbx_path);
		const vector<Mesh*> &meshes = importer.get_scene_meshes();

		string filter_name_wo_extension = Utilities::get_file_name_from_path_wo_extension(names[i]);

		for (int j = 0; j < meshes.size(); j++)
		{
			mesh_accesor_map_[meshes[j]->get_name()] = meshes[j];
			meshes_.push_back(meshes[j]);

			mesh_filter_data_[filter_name_wo_extension].push_back(meshes[j]);
		}
	}
}

Mesh* ResourceManager::get_mesh(string name) const
{
	auto it = mesh_accesor_map_.find(name);
	if (it == mesh_accesor_map_.end())
	{
		return nullptr;
	}
	else
	{
		return it->second;
	}
}

void ResourceManager::get_meshes_with_filter(string name, vector<Mesh*> &meshes) const
{
	auto it = mesh_filter_data_.find(name);
	if (it != mesh_filter_data_.end())
	{
		meshes.insert(meshes.end(), it->second.begin(), it->second.end());
	}
}

void ResourceManager::add_scene(Scene *new_scene)
{
	scenes_[new_scene->get_name()] = new_scene;
}

Scene* ResourceManager::get_scene(string name) const
{
	auto it = scenes_.find(name);
	if (it != scenes_.end())
	{
		return it->second->create_copy();
	}

	return nullptr;
}

ResourceManager resource_manager;

Scene::Scene(string name) : name_(name)
{
}

Scene::~Scene()
{

}

void Scene::add_mesh(Mesh *new_mesh)
{
	meshes_.push_back(new_mesh);

	bb_.enlarge_bb_with_bb(new_mesh->get_bb());
}

void Scene::add_light(Light *new_light)
{
	lights_.push_back(new_light);
}

void Scene::add_camera(Camera *new_camera)
{
	cameras_.push_back(new_camera);
}

const vector<Mesh*> Scene::get_meshes() const
{
	return meshes_;
}

const vector<Light*> Scene::get_lights() const
{
	return lights_;
}

Scene * Scene::create_copy() const
{
	Scene *new_scene = new Scene(name_);
	for (int i = 0; i < meshes_.size(); i++)
	{
		new_scene->add_mesh(meshes_[i]);
	}
	for (int i = 0; i < lights_.size(); i++)
	{
		new_scene->add_light(lights_[i]);
	}
	for (int i = 0; i < cameras_.size(); i++)
	{
		new_scene->add_camera(cameras_[i]);
	}

	return new_scene;
}

std::string Scene::get_name() const
{
	return name_;
}

Mesh* Scene::get_mesh(string name) const
{
	for (int i = 0; i < meshes_.size(); i++)
	{
		if (meshes_[i]->get_name() == name)
		{
			return meshes_[i];
		}
	}

	return nullptr;
}

Camera* Scene::get_camera(string name) const
{
	for (int i = 0; i < cameras_.size(); i++)
	{
		if (cameras_[i]->get_name() == name)
		{
			return cameras_[i];
		}
	}

	return nullptr;
}

const BoundingBox & Scene::get_bb() const
{
	return bb_;
}

void Scene::clear_meshes()
{
	meshes_.clear();
	bb_.reset();
}
