#include <CGAL/Simple_cartesian.h>
#include <vector>
#ifndef POINTS_AND_SEGMENT_H
#define POINTS_AND_SEGMENT_H

using namespace std;
typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_2 Point_2;
Point_2 get_lowest(vector<Point_2> points);

#endif
