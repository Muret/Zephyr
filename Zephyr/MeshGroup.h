
#ifndef __INCLUDE_MESH_GROUP_H
#define __INCLUDE_MESH_GROUP_H

#include "includes.h"

class Mesh;

class MeshGroup
{
public:
	MeshGroup(string name) : name_(name)
	{
		D3DXMatrixIdentity(&frame_);
	}

	MeshGroup(const MeshGroup *rhs);

	void add_lod_mesh_with_start_distance(Mesh *m, float distance);
	Mesh* get_mesh_to_render(float distance);

	void set_frame(const D3DXMATRIX &frm)
	{
		frame_ = frm;
	}

	const D3DXMATRIX& get_frame() const
	{
		return frame_;
	}

private:

	std::map<float, Mesh*> meshes_per_start_distance_;
	string name_;

	D3DXMATRIX frame_;
};


#endif