#include <iostream>
#include <CGAL/Simple_cartesian.h>
#include <list>

typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_2 Point_2;
typedef Kernel::Segment_2 Segment_2;

int main()
{
    list<Point_2> points_list;
    Point_2 p(1,1), q(10,10);
    points_list.push_back(p);
    points_list.push_back(q);
    auto output = graham(points_list);
    std::cout << output << "\n";
    // Segment_2 s(p, q);
    // std::cout << "Segment: " << s << "\n";
    return 0;
}

list<Point_2> graham(list<Point_2> points)
{
    return points;
}