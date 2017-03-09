#ifndef _ZRF_IMPORTER_H
#define _ZRF_IMPORTER_H

#include "includes.h"

class Mesh;
class Material;
class Scene;
class MeshGroup;

#define ZRF_VERSION 2

class ZRFSceneImporter
{
public:

	ZRFSceneImporter(std::string path);

	const vector<Mesh*> get_scene_meshes() const;
	const vector<MeshGroup*> get_scene_mesh_groups() const;

	bool is_valid() const
	{
		return is_valid_;
	}

private:

	bool is_valid_;
	vector<Mesh*> meshes_;
	Scene *scene_to_fill;
};

#endif