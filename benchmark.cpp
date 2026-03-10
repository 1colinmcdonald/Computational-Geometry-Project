#include <iostream>
#include <list>
#include <algorithm> // std::min_element

#include <vector>
#include "svg_plot.h"
#include "benchmark.h"

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/ch_graham_andrew.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Point_set_2.h>
#include <CGAL/enum.h>
#include <chrono>
#include <functional>
#include <fstream>

#include <vector>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
using namespace std;

typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_2 Point_2;
typedef Kernel::Segment_2 Segment_2;
typedef CGAL::Polygon_2<K> Polygon_2;

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

int main(int argc, char* argv[])
{
	using std::chrono::high_resolution_clock;
	using std::chrono::duration_cast;
	using std::chrono::duration;
	using std::chrono::milliseconds;
	using AlgoType = std::function<void(std::vector<Point_2>::const_iterator, std::vector<Point_2>::const_iterator, std::back_insert_iterator<std::vector<Point_2>>)>;

	AlgoType algo;

    // Take CLI arg for what algo to run
	if (argc != 4)
	{
		std::cerr << "Must specify algorithm, number of points, and point distribution" << std::endl;
		return 1;
	}
	string algo_arg = argv[1];
	string num_points = argv[2];
	string distribution = argv[3];

	if (algo_arg == "jarvis") {
		algo = [](auto f, auto l, auto o) { return jarvis(f, l, o); };	
	} else if (algo_arg == "graham") {
		algo = [](auto f, auto l, auto o) { return CGAL::ch_graham_andrew(f, l, o); };
	} else if (algo_arg == "my_graham") {
		algo = [](auto f, auto l, auto o) { return graham(f, l, o); };
	}
	string input_filename = "input/" + distribution + "_" + num_points + ".txt";
	std::vector<Point_2> hull;
	std::vector<Point_2> pts;
	std::ifstream in(input_filename);
	if (!in) {
		std::cerr << "Error: not a valid input\n";
		return 1;
	}
	double x, y;
	while (in >> x >> y) {
		pts.emplace_back(x, y);
	}


	auto t1 = std::chrono::high_resolution_clock::now();
	algo(pts.begin(), pts.end(), std::back_inserter(hull));
	auto t2 = std::chrono::high_resolution_clock::now();
	volatile std::size_t sink = hull.size();
	std::cout << "Time: "
			  << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count()
			  << " ms\n";
	plot_hull(pts, hull);


	// 1) Read all input points
	SvgPlot plot;
	for (auto& q : pts) plot.add_point(q.x(), q.y(), 3, "gray", "gray");
    return 0;
}

void plot_hull(std::vector<Point_2> points, std::vector<Point_2> hull)
{
	SvgPlot plot;
	std::vector<std::pair<double, double>> poly;
	for (auto& q : points) plot.add_point(q.x(), q.y(), 1, "gray", "gray");
	for (auto& q : hull) plot.add_point(q.x(), q.y(), 2, "red", "red");
	for (auto& q : hull) poly.push_back({q.x(), q.y()});
	plot.add_polyline(poly, .02, "blue", true);
	plot.write("debug.svg");
	std::cout << "Wrote debug.svg\n";

}

vector<Point_2> jarvis_vector(const std::vector<Point_2>& points)
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

template <class InputIt, class OutputIt>
OutputIt jarvis(InputIt first, InputIt last, OutputIt out)
{
	std::vector<Point_2> points(first, last);
	std::vector<Point_2> hull = jarvis_vector(points);
	for (const auto& p : hull)
		*out++ = p;
	return out;
}

template <class InputIt, class OutputIt>
OutputIt graham(InputIt first, InputIt last, OutputIt out) {
	std::vector<Point_2> points(first, last);
	std::sort(points.begin(), points.end());
	vector<Point_2> l_upper;
	
	l_upper.push_back(points[0]);
	l_upper.push_back(points[1]);
	for (int i = 3; i < points.size(); i++)
	{
		while (l_upper.size() > 1 &&
					orientation(l_upper[l_upper.size() - 2], 
					l_upper[l_upper.size() - 1], 
					points[i]) != CGAL::RIGHT_TURN) {
			l_upper.pop_back();
		}
		l_upper.push_back(points[i]);
	}

	for (auto it = l_upper.begin(); it != l_upper.end(); ++it) {
		*out++ = *it;
	}
	return out;
}

// Tests
#define IS_TRUE(x) { if (!(x)) std::cout << __FUNCTION__ << " failed on line " << __LINE__ << std::endl; }

std::vector<Point_2> test_cgal_graham(std::vector<Point_2> pts)
{
	volatile std::size_t sink = 0;
	std::vector<Point_2> hull;
	for (int i = 0; i < 100; i++)
	{
		hull.clear();
		CGAL::ch_graham_andrew(pts.begin(), pts.end(), std::back_inserter(hull));
		sink += hull.size();
	}
	return hull;
}
