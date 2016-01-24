
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
	};

	struct CC_Face
	{
		vector<int> edges_of_face;
		vector<int> vertices_of_face;
		
		vector<int> newly_added_edges_from_face_center_to_edge_midpoint;
		int newly_added_face_point;
	};

	struct CC_Vertex
	{
		D3DXVECTOR4 position;
		
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
	CatmullClark(Mesh *mesh_to_edit);

	void subdivide();
	void fill_mesh_representation(CC_MeshRepresenation &mesh_r) const;
	void sub_divide_imp(CC_MeshRepresenation &mesh_r);
	bool get_common_face_indexes_of_an_edge(const CC_MeshRepresenation & mesh_r, int i, int &first_face_index, int &second_face_index);
	void remove_from_vector(vector<int> & vertices, int i);
private:

	Mesh *mesh_to_edit_;

};




#endif