
#include "Demo.h"
#include "Mesh.h"
#include "Camera.h"
#include <assert.h>

#include "d11.h"

Mesh::Mesh()
{
	vertex_buffer_ = nullptr;
	index_buffer_ = nullptr;

	mesh_material_ = nullptr;

	D3DXMatrixIdentity(&frame_);
}

void Mesh::create_from_fbx(const std::vector<Vertex> &vertices, const std::vector<int> &indices)
{
	vertices_ = vertices;
	indices_ = indices;
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

void Mesh::set_uniform_values() const
{
	D3DXVECTOR3 camera_position = demo_camera.get_position();
	D3DXVECTOR3 view_direction = demo_camera.get_forward_vector();
	D3DXVECTOR3 up_vector = demo_camera.get_up_vector();
	D3DXVECTOR3 right_vector = demo_camera.get_right_vector();

	D3DXMATRIX view, projection, inverseViewProjection, inverseProjection, worldMatrix;
	D3DXVECTOR3 lookat = camera_position + view_direction;

	D3DXMatrixLookAtRH(&view, &camera_position, &lookat, &up_vector);

	D3DXMatrixPerspectiveFovRH(&projection, PI * 0.25, float(g_screenWidth) / float(g_screenHeight), 0.1f, 1000.0f);

	float determinant;
	D3DXMATRIX mWorldViewProjection = frame_ * view * projection;

	worldMatrix = frame_;

	D3DXMatrixInverse(&inverseViewProjection, &determinant, &mWorldViewProjection);
	D3DXMatrixInverse(&inverseProjection, &determinant, &projection);

	// Transpose the matrices to prepare them for the shader.
	D3DXMatrixTranspose(&mWorldViewProjection, &mWorldViewProjection);
	D3DXMatrixTranspose(&inverseViewProjection, &inverseViewProjection);
	D3DXMatrixTranspose(&projection, &projection);
	D3DXMatrixTranspose(&inverseProjection, &inverseProjection);
	D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
	D3DXMatrixTranspose(&view, &view);
	
	render_constantsBuffer_cpu.WorldViewProjectionMatrix = mWorldViewProjection;
	render_constantsBuffer_cpu.WorldMatrix = worldMatrix;
	render_constantsBuffer_cpu.right_direction = D3DXVECTOR4(right_vector, 0);
	render_constantsBuffer_cpu.up_direction = D3DXVECTOR4(up_vector, 0);
	render_constantsBuffer_cpu.view_direction = D3DXVECTOR4(view_direction, 0);
	render_constantsBuffer_cpu.camera_position = D3DXVECTOR4(camera_position, 0);
	render_constantsBuffer_cpu.screen_texture_half_pixel_forced_mipmap = D3DXVECTOR4((1.0f / float(g_screenWidth)), (1.0f / float(g_screenHeight)), -1, 0);
	render_constantsBuffer_cpu.inverseWorldViewProjectionMatrix = inverseViewProjection;
	render_constantsBuffer_cpu.inverseProjectionMatrix = inverseProjection;
	render_constantsBuffer_cpu.projectionMatrix = projection;
	render_constantsBuffer_cpu.viewMatrix = view;
	
	render_constantsBuffer_cpu.near_far_padding2 = D3DXVECTOR4(0.1f, 100.0f,0,0);

	UpdateGlobalBuffers();
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

std::string Mesh::get_name() const
{
	return name_;
}


bool Mesh::Vertex::operator==(const Vertex &rhs)
{
	bool same = true;
	same = same && (position == rhs.position);
	same = same && (color == rhs.color);
	same = same && (texture_coord == rhs.texture_coord);
	same = same && (normal == rhs.normal);
	same = same && (normal == rhs.tangent);

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
