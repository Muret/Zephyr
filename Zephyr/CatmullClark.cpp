
#include "CatmullClark.h"
#include "Mesh.h"
#include <set>

CatmullClark::CatmullClark(Mesh *mesh_to_edit, int mode)
{
	mesh_to_edit_ = mesh_to_edit;
	mode_ = mode;
}

void CatmullClark::subdivide(int count)
{
	CC_MeshRepresenation mesh_r;
	fill_mesh_representation(mesh_r);

	for (int i = 0; i < count; i++)
	{
		if (mode_ == 1)
		{
			sub_divide_imp_v1(mesh_r);
		}
		else if(mode_ == 2)
		{
			sub_divide_imp_v2(mesh_r);
		}
	}

	refill_mesh_render_buffers(mesh_r);
}

void CatmullClark::fill_mesh_representation(CC_MeshRepresenation &mesh_r) const
{
	const vector<int> mesh_indices = mesh_to_edit_->get_indices();
	const vector<Mesh::Vertex> mesh_vertices = mesh_to_edit_->get_vertices();

	const int vertex_count = mesh_vertices.size();
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

		CC_Edge cc_e_1;	cc_e_1.edge_points[0] = v_1;	cc_e_1.edge_points[1] = v_2;	mesh_r.edges.push_back(cc_e_1);		cc_e_1.newly_added_midpoint = -1;
		{
			mesh_r.vertices[v_1].edges.push_back(last_edge_count + 1);
			mesh_r.vertices[v_2].edges.push_back(last_edge_count + 1);
		}

		CC_Edge cc_e_2;	cc_e_2.edge_points[0] = v_2;	cc_e_2.edge_points[1] = v_3;	mesh_r.edges.push_back(cc_e_2);		cc_e_2.newly_added_midpoint = -1;
		{
			mesh_r.vertices[v_2].edges.push_back(last_edge_count + 2);
			mesh_r.vertices[v_3].edges.push_back(last_edge_count + 2);
		}

		CC_Edge cc_e_3;	cc_e_3.edge_points[0] = v_3;	cc_e_3.edge_points[1] = v_1;	mesh_r.edges.push_back(cc_e_3);		cc_e_3.newly_added_midpoint = -1;
		{
			mesh_r.vertices[v_3].edges.push_back(last_edge_count + 3);
			mesh_r.vertices[v_1].edges.push_back(last_edge_count + 3);
		}

		CC_Face cc_face;	
		cc_face.vertices_of_face.push_back(v_1);	cc_face.vertices_of_face.push_back(v_2);	cc_face.vertices_of_face.push_back(v_3);
		cc_face.edges_of_face.push_back(last_edge_count + 1);		cc_face.edges_of_face.push_back(last_edge_count + 2);		cc_face.edges_of_face.push_back(last_edge_count + 3);
		mesh_r.faces.push_back(cc_face);

		mesh_r.vertices[v_1].faces.push_back(last_face_count + 1);
		mesh_r.vertices[v_2].faces.push_back(last_face_count + 1);
		mesh_r.vertices[v_3].faces.push_back(last_face_count + 1);
	}


}

void CatmullClark::sub_divide_imp_v1(CC_MeshRepresenation &mesh_r)
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

		int face1, face2;

		int twin_edge_midpoint;
		if (get_twin_edge_midpoint(mesh_r, i, twin_edge_midpoint))
		{
			mesh_r.edges[i].newly_added_midpoint = twin_edge_midpoint;
			continue;
		}


		D3DXVECTOR4 center_of_edge = (vertices[edge_vertex_1].position + vertices[edge_vertex_2].position);
		if (get_common_face_indexes_of_an_edge(mesh_r, i, edge_vertex_3, edge_vertex_4, face1, face2))
		{
			//center_of_edge *= 0.5f;
			//
			//D3DXVECTOR3 normal1 = get_normal_of_face(face1, mesh_r);
			//D3DXVECTOR3 distance_to_center = center_of_edge - vertices[edge_vertex_3].position;
			//D3DXVECTOR4 move_1 = D3DXVECTOR4(D3DXVec3Dot(&distance_to_center, &normal1) * normal1, 0);
			//
			//D3DXVECTOR3 normal2 = get_normal_of_face(face2, mesh_r);
			//D3DXVECTOR3 distance_to_center2 = center_of_edge - vertices[edge_vertex_4].position;
			//D3DXVECTOR4 move_2 = D3DXVECTOR4(D3DXVec3Dot(&distance_to_center2, &normal2) * normal2, 0);
			//center_of_edge -= move_1 + move_2;

			float multiplier = 1.0f;
			float div = 1.0f / (multiplier * 2.0 + 2.0);

			center_of_edge += (vertices[edge_vertex_3].position + vertices[edge_vertex_4].position) * multiplier;
			center_of_edge *= div;
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

		cur_vertex.position = (F * 2 + 2 * R + (n - 3) * P) / (n + 1);
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

				float beta = 0;

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

void CatmullClark::sub_divide_imp_v2(CC_MeshRepresenation &mesh_r)
{
	static int co = 0;
	co++;
	//divide the old triangle into 4 smaller triangle
	vector<CC_Face> new_faces;

	const float edge_mid_c_edge_points_factor = 3.0f / 8.0f;
	const float edge_mid_c_face_points_factor = 1.0f / 8.0f;

	int old_vertex_count = mesh_r.vertices.size();

	int temp, face_index1, face_index2;

	//fill edge points
	for (int edge_index = 0; edge_index < mesh_r.edges.size(); edge_index++)
	{
		int old_v1_index = mesh_r.edges[edge_index].edge_points[0];
		int old_v2_index = mesh_r.edges[edge_index].edge_points[1];

		CC_Vertex &old_v1 = mesh_r.vertices[old_v1_index];
		CC_Vertex &old_v2 = mesh_r.vertices[old_v2_index];

		CC_Vertex new_v;	new_v.position = (old_v1.position + old_v2.position) * edge_mid_c_edge_points_factor;
		
		int twin_edge_midpoint;
		if (get_twin_edge_midpoint(mesh_r, edge_index, twin_edge_midpoint))
		{
			mesh_r.edges[edge_index].newly_added_midpoint = twin_edge_midpoint;
			continue;
		}

		if (get_common_face_indexes_of_an_edge(mesh_r, edge_index, temp, temp, face_index1, face_index2))
		{
			new_v.position += mesh_r.vertices[get_last_vertex_from_face(mesh_r.faces[face_index1], old_v1_index, old_v2_index)].position * edge_mid_c_face_points_factor;
			new_v.position += mesh_r.vertices[get_last_vertex_from_face(mesh_r.faces[face_index2], old_v1_index, old_v2_index)].position * edge_mid_c_face_points_factor;
		}
		else
		{
			new_v.position = (old_v1.position + old_v2.position) * 0.5;
		}

		mesh_r.vertices.push_back(new_v);
		int new_v4_index = mesh_r.vertices.size() - 1;
		mesh_r.edges[edge_index].newly_added_midpoint = new_v4_index;
	}

	for (int i = 0; i < mesh_r.faces.size(); i++)
	{
		int old_v1_index = mesh_r.faces[i].vertices_of_face[0];
		int old_v2_index = mesh_r.faces[i].vertices_of_face[1];
		int old_v3_index = mesh_r.faces[i].vertices_of_face[2];

		CC_Vertex &old_v1 = mesh_r.vertices[mesh_r.faces[i].vertices_of_face[0]];
		CC_Vertex &old_v2 = mesh_r.vertices[mesh_r.faces[i].vertices_of_face[1]];
		CC_Vertex &old_v3 = mesh_r.vertices[mesh_r.faces[i].vertices_of_face[2]];
		
		int new_v4_index = mesh_r.edges[mesh_r.faces[i].edges_of_face[0]].newly_added_midpoint;
		int new_v5_index = mesh_r.edges[mesh_r.faces[i].edges_of_face[1]].newly_added_midpoint;
		int new_v6_index = mesh_r.edges[mesh_r.faces[i].edges_of_face[2]].newly_added_midpoint;

		CC_Face f_1;	f_1.vertices_of_face.push_back(old_v1_index); f_1.vertices_of_face.push_back(new_v4_index); f_1.vertices_of_face.push_back(new_v6_index);
		CC_Face f_2;	f_2.vertices_of_face.push_back(new_v4_index); f_2.vertices_of_face.push_back(old_v2_index); f_2.vertices_of_face.push_back(new_v5_index);
		CC_Face f_3;	f_3.vertices_of_face.push_back(new_v6_index); f_3.vertices_of_face.push_back(new_v5_index); f_3.vertices_of_face.push_back(old_v3_index);
		CC_Face f_4;	f_4.vertices_of_face.push_back(new_v6_index); f_4.vertices_of_face.push_back(new_v4_index); f_4.vertices_of_face.push_back(new_v5_index);

		new_faces.push_back(f_1);
		new_faces.push_back(f_2);
		new_faces.push_back(f_3);
		new_faces.push_back(f_4);
	}

	for (int i = 0; i < old_vertex_count; i++)
	{
		int edge_count = mesh_r.vertices[i].edges.size();
		float beta = edge_count == 3 ? (3.0f / 16.0f) : 3.0f / (edge_count * 8.0f);

		D3DXVECTOR4 total = mesh_r.vertices[i].position * (1 - edge_count * beta);

		for (int j = 0; j < edge_count; j++)
		{
			int v2 = get_other_vertex_index_from_edge(mesh_r.edges[mesh_r.vertices[i].edges[j]], i);
			total = total + mesh_r.vertices[v2].position * beta;
		}

		mesh_r.vertices[i].v2_new_position = total;
	}



	for (int i = 0; i < old_vertex_count; i++)
	{
		mesh_r.vertices[i].position = mesh_r.vertices[i].v2_new_position;
	}

	for (int i = 0; i < old_vertex_count; i++)
	{
		for (int j = 0; j < old_vertex_count; j++)
		{
			if (i != j)
			{
				D3DXVECTOR4 a = mesh_r.vertices[i].position - mesh_r.vertices[j].position;
				D3DXVECTOR4 b;
				float dist = sqrt(D3DXVec4Dot(&b, &b));
				if (dist < 1e-3)
				{
					int a = 5;
				}

			}
		}
	}

	mesh_r.faces = new_faces;

	refill_mesh_render_buffers(mesh_r);
}

bool CatmullClark::get_common_face_indexes_of_an_edge(const CC_MeshRepresenation & mesh_r, int edge_index, int &first_face_vertex_index, int &second_face_vertex_index, int &face1, int &face2)
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
		face1 = found_common_vertices[0];
		face2 = found_common_vertices[1];
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

int CatmullClark::get_last_vertex_from_face(const CC_Face &face, int v1, int v2)
{
	vector<int> vertices = face.vertices_of_face;
	vertices.erase(find(vertices.begin(), vertices.end(), v1));
	vertices.erase(find(vertices.begin(), vertices.end(), v2));

	_ASSERT(vertices.size() == 1);
	return vertices[0];
}

int CatmullClark::get_other_vertex_index_from_edge(const CC_Edge &edge, int v1) const
{
	if (edge.edge_points[0] == v1)
	{
		return edge.edge_points[1];
	}
	else
	{
		return edge.edge_points[0];
	}
}

void CatmullClark::refill_mesh_render_buffers(CC_MeshRepresenation &mesh_r)
{
	vector<Mesh::Vertex> new_vertices;
	vector<int> new_indices;

	std::set<D3DXVECTOR4> positions;

	for (int i = 0; i < mesh_r.vertices.size(); i++)
	{
		Mesh::Vertex new_mesh;
		new_mesh.position = mesh_r.vertices[i].position;
		new_mesh.color = D3DXVECTOR4(0, 0, 1, 1);
		new_vertices.push_back(new_mesh);
		mesh_r.vertices[i].faces.clear();
	}

	for (int i = 0; i < mesh_r.faces.size(); i++)
	{
		new_indices.push_back(mesh_r.faces[i].vertices_of_face[0]);
		new_indices.push_back(mesh_r.faces[i].vertices_of_face[1]);
		new_indices.push_back(mesh_r.faces[i].vertices_of_face[2]);

		mesh_r.vertices[mesh_r.faces[i].vertices_of_face[0]].faces.push_back(i);
	}

	//calculate normals via smotthing group technique
	for (int i = 0; i < mesh_r.vertices.size(); i++)
	{
		CC_Vertex &v = mesh_r.vertices[i];
		D3DXVECTOR3 normal, f_normal;
		normal = D3DXVECTOR3(0, 0, 0);
		for (int j = 0; j < v.faces.size(); j++)
		{
			normal += get_normal_of_face(v.faces[j], mesh_r);
		}

		D3DXVec3Normalize(&f_normal, &normal);
		new_vertices[i].normal = D3DXVECTOR4(f_normal,0);
	}

	mesh_to_edit_->create_from_buffers(new_vertices, new_indices);
}

D3DXVECTOR3 CatmullClark::get_normal_of_face(int face_index, const CC_MeshRepresenation &mesh_r) const
{
	const CC_Face &face = mesh_r.faces[face_index];
		
	D3DXVECTOR3 v1 = mesh_r.vertices[face.vertices_of_face[0]].position - mesh_r.vertices[face.vertices_of_face[1]].position;
	D3DXVECTOR3 v2 = mesh_r.vertices[face.vertices_of_face[0]].position - mesh_r.vertices[face.vertices_of_face[2]].position;

	D3DXVECTOR3 final, final_n;
	D3DXVec3Cross(&final, &v1, &v2);
	D3DXVec3Normalize(&final_n, &final);

	return final_n;
}

bool CatmullClark::get_twin_edge_midpoint(const CC_MeshRepresenation &mesh_r, int edge_index, int &twin_edge_midpoint) const
{
	
	for (int i = 0; i < mesh_r.edges.size(); i++)
	{
		if (i != edge_index)
		{
			if (mesh_r.edges[i].edge_points[0] == mesh_r.edges[edge_index].edge_points[1] &&
				mesh_r.edges[i].edge_points[1] == mesh_r.edges[edge_index].edge_points[0])
			{
				if (mesh_r.edges[i].newly_added_midpoint != -1)
				{
					twin_edge_midpoint = mesh_r.edges[i].newly_added_midpoint;
					return true;
				}
				else
				{
					return false;
				}
			}

		}
	}

	_ASSERT(false);
	return false;

}
