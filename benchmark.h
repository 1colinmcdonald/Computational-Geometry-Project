#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/ch_graham_andrew.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Point_set_2.h>
#include <CGAL/enum.h>
#include <vector>
#include <chrono>
#ifndef BENCHMARK_H
#define BENCHMARK_H

using namespace std;
typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_2 Point_2;
Point_2 get_lowest(vector<Point_2> points);
void plot_hull(std::vector<Point_2> points, std::vector<Point_2> hull);
int test_jarvis(std::vector<Point_2> pts);
vector<Point_2> jarvis_vector(const std::vector<Point_2> &points);
template <class InputIt, class OutputIt>
OutputIt jarvis(InputIt first, InputIt last, OutputIt out);

#endif
