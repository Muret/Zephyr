#ifndef _OBJ_IMPORTER_H
#define _OBJ_IMPORTER_H

#include "includes.h"
#include <tiny_obj_loader.h>

class Mesh;
class Material;
class Scene;
class MeshGroup;

class OBJSceneImporter
{
public:

	OBJSceneImporter(std::string base_path, std::string file_name);

	const vector<Mesh*> get_scene_meshes() const;
	const vector<MeshGroup*> get_scene_mesh_groups() const;

private:

	vector<Mesh*> meshes_;
	Scene *scene_to_fill;
};

#endif