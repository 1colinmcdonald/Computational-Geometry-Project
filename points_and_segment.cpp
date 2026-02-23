#include <iostream>
#include <CGAL/Simple_cartesian.h>
#include <list>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/ch_graham_andrew.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
using namespace std;

typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_2 Point_2;
typedef Kernel::Segment_2 Segment_2;

vector<Point_2> graham(std::vector<Point_2> points)
{

    return points;
}

// vector<Point_2> jarvis(std::vector<Point_2> points)
// {
//     auto v0 = numeric_limits<double>::infinity();
// }


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
    Point_2 p(1,1), q(10,10);
    points_list.push_back(p);
    points_list.push_back(q);
    auto output = graham(points_list);
    std::cout << "yo" << "\n";
	CGAL::IO::set_ascii_mode(std::cin);
	CGAL::IO::set_ascii_mode(std::cout);
	std::istream_iterator< Point_2 > in_start( std::cin );
	std::istream_iterator< Point_2 > in_end;
	std::ostream_iterator< Point_2 > out( std::cout, "\n" );
	CGAL::ch_graham_andrew( in_start, in_end, out );
    // std::cout << output << "\n";
    // Segment_2 s(p, q);
    // std::cout << "Segment: " << s << "\n";
    return 0;
}

