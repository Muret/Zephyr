
#include "MeshGroup.h"
#include "Mesh.h"

MeshGroup::MeshGroup(const MeshGroup * rhs)
{
	name_ = rhs->name_;
	for (auto it = rhs->meshes_per_start_distance_.begin(); it != rhs->meshes_per_start_distance_.end(); it++)
	{
		Mesh *new_mesh = new Mesh(it->second);
		meshes_per_start_distance_.insert(make_pair(it->first, new_mesh));
	}

	frame_ = rhs->frame_;
}

void MeshGroup::add_lod_mesh_with_start_distance(Mesh * m, float distance)
{
	meshes_per_start_distance_.insert(make_pair(distance, m));
}

Mesh * MeshGroup::get_mesh_to_render(float distance)
{
	auto it = meshes_per_start_distance_.lower_bound(distance);

	if (it != meshes_per_start_distance_.end())
	{
		return it->second;
	}
	else if (meshes_per_start_distance_.empty() == false)
	{
		return meshes_per_start_distance_.rbegin()->second;
	}

	return nullptr;
}
