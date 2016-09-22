
#include "VoronoiSolver.h"
#include "Mesh.h"

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
