#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/ch_graham_andrew.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Point_set_2.h>
#include <CGAL/enum.h>
#include <vector>
#include <chrono>
#include <functional>
#include <optional>

using namespace std;
typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_2 Point_2;

bool y_comp(Point_2 a, Point_2 b);
bool y_neg_x_comp(Point_2 a, Point_2 b);
bool x_comp(Point_2 a, Point_2 b);
bool x_neg_comp(Point_2 a, Point_2 b);
function<bool(Point_2, Point_2)> make_upper_hull_comp(Point_2 highest);
Point_2 get_highest(const vector<Point_2> &points);
Point_2 get_lowest(vector<Point_2> points);
Point_2 get_highest_by_x(const vector<Point_2> &points);
Point_2 get_lowest_by_x(vector<Point_2> points);
bool same_hull(vector<Point_2> hull1, vector<Point_2> hull2);

void plot_hull(std::vector<Point_2> points, std::vector<Point_2> hull);

vector<Point_2> jarvis_vector(const std::vector<Point_2> &points);

template <class InputIt, class OutputIt>
OutputIt jarvis(InputIt first, InputIt last, OutputIt out);

template <class InputIt, class OutputIt>
OutputIt graham(InputIt first, InputIt last, OutputIt out);

template <class InputIt, class OutputIt>
OutputIt ray_shooting_quickhull(InputIt first, InputIt last, OutputIt out);



vector<Point_2> ray_shooting_quickhull_recurse(std::vector<Point_2> points, Point_2 a, Point_2 b);
std::pair<Point_2, Point_2> ray_shoot(std::vector<Point_2> points, Point_2 q, Point_2 p, Point_2 r);

void test_same_hull();
std::vector<Point_2> test_cgal_graham(std::vector<Point_2> pts);

#endif
