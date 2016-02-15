
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

	D3DXMatrixIdentity(&frame_);
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

ID3D11Buffer* Mesh::get_vertex_buffer()
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


ID3D11Buffer* Mesh::get_index_buffer()
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

void Mesh::validate_bounding_box()
{
	bb.reset();
	for (int i = 0; i < vertices_.size(); i++)
	{
		bb.enlarge_bb_with_point(vertices_[i].position);
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
	color			= D3DXVECTOR4(0,0,0,0);	
	texture_coord	= D3DXVECTOR4(0,0,0,0);
	normal			= D3DXVECTOR4(0,0,0,0);
	tangent			= D3DXVECTOR4(0,0,0,0);
}
