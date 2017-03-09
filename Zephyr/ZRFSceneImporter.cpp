
#include <assert.h>

#include "ZRFSceneImporter.h"
#include "Mesh.h"
#include "Material.h"
#include "Light.h"
#include "ResourceManager.h"
#include "Utilities.h"
#include "Camera.h"
#include "MeshGroup.h"

ZRFSceneImporter::ZRFSceneImporter(std::string path)
{
	SCOPE_TIMER(path);

	ifstream file(path, ios::in | ios::binary);

	int version = 0;
	file.read((char*)&version, sizeof(int));

	if (version != ZRF_VERSION)
	{
		is_valid_ = false;
		return;
	}
	else
	{
		is_valid_ = true;
	}

	int mesh_count = 0;
	file.read((char*)&mesh_count, sizeof(int));

	Scene *scene = new Scene(Utilities::get_file_name_from_path_wo_extension(path));

	for (int i = 0; i < mesh_count; i++)
	{
		Mesh *new_mesh = new Mesh();
		new_mesh->read_from_file(file);
		new_mesh->validate_bounding_box();
		meshes_.push_back(new_mesh);

		scene->add_mesh(new_mesh);
	}

	resource_manager.add_scene(scene);
}

const vector<Mesh*> ZRFSceneImporter::get_scene_meshes() const
{
	return meshes_;
}

const vector<MeshGroup*> ZRFSceneImporter::get_scene_mesh_groups() const
{
	return vector<MeshGroup*>();
}
