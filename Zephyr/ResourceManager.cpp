
#include "ResourceManager.h"
#include "Utilities.h"
#include "TextureLoader.h"
#include "FBXImporter.h"
#include "Mesh.h"

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
		vector<Mesh*> meshes;
		string fbx_path(resource_folder + "/" + names[i]);
		FBXImporter importer(fbx_path, meshes);

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

ResourceManager resource_manager;
