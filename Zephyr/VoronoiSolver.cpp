
#include "VoronoiSolver.h"
#include "Mesh.h"
#include "Utilities.h"

const float VoronoiSolver::scale_amount = RAND_MAX;

VoronoiSolver::VoronoiSolver()
{

}

VoronoiSolver::~VoronoiSolver()
{
	SAFE_DELETE(vd_);
}

void VoronoiSolver::calculate(const std::vector<VoronoiSite>& points)
{
	SAFE_DELETE(vd_);

	site_points_ = points;

	std::vector<Point> sample_points;
	for (int i = 0; i < points.size(); i++)
	{
		sample_points.push_back(Point(points[i].point.x * RAND_MAX, points[i].point.y * RAND_MAX));
	}

	std::vector<Segment> segments;

	vd_ = new voronoi_diagram<double>();
	// Construction of the Voronoi Diagram.
	construct_voronoi(sample_points.begin(), sample_points.end(),
		segments.begin(), segments.end(),
		vd_);

	//fill neighbours
	for (voronoi_diagram<double>::const_cell_iterator it = vd_->cells().begin(); it != vd_->cells().end(); ++it)
	{
		const voronoi_diagram<double>::cell_type& cell = *it;
		const voronoi_diagram<double>::edge_type* edge = cell.incident_edge();
		// This is convenient way to iterate edges around Voronoi cell.

		int primary_cell_index = cell.source_index();

		if (edge && edge->twin() && edge->twin()->cell())
		{
			int neighbour_index = edge->twin()->cell()->source_index();
			site_points_[primary_cell_index].neighbours.push_back(neighbour_index);
		}
	}

	//fill vertices
	for (voronoi_diagram<double>::const_cell_iterator it = vd_->cells().begin(); it != vd_->cells().end(); ++it)
	{
		const voronoi_diagram<double>::cell_type& cell = *it;
		const voronoi_diagram<double>::edge_type* edge = cell.incident_edge();
		// This is convenient way to iterate edges around Voronoi cell.

		if (cell.contains_point() && edge && edge->vertex0())
		{
			VoronoiSite &cell_info = site_points_[cell.source_index()];
			const voronoi_diagram<double>::edge_type* cur_edge = edge;

			while (cur_edge)
			{
				const voronoi_diagram<double>::vertex_type* v0 = cur_edge->vertex0();

				if (v0)
				{
					cell_info.vertices.push_back(D3DXVECTOR2(v0->x() / scale_amount, v0->y() / scale_amount));
				}

				cur_edge = cur_edge->next();

				if (cur_edge == cell.incident_edge())
				{
					break;
				}
			}

		}
	}
}

Mesh * VoronoiSolver::get_triangulated_voronoi_mesh()
{
	Mesh *voronoi_mesh = new Mesh();
	std::vector<Mesh::Vertex> vertices;
	std::vector<int> indices;

	for (int site_index = 0; site_index < site_points_.size(); site_index++)
	{
		const VoronoiSite &cur_site = site_points_[site_index];

		if (cur_site.vertices.size() > 0)
		{
			Mesh::Vertex new_vertex1;
			new_vertex1.position = D3DXVECTOR4(cur_site.vertices[0].x , cur_site.vertices[0].y, 0, 1);
			new_vertex1.color = D3DXVECTOR4(cur_site.color, 1);

			for (int i = 0; i < cur_site.vertices.size() - 2; i++)
			{
				Mesh::Vertex new_vertex2;
				new_vertex2.position = D3DXVECTOR4(cur_site.vertices[i + 1].x, cur_site.vertices[i + 1].y, 0, 1);
				new_vertex2.color = D3DXVECTOR4(cur_site.color, 1);

				Mesh::Vertex new_vertex3;
				new_vertex3.position = D3DXVECTOR4(cur_site.vertices[i + 2].x, cur_site.vertices[i + 2].y, 0, 1);
				new_vertex3.color = D3DXVECTOR4(cur_site.color, 1);

				indices.push_back(vertices.size());
				vertices.push_back(new_vertex1);
				indices.push_back(vertices.size());
				vertices.push_back(new_vertex2);
				indices.push_back(vertices.size());
				vertices.push_back(new_vertex3);
			}
		}
	}

	voronoi_mesh->create_from_buffers(vertices, indices);
	return voronoi_mesh;
}

Mesh * VoronoiSolver::get_edge_line_mesh(float smoothing_amount)
{
	Mesh *line_mesh = new Mesh();
	line_mesh->set_mesh_type(Mesh::MeshType::line_mesh);

	std::vector<Mesh::Vertex> vertices;
	std::vector<int> indices;
	std::set<void*> twin_edges;

	for (voronoi_diagram<double>::const_edge_iterator it = vd_->edges().begin(); it != vd_->edges().end(); ++it)
	{
		if (it->is_primary() && it->vertex0() && it->vertex1() && it->cell() && it->twin() && it->twin()->cell())
		{
			D3DXVECTOR2 v0(it->vertex0()->x(), it->vertex0()->y());
			D3DXVECTOR2 v1(it->vertex1()->x(), it->vertex1()->y());

			const boost::polygon::voronoi_edge<double> *cur_edge_pointer = it->twin()->twin();;
			const boost::polygon::voronoi_edge<double> *cur_edge_pointer_test = cur_edge_pointer->twin()->twin();;

			if (twin_edges.find((void*)cur_edge_pointer) != twin_edges.end())
			{
				continue;
			}

			twin_edges.insert((void*)it->twin());

			int cell_index = it->cell()->source_index();
			int twin_cell_index = it->twin()->cell()->source_index();

			VoronoiSite new_site = site_points_[cell_index];

			D3DXVECTOR2 cell_center(site_points_[cell_index].point);
			D3DXVECTOR2 twin_cell_center(site_points_[twin_cell_index].point);
			
			D3DXVECTOR2 delunay_edge_vec = (twin_cell_center - cell_center);
			D3DXVECTOR2 delunay_edge_center = (twin_cell_center + cell_center) * 0.5f;
			D3DXVECTOR2 delunay_position = delunay_edge_center + delunay_edge_vec * (Utilities::random_normalized_float() - 0.5f) * smoothing_amount;

			D3DXVECTOR2 voronoi_edge_vec = (v0 - v1);
			D3DXVECTOR2 voronoi_edge_center = (v0 + v1) * 0.5f;
			D3DXVECTOR2 voronoi_position = voronoi_edge_center + voronoi_edge_vec * (Utilities::random_normalized_float() - 0.5f) * smoothing_amount;
			
			D3DXVECTOR2 bezier_control_point = (voronoi_position + delunay_position) * 0.5f;

			static const int number_of_segments = 10;
			static const float dt = 1.0f / float(number_of_segments);

			for (int i = 0; i < number_of_segments; i++)
			{
				float t0 = dt * i;
				float t1 = dt * (i + 1);

				D3DXVECTOR2 first_point = v0 * (t0 * t0) + 2 * bezier_control_point * (1 - t0) * t0 + (1 - t0) * (1 - t0) * v1;
				D3DXVECTOR2 second_point = v0 * (t1 * t1) + 2 * bezier_control_point * (1 - t1) * t1 + (1 - t1) * (1 - t1) * v1;

				Mesh::Vertex new_vertex0;
				new_vertex0.position = D3DXVECTOR4(first_point.x / scale_amount, first_point.y / scale_amount, 0, 1);
				new_vertex0.color = D3DXVECTOR4(new_site.color, 1);

				Mesh::Vertex new_vertex1;
				new_vertex1.position = D3DXVECTOR4(second_point.x / scale_amount, second_point.y / scale_amount, 0, 1);
				new_vertex1.color = D3DXVECTOR4(new_site.color, 1);

				indices.push_back(vertices.size());
				vertices.push_back(new_vertex0);
				indices.push_back(vertices.size());
				vertices.push_back(new_vertex1);
			}


		}
	}

	line_mesh->create_from_buffers(vertices, indices);
	return line_mesh;
}

void VoronoiSolver::set_site_colors(const std::vector<D3DXVECTOR3>& colors)
{
	ZEPHYR_ASSERT(colors.size() == site_points_.size());
	for (int i = 0; i < colors.size(); i++)
	{
		site_points_[i].color = colors[i];
	}
}

void VoronoiSolver::get_site_points(std::vector<VoronoiSite>& sites) const
{
	for (int i = 0; i < site_points_.size(); i++)
	{
		sites.push_back(site_points_[i]);
	}
}

void VoronoiSolver::write_to_file(std::string name)
{
	ofstream file(name, ios::out | ios::binary);

	int cell_count = site_points_.size();
	file.write((char*)&cell_count,sizeof(int));
	for (int i = 0; i < site_points_.size(); i++)
	{
		float center_x = site_points_[i].point.x;
		float center_y = site_points_[i].point.y;
		float center_z = site_points_[i].point.y;

		file.write((char*)&center_x, sizeof(float));
		file.write((char*)&center_y, sizeof(float));
		file.write((char*)&center_z, sizeof(float));

		float color_x = site_points_[i].color.x;
		float color_y = site_points_[i].color.y;
		float color_z = site_points_[i].color.z;

		file.write((char*)&color_x, sizeof(float));
		file.write((char*)&color_y, sizeof(float));
		file.write((char*)&color_z, sizeof(float));

		int count = site_points_[i].vertices.size();
		file.write((char*)&count, sizeof(int));

		for (int j = 0; j < count; j++)
		{
			float x = site_points_[i].vertices[j].x;
			float y = site_points_[i].vertices[j].y;

			file.write((char*)&x, sizeof(float));
			file.write((char*)&y, sizeof(float));
		}

		int neighbour_count = site_points_[i].neighbours.size();
		file.write((char*)&neighbour_count, sizeof(int));

		for (int j = 0; j < neighbour_count; j++)
		{
			int neighbour_index = site_points_[i].neighbours[j];
			file.write((char*)&neighbour_index, sizeof(int));
		}
	}

	file.close();
}

void VoronoiSolver::read_from_file(std::string name)
{
	site_points_.clear();

	ifstream file(name, ios::in | ios::binary);

	int cell_count = 0;
	file.read((char*)&cell_count, sizeof(int));

	for (int i = 0; i < cell_count; i++)
	{
		VoronoiSite new_cell;
		
		float center_x, center_y, center_z;
		file.read((char*)&center_x, sizeof(float));
		file.read((char*)&center_y, sizeof(float));
		file.read((char*)&center_z, sizeof(float));
		
		new_cell.point = D3DXVECTOR3(center_x, center_y, center_z);

		float color_x, color_y, color_z;
		file.read((char*)&color_x, sizeof(float));
		file.read((char*)&color_y, sizeof(float));
		file.read((char*)&color_z, sizeof(float));

		new_cell.color = D3DXVECTOR3(color_x, color_y, color_z);

		int vertices_count;
		file.read((char*)&vertices_count, sizeof(int));

		for (int j = 0; j < vertices_count; j++)
		{
			float vert_x, vert_y;
			file.read((char*)&vert_x, sizeof(float));
			file.read((char*)&vert_y, sizeof(float));

			new_cell.vertices.push_back(D3DXVECTOR2(vert_x, vert_y));
		}

		int neighbour_count;
		file.read((char*)&neighbour_count, sizeof(int));

		for (int j = 0; j < neighbour_count; j++)
		{
			int neighbour_index;
			file.read((char*)&neighbour_index, sizeof(int));
			new_cell.neighbours.push_back(neighbour_index);
		}


		site_points_.push_back(new_cell);
	}

	file.close();

}

void VoronoiSolver::increment_uniformity()
{
	site_points_.clear();
	for (voronoi_diagram<double>::const_cell_iterator it = vd_->cells().begin(); it != vd_->cells().end(); ++it)
	{
		const voronoi_diagram<double>::cell_type& cell = *it;
		const voronoi_diagram<double>::edge_type* edge = cell.incident_edge();

		D3DXVECTOR3 sum(0, 0, 0);
		int count = 0;

		if (cell.contains_point() && edge)
		{
			const voronoi_diagram<double>::vertex_type* starting_vertex = edge->vertex0();
			const voronoi_diagram<double>::edge_type* cur_edge = edge->next();

			VoronoiSite new_site;

			while (1)
			{
				const voronoi_diagram<double>::vertex_type* cur_vertex = cur_edge->vertex0();
				if (cur_vertex)
				{
					sum += D3DXVECTOR3(cur_vertex->x(), cur_vertex->y(), 0);
					count++;
				}

				cur_edge = cur_edge->next();

				if (cur_edge == cell.incident_edge())
				{
					break;
				}
			}

			if (count > 0)
			{
				new_site.point = sum / (float)count;
				site_points_.push_back(new_site);
			}
		}
	}

	calculate(site_points_);
}
