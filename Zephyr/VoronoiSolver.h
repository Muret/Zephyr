#ifndef _VORONOI_SOLVER_H
#define _VORONOI_SOLVER_H


#include "includes.h"
#include "FBXSceneImporter.h"

class Mesh;

#include <boost/polygon/voronoi.hpp>
using boost::polygon::voronoi_builder;
using boost::polygon::voronoi_diagram;
using boost::polygon::x;
using boost::polygon::y;
using boost::polygon::low;
using boost::polygon::high;

struct Point 
{
	float a;
	float b;
	Point(float x, float y) : a(x), b(y) {}
};

struct Segment 
{
	Point p0;
	Point p1;
	Segment(float x1, float y1, float x2, float y2) : p0(x1, y1), p1(x2, y2) {}
};

namespace boost {
	namespace polygon {

		template <>
		struct geometry_concept<Point> {
			typedef point_concept type;
		};

		template <>
		struct point_traits<Point> {
			typedef float coordinate_type;

			static inline coordinate_type get(
				const Point& point, orientation_2d orient) {
				return (orient == HORIZONTAL) ? point.a : point.b;
			}
		};

		template <>
		struct geometry_concept<Segment> {
			typedef segment_concept type;
		};

		template <>
		struct segment_traits<Segment> {
			typedef float coordinate_type;
			typedef Point point_type;

			static inline point_type get(const Segment& segment, direction_1d dir) {
				return dir.to_int() ? segment.p1 : segment.p0;
			}
		};
	}  // polygon
}  // boost

   // Traversing Voronoi edges using edge iterator.
static int iterate_primary_edges1(const voronoi_diagram<double>& vd) 
{
	int result = 0;
	for (voronoi_diagram<double>::const_edge_iterator it = vd.edges().begin();
	it != vd.edges().end(); ++it) {
		if (it->is_primary())
			++result;
	}
	return result;
}

// Traversing Voronoi edges using cell iterator.
static int iterate_primary_edges2(const voronoi_diagram<double> &vd)
{
	int result = 0;
	for (voronoi_diagram<double>::const_cell_iterator it = vd.cells().begin();
	it != vd.cells().end(); ++it) {
		const voronoi_diagram<double>::cell_type& cell = *it;
		const voronoi_diagram<double>::edge_type* edge = cell.incident_edge();
		// This is convenient way to iterate edges around Voronoi cell.
		do {
			if (edge->is_primary())
				++result;
			edge = edge->next();
		} while (edge != cell.incident_edge());
	}
	return result;
}

// Traversing Voronoi edges using vertex iterator.
// As opposite to the above two functions this one will not iterate through
// edges without finite endpoints and will iterate only once through edges
// with single finite endpoint.
static int iterate_primary_edges3(const voronoi_diagram<double> &vd)
{
	int result = 0;
	for (voronoi_diagram<double>::const_vertex_iterator it =
		vd.vertices().begin(); it != vd.vertices().end(); ++it) {
		const voronoi_diagram<double>::vertex_type& vertex = *it;
		const voronoi_diagram<double>::edge_type* edge = vertex.incident_edge();
		// This is convenient way to iterate edges around Voronoi vertex.
		do {
			if (edge->is_primary())
				++result;
			edge = edge->rot_next();
		} while (edge != vertex.incident_edge());
	}
	return result;
}


struct VoronoiSite
{
	D3DXVECTOR2 point;
	D3DXVECTOR3 color;
};


class VoronoiSolver
{
public:
	
	VoronoiSolver();
	~VoronoiSolver();

	void calculate(const std::vector<VoronoiSite> &points);

	Mesh* get_triangulated_voronoi_mesh();
	void increment_uniformity(std::vector<VoronoiSite> &new_points);

private:
	voronoi_diagram<double> *vd_;
	std::vector<VoronoiSite> site_points_;
};



#endif