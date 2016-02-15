
#ifndef __INCLUDE_CATMULLCLARK_H
#define __INCLUDE_CATMULLCLARK_H

#include "includes.h"

class Mesh;

class CatmullClark
{
	struct CC_Edge
	{
		int edge_points[2];
		//int touching_faces[2];
		int newly_added_midpoint;
		int face;
	};

	struct CC_Face
	{
		vector<int> edges_of_face;
		vector<int> vertices_of_face;
		
		vector<int> newly_added_edges_from_face_center_to_edge_midpoint;
		int newly_added_face_point;

		vector<int> new_added_midpoints;
	};

	struct CC_Vertex
	{
		D3DXVECTOR4 position;
		D3DXVECTOR4 v2_new_position;
		
		vector<int> edges;
		vector<int> faces;
	};

	struct CC_MeshRepresenation
	{
		vector<CC_Face> faces;
		vector<CC_Edge> edges;
		vector<CC_Vertex> vertices;
	};


public:
	CatmullClark(Mesh *mesh_to_edit, int mode);

	void subdivide(int count);
	void fill_mesh_representation(CC_MeshRepresenation &mesh_r) const;

	void sub_divide_imp_v1(CC_MeshRepresenation &mesh_r);
	void sub_divide_imp_v2(CC_MeshRepresenation &mesh_r);

	bool get_common_face_indexes_of_an_edge(const CC_MeshRepresenation & mesh_r, int i, int &first_face_index, int &second_face_index, int &face1, int &face2);
	void remove_from_vector(vector<int> & vertices, int i);
	int get_last_vertex_from_face(const CC_Face &get_other_vertex, int v1, int v2);
	int get_other_vertex_index_from_edge(const CC_Edge &edge, int v1) const;
	void refill_mesh_render_buffers(CC_MeshRepresenation &mesh_r);

	D3DXVECTOR3 get_normal_of_face(int face_index, const CC_MeshRepresenation &mesh_r) const;
	bool get_twin_edge_midpoint(const CC_MeshRepresenation &mesh_r, int edge_index, int &twin_edge_midpoint) const;
private:

	Mesh *mesh_to_edit_;
	int mode_;

};




#endif