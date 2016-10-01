
#include "VoronoiSolver.h"
#include "Mesh.h"
#include "Utilities.h"

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
		sample_points.push_back(Point(points[i].point.x, points[i].point.y));
	}

	std::vector<Segment> segments;

	vd_ = new voronoi_diagram<double>();
	// Construction of the Voronoi Diagram.
	construct_voronoi(sample_points.begin(), sample_points.end(),
		segments.begin(), segments.end(),
		vd_);
}

Mesh * VoronoiSolver::get_triangulated_voronoi_mesh()
{
	Mesh *voronoi_mesh = new Mesh();
	std::vector<Mesh::Vertex> vertices;
	std::vector<int> indices;

	for (voronoi_diagram<double>::const_cell_iterator it = vd_->cells().begin(); it != vd_->cells().end(); ++it)
	{
		const voronoi_diagram<double>::cell_type& cell = *it;
		const voronoi_diagram<double>::edge_type* edge = cell.incident_edge();
		// This is convenient way to iterate edges around Voronoi cell.

		if (/*cell.contains_point() && */edge && edge->vertex0())
		{
			const voronoi_diagram<double>::vertex_type* starting_vertex = edge->vertex0();
			const voronoi_diagram<double>::edge_type* start_edge = edge;
			const voronoi_diagram<double>::edge_type* cur_edge = edge->next();

			Mesh::Vertex new_vertex1;
			new_vertex1.position = D3DXVECTOR4(starting_vertex->x(), starting_vertex->y(), 0, 1);
			new_vertex1.color = D3DXVECTOR4(site_points_[cell.source_index()].color, 1 );

			while (cur_edge)
			{
				const voronoi_diagram<double>::vertex_type* v0 = cur_edge->vertex0();
				const voronoi_diagram<double>::vertex_type* v1 = cur_edge->vertex1();

				if (v0 && v1)
				{
					Mesh::Vertex new_vertex2;
					new_vertex2.position = D3DXVECTOR4(v0->x(), v0->y(), 0, 1);
					new_vertex2.color = D3DXVECTOR4(site_points_[cell.source_index()].color, 1);

					Mesh::Vertex new_vertex3;
					new_vertex3.position = D3DXVECTOR4(v1->x(), v1->y(), 0, 1);
					new_vertex3.color = D3DXVECTOR4(site_points_[cell.source_index()].color, 1);

					indices.push_back(vertices.size());
					vertices.push_back(new_vertex1);
					indices.push_back(vertices.size());
					vertices.push_back(new_vertex2);
					indices.push_back(vertices.size());
					vertices.push_back(new_vertex3);
				}

				cur_edge = cur_edge->next();

				if (cur_edge == start_edge)
				{
					break;
				}
			}
		}
	}

	voronoi_mesh->create_from_buffers(vertices, indices);
	return voronoi_mesh;
}

Mesh * VoronoiSolver::get_triangulated_voronoi_mesh(const std::vector<VoronoiCellInfo>& voronoi_cells)
{
	Mesh *voronoi_mesh = new Mesh();
	std::vector<Mesh::Vertex> vertices;
	std::vector<int> indices;

	for (int i = 0; i < voronoi_cells.size(); i++)
	{
		const std::vector<D3DXVECTOR2> &cell_vertices = voronoi_cells[i].vertices;

		Mesh::Vertex new_vertex1;
		new_vertex1.position = D3DXVECTOR4(cell_vertices[0].x, cell_vertices[0].y, 0, 1);
		new_vertex1.color = D3DXVECTOR4(site_points_[i].color.x, site_points_[i].color.y, site_points_[i].color.z, 1);

		for (int j = 0; j < cell_vertices.size() - 2; j++)
		{
			Mesh::Vertex new_vertex2;
			new_vertex2.position = D3DXVECTOR4(cell_vertices[j+1].x, cell_vertices[j+1].y, 0, 1);
			new_vertex2.color = D3DXVECTOR4(site_points_[i].color.x, site_points_[i].color.y, site_points_[i].color.z, 1);

			Mesh::Vertex new_vertex3;
			new_vertex3.position = D3DXVECTOR4(cell_vertices[j+2].x, cell_vertices[j+2].y, 0, 1);
			new_vertex3.color = D3DXVECTOR4(site_points_[i].color.x, site_points_[i].color.y, site_points_[i].color.z, 1);

			indices.push_back(vertices.size());
			vertices.push_back(new_vertex1);
			indices.push_back(vertices.size());
			vertices.push_back(new_vertex2);
			indices.push_back(vertices.size());
			vertices.push_back(new_vertex3);
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
				new_vertex0.position = D3DXVECTOR4(first_point.x, first_point.y, 0, 1);
				new_vertex0.color = D3DXVECTOR4(new_site.color, 1);

				Mesh::Vertex new_vertex1;
				new_vertex1.position = D3DXVECTOR4(second_point.x, second_point.y, 0, 1);
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
	_ASSERT(colors.size() == site_points_.size());
	for (int i = 0; i < colors.size(); i++)
	{
		site_points_[i].color = colors[i];
	}
}

void VoronoiSolver::write_to_file(std::string name)
{
	std::vector<VoronoiCellInfo> voronoi_cells;
	for (voronoi_diagram<double>::const_cell_iterator it = vd_->cells().begin(); it != vd_->cells().end(); ++it)
	{
		const voronoi_diagram<double>::cell_type& cell = *it;
		const voronoi_diagram<double>::edge_type* edge = cell.incident_edge();
		// This is convenient way to iterate edges around Voronoi cell.

		if (cell.contains_point() && edge && edge->vertex0())
		{
			VoronoiCellInfo cell_info;
			cell_info.center = site_points_[cell.source_index()].point;

			const voronoi_diagram<double>::edge_type* cur_edge = edge;

			while (cur_edge)
			{
				const voronoi_diagram<double>::vertex_type* v0 = cur_edge->vertex0();

				if (v0)
				{
					cell_info.vertices.push_back(D3DXVECTOR2(v0->x(), v0->y()));
				}

				cur_edge = cur_edge->next();

				if (cur_edge == cell.incident_edge())
				{
					break;
				}
			}

			voronoi_cells.push_back(cell_info);
		}
	}

	ofstream file(name, ios::out | ios::binary);

	int cell_count = voronoi_cells.size();
	file.write((char*)&cell_count,sizeof(int));
	for (int i = 0; i < voronoi_cells.size(); i++)
	{
		float center_x = voronoi_cells[i].center.x;
		float center_y = voronoi_cells[i].center.y;

		file.write((char*)&center_x, sizeof(float));
		file.write((char*)&center_y, sizeof(float));

		int count = voronoi_cells[i].vertices.size();
		file.write((char*)&count, sizeof(int));

		for (int j = 0; j < count; j++)
		{
			float x = voronoi_cells[i].vertices[j].x;
			float y = voronoi_cells[i].vertices[j].y;

			file.write((char*)&x, sizeof(float));
			file.write((char*)&y, sizeof(float));
		}
	}

	file.close();
}

void VoronoiSolver::read_from_file(std::string name, std::vector<VoronoiCellInfo>& voronoi_cells)
{
	site_points_.clear();

	ifstream file(name, ios::in | ios::binary);

	int cell_count = 0;
	file.read((char*)&cell_count, sizeof(int));

	for (int i = 0; i < cell_count; i++)
	{
		VoronoiCellInfo new_cell;
		
		float center_x, center_y;
		file.read((char*)&center_x, sizeof(float));
		file.read((char*)&center_y, sizeof(float));
		
		new_cell.center = D3DXVECTOR2(center_x, center_y);

		int vertices_count;
		file.read((char*)&vertices_count, sizeof(int));

		for (int j = 0; j < vertices_count; j++)
		{
			float vert_x, vert_y;
			file.read((char*)&vert_x, sizeof(float));
			file.read((char*)&vert_y, sizeof(float));

			new_cell.vertices.push_back(D3DXVECTOR2(vert_x, vert_y));
		}

		voronoi_cells.push_back(new_cell);

		VoronoiSite new_site;
		new_site.point = new_cell.center;
		new_site.color = D3DXVECTOR3(1, 1, 1);

		site_points_.push_back(new_site);
	}

	file.close();

}

void VoronoiSolver::increment_uniformity(std::vector<VoronoiSite>& new_points)
{
	new_points.resize(site_points_.size());

	for (voronoi_diagram<double>::const_cell_iterator it = vd_->cells().begin(); it != vd_->cells().end(); ++it)
	{
		const voronoi_diagram<double>::cell_type& cell = *it;
		const voronoi_diagram<double>::edge_type* edge = cell.incident_edge();

		D3DXVECTOR2 sum(0, 0);
		int count = 0;

		if (cell.contains_point() && edge)
		{
			const voronoi_diagram<double>::vertex_type* starting_vertex = edge->vertex0();
			const voronoi_diagram<double>::edge_type* cur_edge = edge->next();

			VoronoiSite new_site = site_points_[cell.source_index()];

			while (1)
			{
				const voronoi_diagram<double>::vertex_type* cur_vertex = cur_edge->vertex0();
				if (cur_vertex)
				{
					sum += D3DXVECTOR2(cur_vertex->x(), cur_vertex->y());
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
			}

			new_points[cell.source_index()] = new_site;
		}
	}
}
