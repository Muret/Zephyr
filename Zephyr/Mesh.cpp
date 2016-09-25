
#include "Demo.h"
#include "Mesh.h"
#include "Camera.h"
#include "Material.h"

#include "d11.h"

Mesh::Mesh()
{
	vertex_buffer_ = nullptr;
	index_buffer_ = nullptr;

	mesh_material_ = nullptr;

	render_wireframe_ = false;
	color_multiplier_ = D3DXVECTOR4(1, 1, 1, 1);

	mesh_type_ = MeshType::triangle_mesh;

	D3DXMatrixIdentity(&frame_);
}

Mesh::Mesh(const Mesh * original_mesh)
{
	original_mesh->get_index_buffer();
	original_mesh->get_vertex_buffer();

	mesh_material_ = original_mesh->mesh_material_;
	vertex_buffer_ = original_mesh->vertex_buffer_;
	index_buffer_ = original_mesh->index_buffer_;
	frame_ = original_mesh->frame_;
	name_ = original_mesh->name_;
	color_multiplier_ = original_mesh->color_multiplier_;
	bb = original_mesh->bb;

	indices_ = original_mesh->indices_;
	mesh_type_ = original_mesh->mesh_type_;
}

void Mesh::create_from_buffers(const std::vector<Vertex> &vertices, const std::vector<int> &indices)
{
	vertices_ = vertices;
	indices_ = indices;

	if (vertex_buffer_ != nullptr)
	{
		//delete vertex_buffer_;
		vertex_buffer_ = nullptr;
	}

	if (index_buffer_ != nullptr)
	{
		//delete index_buffer_;
		index_buffer_ = nullptr;
	}

	validate_bounding_box();

}

ID3D11Buffer* Mesh::get_vertex_buffer() const
{
	if (vertex_buffer_ == nullptr)
	{
		Vertex *vertices = new Vertex[vertices_.size()];

		for (int i = 0; i < vertices_.size(); i++)
		{
			vertices[i] = vertices_[i];
		}

		vertex_buffer_ = CreateVertexBuffer(vertices_.size(), (float*)vertices, sizeof(Vertex));
		delete[] vertices;
	}

	assert(vertex_buffer_ != nullptr);
	return vertex_buffer_;

}


ID3D11Buffer* Mesh::get_index_buffer() const
{
	if (index_buffer_ == nullptr)
	{
		int *indices = new int[indices_.size()];

		for (int i = 0; i < indices_.size(); i++)
		{
			indices[i] = indices_[i];
		}

		index_buffer_ = CreateIndexBuffer(indices_.size(), indices, sizeof(int));
		delete[] indices;
	}

	assert(index_buffer_ != nullptr);
	return index_buffer_;
	

}


int Mesh::get_vertex_count() const
{
	return vertices_.size();
}

int Mesh::get_index_count() const
{
	return indices_.size();
}

void Mesh::set_material(Material *mesh_material)
{
	mesh_material_ = mesh_material;
}

Material* Mesh::get_material() const
{
	return mesh_material_;
}


void Mesh::rotate(float degree, D3DXVECTOR3 axis)
{
	D3DXMATRIX rot_frame;
	D3DXMatrixRotationAxis(&rot_frame, &axis, degree / 180.0f * PI);

	frame_ = rot_frame * frame_;
}

void Mesh::set_frame(D3DXMATRIX frame)
{
	frame_ = frame;
	validate_bounding_box();
}

void Mesh::set_name(const char* name)
{
	name_ = name;
}

const vector<int>& Mesh::get_indices() const
{
	return indices_;
}

const vector<Mesh::Vertex>& Mesh::get_vertices() const
{
	return vertices_;
}

std::string Mesh::get_name() const
{
	return name_;
}


bool Mesh::is_wireframe() const
{
	return render_wireframe_;
}

void Mesh::set_wireframe(bool v)
{
	render_wireframe_ = v;
}

void Mesh::set_color_multiplier(const D3DXVECTOR4& color)
{
	color_multiplier_ = color;
}

const D3DXMATRIX& Mesh::get_frame() const
{
	return frame_;
}

const BoundingBox & Mesh::get_bb() const
{
	return bb;
}

void Mesh::add_cube_mesh(vector<Mesh::Vertex>& vertices, vector<int>& indices, const D3DXVECTOR3 &center, const D3DXVECTOR3 &half_length, const D3DXVECTOR4 &color)
{
	D3DXVECTOR3 cur_vertices[8];
	cur_vertices[0] = D3DXVECTOR3(center + half_length * D3DXVECTOR3(-1, -1, -1));
	cur_vertices[1] = D3DXVECTOR3(center + half_length * D3DXVECTOR3(+1, -1, -1));
	cur_vertices[2] = D3DXVECTOR3(center + half_length * D3DXVECTOR3(-1, -1, +1));
	cur_vertices[3] = D3DXVECTOR3(center + half_length * D3DXVECTOR3(+1, -1, +1));

	cur_vertices[4] = D3DXVECTOR3(center + half_length * D3DXVECTOR3(-1, +1, -1));
	cur_vertices[5] = D3DXVECTOR3(center + half_length * D3DXVECTOR3(+1, +1, -1));
	cur_vertices[6] = D3DXVECTOR3(center + half_length * D3DXVECTOR3(-1, +1, +1));
	cur_vertices[7] = D3DXVECTOR3(center + half_length * D3DXVECTOR3(+1, +1, +1));

	int current_first_vertex = vertices.size();
	for (int i = 0; i < 8; i++)
	{
		Mesh::Vertex new_v;
		new_v.position = D3DXVECTOR4(cur_vertices[i], 1);
		new_v.color = color;

		vertices.push_back(new_v);
	}

	indices.push_back(current_first_vertex + 0);	indices.push_back(current_first_vertex + 2);	indices.push_back(current_first_vertex + 1);
	indices.push_back(current_first_vertex + 2);	indices.push_back(current_first_vertex + 3);	indices.push_back(current_first_vertex + 1);

	indices.push_back(current_first_vertex + 0);	indices.push_back(current_first_vertex + 4);	indices.push_back(current_first_vertex + 1);
	indices.push_back(current_first_vertex + 4);	indices.push_back(current_first_vertex + 5);	indices.push_back(current_first_vertex + 1);

	indices.push_back(current_first_vertex + 6);	indices.push_back(current_first_vertex + 4);	indices.push_back(current_first_vertex + 0);
	indices.push_back(current_first_vertex + 6);	indices.push_back(current_first_vertex + 0);	indices.push_back(current_first_vertex + 2);

	indices.push_back(current_first_vertex + 5);	indices.push_back(current_first_vertex + 7);	indices.push_back(current_first_vertex + 3);
	indices.push_back(current_first_vertex + 5);	indices.push_back(current_first_vertex + 3);	indices.push_back(current_first_vertex + 1);

	indices.push_back(current_first_vertex + 6);	indices.push_back(current_first_vertex + 5);	indices.push_back(current_first_vertex + 4);
	indices.push_back(current_first_vertex + 6);	indices.push_back(current_first_vertex + 7);	indices.push_back(current_first_vertex + 5);

	indices.push_back(current_first_vertex + 7);	indices.push_back(current_first_vertex + 6);	indices.push_back(current_first_vertex + 3);
	indices.push_back(current_first_vertex + 6);	indices.push_back(current_first_vertex + 2);	indices.push_back(current_first_vertex + 3);

}

void Mesh::validate_bounding_box()
{
	bb.reset();
	for (int i = 0; i < vertices_.size(); i++)
	{
		D3DXVECTOR3 world_position, local_position = D3DXVECTOR3(vertices_[i].position);
		D3DXVec3TransformCoord(&world_position, &local_position, &frame_);

		bb.enlarge_bb_with_point(D3DXVECTOR4(world_position, 1));
	}
}

bool Mesh::Vertex::operator==(const Vertex &rhs)
{
	bool same = true;
	same = same && (position == rhs.position);
	same = same && (color == rhs.color);
	same = same && (texture_coord == rhs.texture_coord);
	same = same && (normal == rhs.normal);
	same = same && (tangent == rhs.tangent);

	return same;
}

Mesh::Vertex::Vertex()
{
	position		= D3DXVECTOR4(0,0,0,0);
	color			= D3DXVECTOR4(1,1,1,0);	
	texture_coord	= D3DXVECTOR4(0,0,0,0);
	normal			= D3DXVECTOR4(0,0,0,0);
	tangent			= D3DXVECTOR4(0,0,0,0);
}
