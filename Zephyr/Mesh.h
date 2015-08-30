
#ifndef __INCLUDE_MESH_H
#define __INCLUDE_MESH_H

#include "d11.h"

class Material;

class Mesh
{
public:

	struct Vertex
	{
		D3DXVECTOR4 position;
		D3DXVECTOR4 color;
		D3DXVECTOR4 texture_coord;
		D3DXVECTOR4 normal;
		D3DXVECTOR4 tangent;

		Vertex();

		bool operator==(const Vertex &rhs);
	};

	Mesh();

	void create_from_fbx(const std::vector<Vertex> &vertices, const std::vector<int> &indices);

	ID3D11Buffer* get_vertex_buffer();
	ID3D11Buffer* get_index_buffer();
	int get_vertex_count() const;
	int get_index_count() const;

	void set_material(Material *mesh_material);
	Material* get_material() const;

	void set_uniform_values() const;

	void set_frame(D3DXMATRIX frame);

	void rotate(float degree, D3DXVECTOR3 axis);
private:

	Material *mesh_material_;

	ID3D11Buffer *vertex_buffer_;
	ID3D11Buffer *index_buffer_;

	D3DXMATRIX frame_;

	//CPU side data
	std::vector<Vertex> vertices_;
	std::vector<int> indices_;
};


#endif