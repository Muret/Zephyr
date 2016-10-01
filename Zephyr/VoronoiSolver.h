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



class VoronoiSolver
{
public:
	
	struct VoronoiSite
	{
		D3DXVECTOR2 point;
		D3DXVECTOR3 color;
	};

	struct VoronoiCellInfo
	{
		D3DXVECTOR2 center;
		std::vector<D3DXVECTOR2> vertices;
	};

	VoronoiSolver();
	~VoronoiSolver();

	void calculate(const std::vector<VoronoiSite> &points);
	void increment_uniformity(std::vector<VoronoiSite> &new_points);

	Mesh* get_triangulated_voronoi_mesh();
	Mesh* get_triangulated_voronoi_mesh(const std::vector<VoronoiCellInfo> &voronoi_cells);
	Mesh* get_edge_line_mesh(float smoothing_amount);

	void set_site_colors(const std::vector<D3DXVECTOR3> &color);

	void write_to_file(std::string name);
	void read_from_file(std::string name, std::vector<VoronoiCellInfo> &voronoi_cells);

private:
	voronoi_diagram<double> *vd_;
	std::vector<VoronoiSite> site_points_;
};



#endif