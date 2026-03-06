#include <iostream>
#include <list>
#include <algorithm> // std::min_element

#include <vector>
#include "svg_plot.h"
#include "benchmark.h"

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

int main(int argc, char* argv[])
{
    std::function<std::vector<Point_2>(std::vector<Point_2>)> convex_hull_algo;

    CGAL::IO::set_ascii_mode(std::cin);
    CGAL::IO::set_ascii_mode(std::cout);
    while (true)
    {
        try {
            std::vector<Point_2> pts;
            std::string line;

            while (std::getline(std::cin, line))
            {
                if (line == "RUN")
                {
                    break;
                }

                // Try to parse line as a point (x y format)
                std::istringstream iss(line);
                double x, y;
                if (iss >> x >> y)
                {
                    pts.push_back(Point_2(x, y));
                }
            }

            // If we hit EOF without RUN, exit
            if (std::cin.eof() && line != "RUN")
            {
                break;
            }

            if (!pts.empty())
            {
                std::vector<Point_2> hull;
                hull = jarvis(pts);

                for (const auto& p : hull)
                {
                    std::cout << p << "\n";
                }
            }
            std::cout << "\n"<< std::flush;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 2;
        } catch (...) {
            std::cerr << "Unknown error occurred." << std::endl;
            return 3;
        }

    }
    // Take CLI arg for what algo to run
    if (argc > 1)
    {
        string algo = argv[1];
        if (algo == "test")
        {
            test_same_hull();
            return 0;
        }
        else if (algo == "graham")
        {
            convex_hull_algo = [](std::vector<Point_2> points)
            {
                std::vector<Point_2> result;
                CGAL::ch_graham_andrew(points.begin(), points.end(),
                                    std::back_inserter(result));
                return result;
            };
        }
        else if (algo == "jarvis")
        {
            convex_hull_algo = jarvis(pts);
        }
        else
        {
            std::cerr << "Unknown algorithm: " << algo << ". Use 'test', 'graham', or 'jarvis'." << std::endl;
            return 1;
        }
    }
    else {
        std::cerr << "No algorithm specified. Use 'test', 'graham', or 'jarvis' as an argument." << std::endl;
        return 1;
    }

	


void plot_hull(std::vector<Point_2> points, std::vector<Point_2> hull)
{
	SvgPlot plot;
	std::vector<std::pair<double, double>> poly;
	for (auto& q : points) plot.add_point(q.x(), q.y(), .01, "gray", "gray");
	for (auto& q : hull) plot.add_point(q.x(), q.y(), .02, "red", "red");
	for (auto& q : hull) poly.push_back({q.x(), q.y()});
	plot.add_polyline(poly, .02, "blue", true);
	plot.write("debug.svg");
	std::cout << "Wrote debug.svg\n";

}

double perf_test(

int main()
{
	CGAL::IO::set_ascii_mode(std::cin);
	CGAL::IO::set_ascii_mode(std::cout);
	// 1) Read all input points
	std::vector<Point_2> pts;
	Point_2 p;
	while (std::cin >> p) pts.push_back(p);

	// 2) Compute convex hull points
	std::vector<Point_2> hull;
	CGAL::ch_graham_andrew(pts.begin(), pts.end(), std::back_inserter(hull));
	// std::vector<Point_2> j_hull = jarvis(pts);
	return 0;
}


