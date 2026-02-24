#include <iostream>
#include <CGAL/Simple_cartesian.h>
#include <list>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/ch_graham_andrew.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Point_set_2.h>
#include <algorithm> // std::min_element
#include <CGAL/enum.h>

#include <vector>
#include "svg_plot.h"
#include "points_and_segment.h"

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
using namespace std;

typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_2 Point_2;
typedef Kernel::Segment_2 Segment_2;
typedef CGAL::Polygon_2<K> Polygon_2;

vector<Point_2> graham(std::vector<Point_2> points)
{

    return points;
}

vector<Point_2> jarvis(std::vector<Point_2> points)
{
	Point_2 v1 = get_lowest(points);
	Point_2 v0(-numeric_limits<double>::infinity(), v1.y());
	vector<Point_2> hull;
	hull.push_back(v0);
	hull.push_back(v1);
	while (true)
	{
		optional<Point_2> p;
		for (auto& q : points)
		{
			if (q != *(hull.rbegin() + 1) && q != hull.back())
			{
				if (!p)
				{
					p = q;
				}
				else if (orientation(hull.back(), q, *p) == CGAL::LEFT_TURN)
				{
					p = q;
				}
				else if (orientation(hull.back(), q, *p) == CGAL::COLLINEAR and squared_distance(q, hull.back()) > squared_distance(*p, hull.back()))
				{
					p = q;
				}
			}
		}	
		if (p == v1)
		{
			return vector<Point_2>(begin(hull) + 1, end(hull));
		}
		hull.push_back(*p);
	}
	return points;
}

bool y_comp(Point_2 a, Point_2 b)
{
	if (a.y() < b.y()) {
		return true;
	}
	if (a.y() == b.y()) {
		return a.x() <= b.x();
	}
	return false;
}

Point_2 get_lowest(vector<Point_2> points)
{
	return *min_element(points.begin(), points.end(), y_comp);
}



bool same_hull(vector<Point_2> hull1, vector<Point_2> hull2)
{
    if (hull1.size() != hull2.size())
        return false;

    if (hull1.empty())
        return true;

    // Find a matching point
    auto hull1Front = hull1.begin();
    auto it2 = find(hull2.begin(), hull2.end(), *hull1Front);

    // Go through (looping) and verify all points match
    for (size_t i = 0; i < hull1.size(); i++)
    {
        if (hull1[i] != *it2)
            return false;

        it2++;
        if (it2 == hull2.end())
            it2 = hull2.begin();
    }
    return true;
}

// Tests
#define IS_TRUE(x) { if (!(x)) std::cout << __FUNCTION__ << " failed on line " << __LINE__ << std::endl; }

void test_same_hull()
{
    vector<Point_2> hull1 = { Point_2(0,0), Point_2(1,0), Point_2(1,1) };
    vector<Point_2> hull2 = { Point_2(1,0), Point_2(1,1), Point_2(0,0) };
    IS_TRUE(same_hull(hull1, hull2));

    vector<Point_2> hull3 = { Point_2(0,0), Point_2(1,0) };
    IS_TRUE(!same_hull(hull1, hull3));
}

int main()
{
    test_same_hull();
    std::vector<Point_2> points_list;
	CGAL::IO::set_ascii_mode(std::cin);
	CGAL::IO::set_ascii_mode(std::cout);
	SvgPlot plot;
	// 1) Read all input points
	std::vector<Point_2> pts;
	Point_2 p;
	while (std::cin >> p) pts.push_back(p);
	for (auto& q : pts) plot.add_point(q.x(), q.y(), 3, "gray", "gray");

	// 2) Compute convex hull points
	std::vector<Point_2> hull;
	CGAL::ch_graham_andrew(pts.begin(), pts.end(), std::back_inserter(hull));
	std::vector<Point_2> j_hull = jarvis(pts);
	for (Point_2 t : j_hull)
	{
		cout << t << " ";
	}

	cout << "\n";
	Point_2 lowest = get_lowest(pts);
	plot.add_point(lowest.x(), lowest.y(), 4, "red", "red");

	for (auto& q : hull) plot.add_point(q.x(), q.y(), 4, "red", "red");

	std::vector<std::pair<double, double>> poly;
	for (auto& q : j_hull) poly.push_back({q.x(), q.y()});
	plot.add_polyline(poly, 2.5, "blue", true);
	plot.write("debug.svg");
	std::cout << "Wrote debug.svg\n";
	
    // std::cout << output << "\n";
    // Segment_2 s(p, q);
    // std::cout << "Segment: " << s << "\n";
	Point_2 p2(1, 2);
	std::cout << p2.y() << "\n";
    return 0;
}

