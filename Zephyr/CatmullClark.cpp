
#include "CatmullClark.h"
#include "Mesh.h"

CatmullClark::CatmullClark(Mesh *mesh_to_edit)
{
	mesh_to_edit_ = mesh_to_edit;
}

void CatmullClark::subdivide()
{
	CC_MeshRepresenation mesh_r;
	fill_mesh_representation(mesh_r);

	const int max_subdivision = 1;

	for (int i = 0; i < max_subdivision; i++)
	{
		sub_divide_imp(mesh_r);
	}
}

void CatmullClark::fill_mesh_representation(CC_MeshRepresenation &mesh_r) const
{
	const vector<int> mesh_indices = mesh_to_edit_->get_indices();
	const vector<Mesh::Vertex> mesh_vertices = mesh_to_edit_->get_vertices();

	const int vertex_count = mesh_indices.size();
	for (int vertex_index = 0; vertex_index < vertex_count; vertex_index++)
	{
		CC_Vertex cc_v;	
		cc_v.position = mesh_vertices[vertex_index].position;	
		mesh_r.vertices.push_back(cc_v);
	}

	const int face_count = mesh_indices.size() / 3;
	for (int face_index = 0; face_index < face_count; face_index++)
	{
		int last_face_count = mesh_r.faces.size() - 1;
		int last_edge_count = mesh_r.edges.size() - 1;

		int v_1 = mesh_indices[face_index * 3 + 0];
		int v_2 = mesh_indices[face_index * 3 + 1];
		int v_3 = mesh_indices[face_index * 3 + 2];

		CC_Edge cc_e_1;	cc_e_1.edge_points[0] = v_1;	cc_e_1.edge_points[1] = v_2;	mesh_r.edges.push_back(cc_e_1);
		{
			mesh_r.vertices[v_1].edges.push_back(last_edge_count + 1);
			mesh_r.vertices[v_2].edges.push_back(last_edge_count + 1);
		}

		CC_Edge cc_e_2;	cc_e_2.edge_points[0] = v_2;	cc_e_2.edge_points[1] = v_3;	mesh_r.edges.push_back(cc_e_2);
		{
			mesh_r.vertices[v_2].edges.push_back(last_edge_count + 2);
			mesh_r.vertices[v_3].edges.push_back(last_edge_count + 2);
		}

		CC_Edge cc_e_3;	cc_e_3.edge_points[0] = v_3;	cc_e_3.edge_points[1] = v_1;	mesh_r.edges.push_back(cc_e_3);
		{
			mesh_r.vertices[v_3].edges.push_back(last_edge_count + 3);
			mesh_r.vertices[v_1].edges.push_back(last_edge_count + 3);
		}

		CC_Face cc_face;	
		cc_face.vertices_of_face[0] = v_1;	cc_face.vertices_of_face[1] = v_2;	cc_face.vertices_of_face[2] = v_3;
		cc_face.edges_of_face[0] = last_edge_count + 1;		cc_face.edges_of_face[1] = last_edge_count + 2;		cc_face.edges_of_face[2] = last_edge_count + 3;
		mesh_r.faces.push_back(cc_face);

		mesh_r.vertices[v_1].faces.push_back(last_face_count + 1);
		mesh_r.vertices[v_2].faces.push_back(last_face_count + 1);
		mesh_r.vertices[v_3].faces.push_back(last_face_count + 1);
	}
}

void CatmullClark::sub_divide_imp(CC_MeshRepresenation &mesh_r)
{
	vector<CC_Vertex> &vertices = mesh_r.vertices;
	vector<CC_Edge> &edges = mesh_r.edges;
	vector<CC_Face> &faces = mesh_r.faces;

	int original_vertex_count = vertices.size();
	int original_edge_count = edges.size();
	int original_face_count = faces.size();

	//For each face, add a face point
	for (int i = 0; i < faces.size(); i++)
	{
		D3DXVECTOR4 center_of_face = (vertices[faces[i].vertices_of_face[0]].position + vertices[faces[i].vertices_of_face[1]].position + vertices[faces[i].vertices_of_face[2]].position) * 0.33f;
		CC_Vertex new_vertex; new_vertex.position = center_of_face;
		faces[i].newly_added_face_point = vertices.size();
		vertices.push_back(new_vertex);
	}

	//For each edge, add an edge point.
	for (int i = 0; i < edges.size(); i++)
	{
		int edge_vertex_1 = edges[i].edge_points[0];
		int edge_vertex_2 = edges[i].edge_points[1];
		int edge_vertex_3;
		int edge_vertex_4;

		D3DXVECTOR4 center_of_edge = (vertices[edge_vertex_1].position + vertices[edge_vertex_2].position);
		if (get_common_face_indexes_of_an_edge(mesh_r, i, edge_vertex_3, edge_vertex_4))
		{
			center_of_edge += (vertices[edge_vertex_3].position + vertices[edge_vertex_4].position);
			center_of_edge *= 0.25;
		}
		else
		{
			center_of_edge *= 0.5f;
		}

		CC_Vertex new_vertex; new_vertex.position = center_of_edge;
		edges[i].newly_added_midpoint = vertices.size();
		vertices.push_back(new_vertex);
	}

	//For each face point, add an edge for every edge of the face, connecting the face point to each edge point for the face.
	//this phase will be done at the final triangulation phase
	//for (int i = 0; i < faces.size(); i++)
	//{
	//	CC_Face &cur_face = faces[i];
	//	CC_Vertex &face_center_vertex = vertices[cur_face.newly_added_face_point];
	//	for (int j = 0; j < cur_face.edges_of_face.size(); j++)
	//	{
	//		CC_Edge new_edge_from_face_center_to_edge_midpoint;
	//		new_edge_from_face_center_to_edge_midpoint.edge_points[0] = cur_face.newly_added_face_point;
	//		new_edge_from_face_center_to_edge_midpoint.edge_points[1] = edges[cur_face.edges_of_face[j]].newly_added_midpoint;
	//
	//		edges.push_back(new_edge_from_face_center_to_edge_midpoint);
	//		cur_face.newly_added_edges_from_face_center_to_edge_midpoint.push_back(edges.size() - 1);
	//	}
	//}

	//For each original point P, take the average F of all n (recently created) face points for faces touching P, 
	//and take the average R of all n edge midpoints for edges touching P, where each edge midpoint is the average of its two endpoint vertices. 
	// Move each original point to the point to : ( F + 2 * R + (n - 3) * P ) / n
	for (int i = 0; i < original_vertex_count; i++)
	{
		CC_Vertex &cur_vertex = vertices[i];
		
		D3DXVECTOR4 F = D3DXVECTOR4(0,0,0,0);
		int n = cur_vertex.faces.size();
		for (int j = 0; j < n; j++)
		{
			F += vertices[faces[cur_vertex.faces[j]].newly_added_face_point].position;
		}
		F /= n;

		D3DXVECTOR4 R = D3DXVECTOR4(0, 0, 0, 0);
		int number_of_edges = cur_vertex.edges.size();
		for (int j = 0; j < number_of_edges; j++)
		{
			R += vertices[edges[cur_vertex.edges[j]].newly_added_midpoint].position;
		}
		R /= number_of_edges;

		D3DXVECTOR4 P = cur_vertex.position;

		cur_vertex.position = (F + 2 * R + (n - 3) * P) / n;
	}

	//remove old edges and old faces
	for (int i = 0; i < edges.size(); i++)
	{
		CC_Vertex &first_vertex = vertices[edges[i].edge_points[0]];
		CC_Vertex &secnd_vertex = vertices[edges[i].edge_points[1]];

		remove_from_vector(first_vertex.edges, i);
		remove_from_vector(secnd_vertex.edges, i);
	}

	vector<CC_Edge> old_edges = edges;
	edges.clear();

	vector<CC_Face> old_faces = faces;
	faces.clear();

	//subdivide the new quads , we need render compatible triangles
	//add new edges and faces
	for (int i = 0; i < original_face_count; i++)
	{
		//create 6 new triangle from the original triangle
		CC_Face &cur_face = old_faces[i];
		CC_Vertex &middle_vertex = vertices[cur_face.newly_added_face_point];
		
		for (int cur_vertex_index = 0; cur_vertex_index < cur_face.vertices_of_face.size(); cur_vertex_index++)
		{
			int next_vertex_index = (cur_vertex_index + 1) % cur_face.vertices_of_face.size();

			//remove the references of this old face from vertex
			remove_from_vector(vertices[cur_face.vertices_of_face[cur_vertex_index]].faces, i);

			CC_Edge &cur_edge = old_edges[cur_face.edges_of_face[cur_vertex_index]];
			assert(cur_edge.edge_points[0] == cur_face.vertices_of_face[cur_vertex_index]);

			//first new triangle face
			{
				CC_Face n_face;
				n_face.vertices_of_face.push_back(cur_face.newly_added_face_point);
				n_face.vertices_of_face.push_back(cur_face.vertices_of_face[cur_vertex_index]);
				n_face.vertices_of_face.push_back(cur_edge.newly_added_midpoint);

				vertices[cur_face.newly_added_face_point].faces.push_back(faces.size());
				vertices[cur_face.vertices_of_face[cur_vertex_index]].faces.push_back(faces.size());
				vertices[cur_edge.newly_added_midpoint].faces.push_back(faces.size());

				//add new edges
				{
					CC_Edge edge_1;	edge_1.edge_points[0] = cur_face.newly_added_face_point;	edge_1.edge_points[1] = cur_face.vertices_of_face[cur_vertex_index];
					edges.push_back(edge_1);
					CC_Edge edge_2;	edge_1.edge_points[0] = cur_face.vertices_of_face[cur_vertex_index];	edge_1.edge_points[1] = cur_edge.newly_added_midpoint;
					edges.push_back(edge_1);
					CC_Edge edge_3;	edge_1.edge_points[0] = cur_edge.newly_added_midpoint;	edge_1.edge_points[1] = cur_face.newly_added_face_point;
					edges.push_back(edge_1);

					n_face.edges_of_face.push_back(edges.size() - 3);
					n_face.edges_of_face.push_back(edges.size() - 2);
					n_face.edges_of_face.push_back(edges.size() - 1);
				}

				faces.push_back(n_face);
			}


			//edge 2
			//second new triangle face
			{
				CC_Face n_face;
				n_face.vertices_of_face.push_back(cur_face.newly_added_face_point);
				n_face.vertices_of_face.push_back(cur_edge.newly_added_midpoint);
				n_face.vertices_of_face.push_back(cur_face.vertices_of_face[next_vertex_index]);

				vertices[cur_face.newly_added_face_point].faces.push_back(faces.size());
				vertices[cur_face.vertices_of_face[next_vertex_index]].faces.push_back(faces.size());
				vertices[cur_edge.newly_added_midpoint].faces.push_back(faces.size());

				//add new edges
				{
					CC_Edge edge_1;	edge_1.edge_points[0] = cur_face.newly_added_face_point;	edge_1.edge_points[1] = cur_edge.newly_added_midpoint;
					edges.push_back(edge_1);
					CC_Edge edge_2;	edge_1.edge_points[0] = cur_edge.newly_added_midpoint;	edge_1.edge_points[1] = cur_face.vertices_of_face[next_vertex_index];
					edges.push_back(edge_1);
					CC_Edge edge_3;	edge_1.edge_points[0] = cur_face.vertices_of_face[next_vertex_index];	edge_1.edge_points[1] = cur_face.newly_added_face_point;
					edges.push_back(edge_1);

					n_face.edges_of_face.push_back(edges.size() - 3);
					n_face.edges_of_face.push_back(edges.size() - 2);
					n_face.edges_of_face.push_back(edges.size() - 1);
				}

				faces.push_back(n_face);

			}
		}
	}
	


}

bool CatmullClark::get_common_face_indexes_of_an_edge(const CC_MeshRepresenation & mesh_r, int edge_index, int &first_face_vertex_index, int &second_face_vertex_index)
{
	const CC_Vertex &v_1 = mesh_r.vertices[mesh_r.edges[edge_index].edge_points[0]];
	const CC_Vertex &v_2 = mesh_r.vertices[mesh_r.edges[edge_index].edge_points[1]];

	vector<int> found_common_vertices;

	for (int i = 0; i < v_1.faces.size(); i++)
	{
		int cur_index = v_1.faces[i];
		if (find(v_2.faces.begin(), v_2.faces.end(), cur_index) != v_2.faces.end())
		{
			found_common_vertices.push_back(cur_index);
		}
	}

	if (found_common_vertices.size() == 2)
	{
		first_face_vertex_index = mesh_r.faces[found_common_vertices[0]].newly_added_face_point;
		second_face_vertex_index = mesh_r.faces[found_common_vertices[1]].newly_added_face_point;
		return true;
	}
	else if (found_common_vertices.size() == 1)
	{
		first_face_vertex_index = mesh_r.faces[found_common_vertices[0]].newly_added_face_point;
		second_face_vertex_index = -1;

		return false;
	}

	_ASSERT(false);
	return false;
}

void CatmullClark::remove_from_vector(vector<int> & vertices, int i)
{
	vertices.erase(find(vertices.begin(), vertices.end(), i));
}
