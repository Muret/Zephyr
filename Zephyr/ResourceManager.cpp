
#include "ResourceManager.h"
#include "Utilities.h"
#include "TextureLoader.h"
#include "FBXSceneImporter.h"
#include "ObjSceneImporter.h"
#include "ZRFSceneImporter.h"
#include "Mesh.h"
#include "Camera.h"
#include "MeshGroup.h"
#include "Renderer.h"


ResourceManager::ResourceManager()
{
	texture_extenstions_.push_back("png");
	texture_extenstions_.push_back("jpg");
	texture_extenstions_.push_back("tga");

	mesh_extenstions_.push_back("fbx");
	mesh_extenstions_.push_back("obj");
}

ResourceManager::~ResourceManager()
{

}

void ResourceManager::init_resources(string resource_folder)
{
	vector<string> folders;
	Utilities::get_folders_under_folder(resource_folder, folders);

	for (int i = 0; i < folders.size(); i++)
	{
		init_resources(resource_folder + "/" + folders[i]);
	}

	init_textures(resource_folder);
	init_meshes(resource_folder);
}

void ResourceManager::init_textures(string resource_folder)
{
	vector<string> names;
	Utilities::get_files_under_folder(resource_folder, names, texture_extenstions_);

	for (int i = 0; i < names.size(); i++)
	{
		TextureLoader loader(resource_folder + "/" + names[i]);
		Texture *new_texture = loader.create_texture_from_file();

		string texture_access_name = Utilities::get_file_name_from_path_wo_extension(names[i]);

		textures_.push_back(new_texture);
		texture_accesor_map_[texture_access_name] = new_texture;
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
	vector<string> zrf_names;
	Utilities::get_files_under_folder(resource_folder, zrf_names, {"zrf"});
	Utilities::get_files_under_folder(resource_folder, names, mesh_extenstions_);

	std::set<string> zrf_file_names;
	 
	for (int i = 0; i < zrf_names.size(); i++)
	{
		ZRFSceneImporter zrf_importer(resource_folder + "/" + zrf_names[i]);

		if (zrf_importer.is_valid() == false)
		{
			continue;
		}

		string filter_name_wo_extension = Utilities::get_file_name_from_path_wo_extension(zrf_names[i]);
	
		vector<Mesh*> meshes = zrf_importer.get_scene_meshes();
		for (int j = 0; j < meshes.size(); j++)
		{
			mesh_accesor_map_[meshes[j]->get_name()] = meshes[j];
			meshes_.push_back(meshes[j]);
	
			mesh_filter_data_[filter_name_wo_extension].push_back(meshes[j]);
		}
	
		zrf_file_names.insert(filter_name_wo_extension);
	}

	for (int i = 0; i < names.size(); i++)
	{
		string extension = Utilities::get_extension_from_path(names[i]);
		string filter_name_wo_extension = Utilities::get_file_name_from_path_wo_extension(names[i]);

		if (zrf_file_names.find(filter_name_wo_extension) != zrf_file_names.end())
		{
			continue;
		}

		vector<Mesh*> meshes;
		if (extension == "obj")
		{
			OBJSceneImporter importer(resource_folder, names[i]);

			meshes = importer.get_scene_meshes();
		}
		else if (extension == "fbx")
		{
			string fbx_path(resource_folder + "/" + names[i]);
			FBXSceneImporter importer(fbx_path);
			meshes = importer.get_scene_meshes();
		}

		for (int j = 0; j < meshes.size(); j++)
		{
			mesh_accesor_map_[meshes[j]->get_name()] = meshes[j];
			meshes_.push_back(meshes[j]);

			mesh_filter_data_[filter_name_wo_extension].push_back(meshes[j]);
		}

		//string zrf = resource_folder + "/" + filter_name_wo_extension + ".zrf";
		//ofstream file(zrf, ios::out | ios::binary);
		//
		//int mesh_count = meshes.size();
		//int zrf_version = ZRF_VERSION;
		//
		//file.write((char*)&zrf_version, sizeof(int));
		//file.write((char*)&mesh_count, sizeof(int));
		//
		//for (int j = 0; j < meshes.size(); j++)
		//{
		//	meshes[j]->write_to_file(file);
		//}
		//file.close();
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

void Scene::add_mesh_group(MeshGroup * new_mesh_group)
{
	mesh_groups_.push_back(new_mesh_group);
}

void Scene::add_light(Light *new_light)
{
	lights_.push_back(new_light);
}

void Scene::add_camera(Camera *new_camera)
{
	cameras_.push_back(new_camera);
}

void Scene::get_meshes_to_render(const Camera * cam, vector<Renderer::DrawRecord> &meshes) const
{	
	for (int i = 0; i < meshes_.size(); i++)
	{
		meshes.push_back(Renderer::DrawRecord(meshes_[i], meshes_[i]->get_frame()));
	}

	const D3DXVECTOR4 &cam_pos = cam->get_position();

	for (int i = 0; i < mesh_groups_.size(); i++)
	{
		MeshGroup *cur_group = mesh_groups_[i];
		const D3DXMATRIX &mg_frame = cur_group->get_frame();
		D3DXVECTOR4 cam_vec(cam_pos.x - mg_frame.m[3][0], cam_pos.y - mg_frame.m[3][1], cam_pos.z - mg_frame.m[3][2], 0);
		
		float distance = sqrt(D3DXVec4Dot(&cam_vec, &cam_vec));
		Mesh *lod_mesh = cur_group->get_mesh_to_render(distance);
		const D3DXMATRIX &local_frame = lod_mesh->get_frame();
		D3DXMATRIX final_frame = local_frame * mg_frame;
		meshes.push_back(Renderer::DrawRecord(lod_mesh, final_frame));
	}
}

const vector<Mesh*> Scene::get_meshes() const
{
	return meshes_;
}

const vector<MeshGroup*> Scene::get_mesh_groups() const
{
	return mesh_groups_;
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
	for (int i = 0; i < mesh_groups_.size(); i++)
	{
		new_scene->add_mesh_group(mesh_groups_[i]);
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
