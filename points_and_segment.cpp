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


std::random_device rd;        // Seed source
std::mt19937 gen(rd()); 
    
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
				else if (orientation(hull.back(), q, *p) == CGAL::COLLINEAR && squared_distance(q, hull.back()) > squared_distance(*p, hull.back()))
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

vector<Point_2> ray_shooting_quickhull(std::vector<Point_2> points)
{
	Point_2 v1 = get_lowest(points);
	Point_2 v0(-numeric_limits<double>::infinity(), v1.y());
	vector<Point_2> hull;
	
    Point_2 left_most = get_lowest(points);
    Point_2 right_most = get_highest(points);

    srand (time(NULL));

    if (left_most == right_most) {
        // All points are the same
        return { left_most };
    }

    vector<Point_2> above_set;
    vector<Point_2> below_set;

    double line_slope;
    double line_intercept;
    line_to_slope_and_intercept(left_most, right_most, &line_slope, &line_intercept);

    for (const auto& p : points) {
        double position = line_slope * p.x() + line_intercept - p.y();
        if (position >= 0) {
            above_set.push_back(p);
        } else if (position < 0) {
            below_set.push_back(p);
        }
    }

    // Find farthest point from the line
    double max_dist_above = -1;
    Point_2 farthest_point_above;
    for (const auto& p : above_set) {
        double dist = get_squared_distance_from_line(p, line_slope, line_intercept);
        if (dist > max_dist_above) {
            max_dist_above = dist;
            farthest_point_above = p;
        }
    }

    double max_dist_below = -1;
    Point_2 farthest_point_below;
    for (const auto& p : below_set) {
        double dist = get_squared_distance_from_line(p, line_slope, line_intercept);
        if (dist > max_dist_below) {
            max_dist_below = dist;
            farthest_point_below = p;
        }
    } 


    // Remove points inside triangle, formed by left_most, right_most and farthest_point
    vector<Point_2> new_above_set;
    for (const auto& p : above_set) {
        if (!check_inside_triangle(p, left_most, right_most, farthest_point_above)) {
            new_above_set.push_back(p);
        }
    }
    vector<Point_2> new_below_set;
    for (const auto& p : below_set) {
        if (!check_inside_triangle(p, left_most, right_most, farthest_point_below)) {
            new_below_set.push_back(p);
        }
    }


	return points;
}

vector <Point_2> ray_shooting_quickhull_recurse(std::vector<Point_2> points, Point_2 a, Point_2 b)
{   
    int rand_q_index = rand() % points.size();
    Point_2 q = points[rand_q_index];

    Point_2 v = r - p;
    Point_2 n(-v.y(), v.x()); // Perpendicular vector

}

bool is_above_line(Point_2 p, Point_2 a, Point_2 b)
{
    if (a.x() > b.x())
    {
        return orientation(b, a, p) == CGAL::LEFT_TURN;
    }
    else
    {
        return orientation(a, b, p) == CGAL::LEFT_TURN;
    }
}

void ray_shoot(std::vector<Point_2> points, Point_2 q, Point_2 p, Point_2 r)
{
    Point_2 s, t;
    s = q;
    t = q;

    std::vector<Point_2> Sl;
    std::vector<Point_2> Sr;
    Sl.push_back(q);
    Sr.push_back(q);

    std::shuffle(points.begin(), points.end(), engine);

    for (const auto& pi : points) {
        bool is_above;
        if (s == t) {
            // Check if pi is above the line pr
            is_above = pi.y() > s.y()
        } else {
            // Check if pi is above the line st
            is_above = is_above_line(pi, s, t);
        }

        if (is_above) {
           if (pi.x() < q.x()) {
            // in Sl
            double smallest_slope = numeric_limits<double>::infinity();
            double smallest_slope_ti = 0;
            for (const auto& tprime : Sl) {
                // Slope of pi t'
                double slope_pi_tprime = (tprime.y() - pi.y()) / (tprime.x() - pi.x());
                if (abs(slope_pi_tprime) < smallest_slope) {
                    smallest_slope = abs(slope_pi_tprime);
                }
            }

            t = smallest_slope_ti;


           } else {
            // in Sr
            double smallest_slope = numeric_limits<double>::infinity();
            double smallest_slope_si = 0;
            for (const auto& tprime : Sr) {
                // Slope of pi t'
                double slope_pi_tprime = (tprime.y() - pi.y()) / (tprime.x() - pi.x());
                if (abs(slope_pi_tprime) < smallest_slope) {
                    smallest_slope_si = abs(slope_pi_tprime);
                }
            }

            t = smallest_slope_si;
           }
        }

        if (pi.x() < q.x()) {
            Sl.push_back(pi);
        }
        if (pi.x() > q.x()) {
            Sr.push_back(pi);
        }
    }
}


// From https://gamedev.stackexchange.com/questions/23743/whats-the-most-efficient-way-to-find-barycentric-coordinates
void Barycentric(Point_2 p, Point_2 a, Point_2 b, Point_2 c, float &u, float &v, float &w)
{
    Point_2 v0 = b - a, v1 = c - a, v2 = p - a;
    float d00 = Dot(v0, v0);
    float d01 = Dot(v0, v1);
    float d11 = Dot(v1, v1);
    float d20 = Dot(v2, v0);
    float d21 = Dot(v2, v1);
    float denom = d00 * d11 - d01 * d01;
    v = (d11 * d20 - d01 * d21) / denom;
    w = (d00 * d21 - d01 * d20) / denom;
    u = 1.0f - v - w;
}

double check_inside_triangle(Point_2 p, Point_2 a, Point_2 b, Point_2 c)
{

    if (p == a || p == b || p == c)
        return true;

    double u,v,w;
    Barycentric(p, a, b, c, &u, &v, &w);

    return 0 <= u && u <= 1 && 0 <= v && v <= 1 && 0 <= w && w <= 1;
}

void line_to_slope_and_intercept(Point_2 a, Point_2 b, double& slope, double& intercept)
{
    assert(a.x() != b.x()); // Ensure we don't have vertical lines
    slope = (b.y() - a.y()) / (b.x() - a.x());
    intercept = a.y() - slope * a.x();
}


double get_squared_distance_from_line(Point_2 p, double slope, double intercept)
{
    // Distance from point to line squared = (Ax + By + C)^2 / (A^2 + B^2) where line is Ax + By + C = 0
    double A = -slope;
    double B = 1;
    double C = -intercept;
    return (A * p.x() + B * p.y() + C) * (A * p.x() + B * p.y() + C) / (A * A + B * B);
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

Point_2 get_highest(vector<Point_2> points)
{
	return *max_element(points.begin(), points.end(), y_comp);
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
    std::function<std::vector<Point_2>(std::vector<Point_2>)> convex_hull_algo;
    
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
            convex_hull_algo = jarvis;
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
	
	return 0;
}

