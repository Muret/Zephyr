
#ifndef __INCLUDE_MESH_H
#define __INCLUDE_MESH_H

#include "includes.h"
#include "d11.h"
#include "BoundingBox.h"

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

	void create_from_buffers(const std::vector<Vertex> &vertices, const std::vector<int> &indices);

	ID3D11Buffer* get_vertex_buffer() const;
	ID3D11Buffer* get_index_buffer() const;
	int get_vertex_count() const;
	int get_index_count() const;

	void set_material(Material *mesh_material);
	Material* get_material() const;

	void set_frame(D3DXMATRIX frame);

	void rotate(float degree, D3DXVECTOR3 axis);
	void set_name(const char* name);

	const vector<int>& get_indices() const;
	const vector<Vertex>& get_vertices() const;

	string get_name() const;
	bool is_wireframe() const;
	void set_wireframe(bool v);

	void set_color_multiplier(const D3DXVECTOR4& color);
	const D3DXMATRIX& get_frame() const;

	const BoundingBox& get_bb() const;
private:

	void validate_bounding_box();

	Material *mesh_material_;

	mutable ID3D11Buffer *vertex_buffer_;
	mutable ID3D11Buffer *index_buffer_;

	D3DXMATRIX frame_;

	//CPU side data
	std::vector<Vertex> vertices_;
	std::vector<int> indices_;

	string name_;

	D3DXVECTOR4 color_multiplier_;

	bool render_wireframe_;

	BoundingBox bb;
};


#endif