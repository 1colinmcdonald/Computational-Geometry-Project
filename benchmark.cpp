#include <iostream>
#include <list>
#include <algorithm> // std::min_element
#include <random>
#include <set>

#include <vector>
#include "svg_plot.h"
#include "benchmark.h"

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/ch_graham_andrew.h>
#include <CGAL/ch_akl_toussaint.h>
#include <CGAL/ch_bykat.h>
#include <CGAL/ch_eddy.h>
#include <CGAL/ch_jarvis.h>
#include <CGAL/ch_melkman.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Point_set_2.h>
#include <CGAL/enum.h>
#include <chrono>
#include <functional>
#include <fstream>
#include <cassert>
#include <iterator>
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
using namespace std;

typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_2 Point_2;
typedef Kernel::Segment_2 Segment_2;
typedef CGAL::Polygon_2<K> Polygon_2;
constexpr int LR = 1;
constexpr int LS = 2;
constexpr int LL = 4;
constexpr int SL = 8;
constexpr int RL = 16;
constexpr int RS = 32;
constexpr int RR = 64;
constexpr int SR = 128;

std::random_device rd; // Seed source
std::mt19937 gen(rd());

bool y_comp(Point_2 a, Point_2 b) {
    if (a.y() < b.y()) {
        return true;
    }
    if (a.y() == b.y()) {
        return a.x() < b.x();
    }
    return false;
}

bool y_neg_x_comp(Point_2 a, Point_2 b) {
    if (a.y() < b.y()) {
        return true;
    }
    if (a.y() == b.y()) {
        return a.x() > b.x();
    }
    return false;
}

bool x_comp(Point_2 a, Point_2 b) {
    if (a.x() < b.x()) {
        return true;
    }
    if (a.x() == b.x()) {
        return a.y() < b.y();
    }
    return false;
}

bool x_neg_comp(Point_2 a, Point_2 b) {
    if (a.x() < b.x()) {
        return true;
    }
    if (a.x() == b.x()) {
        return a.y() > b.y();
    }
    return false;
}

function<bool(Point_2, Point_2)> make_upper_hull_comp(Point_2 highest) {
    return [highest](Point_2 a, Point_2 b) {
        if (a.x() < b.x()) {
            return true;
        }
        if (a.x() > b.x()) {
            return false;
        }
        if (a.x() <= highest.x()) {
            return a.y() < b.y();
        }
        return a.y() > b.y();
    };
}

// vector<Point_2> jarvis_vector(const std::vector<Point_2> &points)
Point_2 get_highest(const vector<Point_2> &points) {
    return *max_element(points.begin(), points.end(), y_neg_x_comp);
}

template<class InputIt>
Point_2 get_lowest(InputIt first, InputIt last) {
    return *min_element(first, last, y_comp);
}

Point_2 get_highest_by_x(const vector<Point_2> &points) {
    return *max_element(points.begin(), points.end(), x_neg_comp);
}

Point_2 get_lowest_by_x(vector<Point_2> points) {
    return *min_element(points.begin(), points.end(), x_comp);
}

bool same_hull_simple(vector<Point_2> hull1, vector<Point_2> hull2) {
    set<Point_2> set1(hull1.begin(), hull1.end());
    set<Point_2> set2(hull2.begin(), hull2.end());
    return set1 == set2;
}

bool same_hull(vector<Point_2> hull1, vector<Point_2> hull2) {
    if (hull1.size() != hull2.size())
        return false;

    if (hull1.empty())
        return true;

    // Find a matching point
    auto hull1Front = hull1.begin();
    auto it2 = find(hull2.begin(), hull2.end(), *hull1Front);

    // Go through (looping) and verify all points match
    for (size_t i = 0; i < hull1.size(); i++) {
        if (hull1[i] != *it2)
            return false;

        it2++;
        if (it2 == hull2.end())
            it2 = hull2.begin();
    }
    return true;
}

// Tests
#define IS_TRUE(x)                                                                    \
	{                                                                                 \
		if (!(x))                                                                     \
			std::cout << __FUNCTION__ << " failed on line " << __LINE__ << std::endl; \
	}

void test_same_hull() {
    vector<Point_2> hull1 = {Point_2(0, 0), Point_2(1, 0), Point_2(1, 1)};
    vector<Point_2> hull2 = {Point_2(1, 0), Point_2(1, 1), Point_2(0, 0)};
    IS_TRUE(same_hull(hull1, hull2));

    vector<Point_2> hull3 = {Point_2(0, 0), Point_2(1, 0)};
    IS_TRUE(!same_hull(hull1, hull3));
}

int main(int argc, char *argv[]) {
    using std::chrono::duration;
    using std::chrono::duration_cast;
    using std::chrono::high_resolution_clock;
    using std::chrono::milliseconds;
    using AlgoType = std::function<void(std::vector<Point_2>::const_iterator, std::vector<Point_2>::const_iterator,
                                        std::back_insert_iterator<std::vector<Point_2> >)>;

    AlgoType algo;

    // Take CLI arg for what algo to run
    if (argc != 4) {
        std::cerr << "Must specify algorithm, number of points, and point distribution" << std::endl;
        return 1;
    }
    Point_2 p1(-10, 0);
    Point_2 p2(10, 0);
    Point_2 p3(0, 0);
    int orient_2_same = orientation(p1, p2, p3);
    cout << "orient_2_same" << orient_2_same << endl;
    string algo_arg = argv[1];
    string num_points = argv[2];
    string distribution = argv[3];

    if (algo_arg == "jarvis") {
        algo = [](auto f, auto l, auto o) { return jarvis(f, l, o); };
    } else if (algo_arg == "graham") {
        algo = [](auto f, auto l, auto o) { return CGAL::ch_graham_andrew(f, l, o); };
    } else if (algo_arg == "my_graham") {
        algo = [](auto f, auto l, auto o) { return graham(f, l, o); };
    } else if (algo_arg == "ray_shooting_quickhull") {
        algo = [](auto f, auto l, auto o) { return ray_shooting_quickhull(f, l, o); };
    } else if (algo_arg == "toussaint") {
        algo = [](auto f, auto l, auto o) {
            return CGAL::ch_akl_toussaint(f, l, o);
        };
    } else if (algo_arg == "bykat") {
        algo = [](auto f, auto l, auto o) {
            return CGAL::ch_bykat(f, l, o);
        };
    } else if (algo_arg == "cgal_jarvis") {
        algo = [](auto f, auto l, auto o) {
            return CGAL::ch_jarvis(f, l, o);
        };
    } else if (algo_arg == "melkman") {
        algo = [](auto f, auto l, auto o) {
            return CGAL::ch_melkman(f, l, o);
        };
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

    vector<Point_2> cond;
    // conditional_hull(pts.begin(), pts.end(), 2, cond);
    auto t1 = std::chrono::high_resolution_clock::now();
    algo(pts.begin(), pts.end(), std::back_inserter(hull));
    auto t2 = std::chrono::high_resolution_clock::now();
    volatile std::size_t sink = hull.size();
    std::cout << "Time: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count()
            << " ms\n";
    plot_hull(pts, hull);
    vector<Point_2> correct_hull;
    CGAL::ch_bykat(pts.begin(), pts.end(), std::back_inserter(correct_hull));
    assert((same_hull_simple(hull, correct_hull)));
    if (same_hull_simple(hull, correct_hull)) {
        cout << "Correct hull found" << endl;
    } else {
        cout << "Incorrect hull found" << endl;
    }
    return 0;
}

void plot_hull(std::vector<Point_2> points, std::vector<Point_2> hull) {
    SvgPlot plot;

    std::vector<std::pair<double, double> > poly;
    for (auto &q: points)
        plot.add_point(q.x(), q.y(), 10, "gray", "gray");
    for (auto &q: hull)
        plot.add_point(q.x(), q.y(), 20, "red", "red");
    for (auto &q: hull)
        poly.push_back({q.x(), q.y()});
    for (auto& p : hull) {
        cout << "(" << p << ")";
    }
    cout << "" << endl;
    const Point_2 q(200,200);
    const auto t_ll_it = get_tangent_point(hull.begin(),hull.end(),
                                                                        hull.begin(), hull.end() - 1,
                                                                        q, LL | SL);
    const auto t_ll = *t_ll_it;
    const auto t_rr_it = get_tangent_point(hull.begin(),hull.end(),
                                                                        hull.begin(), hull.end() - 1,
                                                                        q, RR | RS);
    const auto t_rr = *t_rr_it;
    plot.add_point(q.x(), q.y(), 5, "green", "green");
    plot.add_point(t_ll.x(), t_ll.y(), 5, "orange", "orange");
    plot.add_point(t_rr.x(), t_rr.y(), 5, "orange", "orange");
    vector<pair<double, double>> t1;
    vector<pair<double, double>> t2;
    t1.push_back({q.x(), q.y()});
    t1.push_back({t_ll.x(), t_ll.y()});
    t2.push_back({q.x(), q.y()});
    t2.push_back({t_rr.x(), t_rr.y()});
    plot.add_polyline(t1, 2, "purple", true);
    plot.add_polyline(t2, 2, "purple", true);
    plot.add_polyline(poly, 2, "blue", true);
    plot.write("debug.svg");
    std::cout << "Wrote debug.svg\n";
}

vector<Point_2> jarvis_vector(const std::vector<Point_2> &points) {
    Point_2 v1 = get_lowest(points.begin(), points.end());
    Point_2 v0(-numeric_limits<double>::infinity(), v1.y());
    vector<Point_2> hull;
    hull.push_back(v0);
    hull.push_back(v1);
    while (true) {
        optional<Point_2> p;
        for (auto &q: points) {
            if (q != *(hull.rbegin() + 1) && q != hull.back()) {
                if (!p) {
                    p = q;
                } else if (orientation(hull.back(), q, *p) == CGAL::LEFT_TURN) {
                    p = q;
                } else if (orientation(hull.back(), q, *p) == CGAL::COLLINEAR && squared_distance(q, hull.back()) >
                           squared_distance(*p, hull.back())) {
                    p = q;
                }
            }
        }
        if (p == v1) {
            return vector<Point_2>(begin(hull) + 1, end(hull));
        }
        hull.push_back(*p);
    }
    return points;
}

template<class InputIt, class OutputIt>
OutputIt jarvis(InputIt first, InputIt last, OutputIt out) {
    std::vector<Point_2> points(first, last);
    std::vector<Point_2> hull = jarvis_vector(points);
    for (const auto &p: hull)
        *out++ = p;
    return out;
}

// Assumption: No duplicate points. We might see if we can handle this edge case
// Warning: Probably buggy
template<class InputIt, class OutputIt>
OutputIt graham(InputIt first, InputIt last, OutputIt out) {
    std::vector<Point_2> points(first, last);
    Point_2 highest = get_highest(points);
    std::sort(points.begin(), points.end(), make_upper_hull_comp(highest));
    vector<Point_2> l_upper;

    l_upper.push_back(points[0]);
    l_upper.push_back(points[1]);
    for (int i = 2; i < points.size(); i++) {
        while (l_upper.size() > 1 &&
               orientation(l_upper[l_upper.size() - 2],
                           l_upper[l_upper.size() - 1],
                           points[i]) != CGAL::RIGHT_TURN) {
            l_upper.pop_back();
        }
        l_upper.push_back(points[i]);
    }

    size_t l_upper_size = l_upper.size();
    for (int i = points.size() - 1; i >= 0; i--) {
        while (l_upper.size() > l_upper_size + 1 && orientation(l_upper[l_upper.size() - 2],
                                                                l_upper[l_upper.size() - 1],
                                                                points[i]) != CGAL::RIGHT_TURN) {
            l_upper.pop_back();
        }
        l_upper.push_back(points[i]);
    }

    for (auto it = l_upper.end() - 1; it != l_upper.begin() - 1; --it) {
        *out++ = *it;
    }
    return out;
}

template<class InputIt, class OutputIt>
OutputIt ray_shooting_quickhull(InputIt first, InputIt last, OutputIt out) {
    std::vector<Point_2> points(first, last);
    Point_2 p = get_lowest_by_x(points);
    Point_2 r = get_highest_by_x(points);

    if (p == r) {
        // All points are the same
        *out++ = p;
        return out;
    }

    vector<Point_2> above_set;
    vector<Point_2> below_set;

    for (const auto &x: points) {
        if (x == p || x == r)
            continue;
        auto o = CGAL::orientation(p, r, x);
        if (o == CGAL::LEFT_TURN)
            above_set.push_back(x);
        else if (o == CGAL::RIGHT_TURN)
            below_set.push_back(x);
    }

    vector<Point_2> hull_above = ray_shooting_quickhull_recurse(above_set, p, r);
    vector<Point_2> hull_below = ray_shooting_quickhull_recurse(below_set, r, p);

    *out++ = p;
    out = std::copy(hull_above.begin(), hull_above.end(), out);
    *out++ = r;
    out = std::copy(hull_below.begin(), hull_below.end(), out);

    return out;
}

vector<Point_2> ray_shooting_quickhull_recurse(std::vector<Point_2> points, Point_2 a, Point_2 b) {
    if (points.empty()) {
        return {};
    }

    std::uniform_int_distribution<size_t> dist(0, points.size() - 1);
    Point_2 q = points[dist(gen)];

    std::pair<Point_2, Point_2> result = ray_shoot(points, q, a, b);
    Point_2 s = result.first;
    Point_2 t = result.second;

    // Prune points in polygon!
    vector<Point_2> points_above_as;
    vector<Point_2> points_above_tb;

    for (const auto &pi: points) {
        // Skip points that we already know about.
        if (pi == s || pi == t || pi == a || pi == b)
            continue;

        // Point in left triangle
        if (CGAL::orientation(a, s, pi) == CGAL::LEFT_TURN) {
            points_above_as.push_back(pi);
        }

        // Point in right triangle
        else if (CGAL::orientation(t, b, pi) == CGAL::LEFT_TURN) {
            points_above_tb.push_back(pi);
        }
    }

    vector<Point_2> left_hull = ray_shooting_quickhull_recurse(points_above_as, a, s);
    vector<Point_2> right_hull = ray_shooting_quickhull_recurse(points_above_tb, t, b);

    vector<Point_2> hull;
    hull.insert(hull.end(), left_hull.begin(), left_hull.end());

    // Ensure we don't add duplicates!
    if (s != a && s != b) {
        hull.push_back(s);
    }
    if (t != s && t != a && t != b) {
        hull.push_back(t);
    }

    hull.insert(hull.end(), right_hull.begin(), right_hull.end());

    return hull;
}

std::pair<Point_2, Point_2> ray_shoot(std::vector<Point_2> points, Point_2 q, Point_2 p, Point_2 r) {
    // Ensure we give it the boundaries
    points.push_back(p);
    points.push_back(r);

    Point_2 s, t;
    s = q;
    t = q;

    std::vector<Point_2> Sl;
    std::vector<Point_2> Sr;
    Sl.push_back(q);
    Sr.push_back(q);

    std::shuffle(points.begin(), points.end(), gen);
    double dx = r.x() - p.x();
    double dy = r.y() - p.y();
    Point_2 q_parallel(q.x() + dx, q.y() + dy);
    Point_2 q_up(q.x() - dy, q.y() + dx);

    for (const auto &pi: points) {
        bool is_above;

        if (s == t) {
            is_above = (CGAL::orientation(q, q_parallel, pi) == CGAL::LEFT_TURN);
        } else {
            is_above = (CGAL::orientation(s, t, pi) == CGAL::LEFT_TURN);
        }

        if (is_above) {
            if (CGAL::orientation(q, q_up, pi) == CGAL::LEFT_TURN) {
                // in Sl
                Point_2 smallest_slope_ti = q;
                for (const auto &tprime: Sr) {
                    // Check smallest_slope_ti vs the new candidate
                    auto orient = orientation(pi, smallest_slope_ti, tprime);

                    if (orient == CGAL::LEFT_TURN) {
                        smallest_slope_ti = tprime;
                    }
                }

                t = smallest_slope_ti;
                s = pi;
            } else {
                // in Sr
                Point_2 smallest_slope_si = q;
                for (const auto &sprime: Sl) {
                    // Check smallest_slope_si vs the new candidate
                    auto orient = orientation(smallest_slope_si, pi, sprime);

                    if (orient == CGAL::LEFT_TURN) {
                        smallest_slope_si = sprime;
                    }
                }

                s = smallest_slope_si;
                t = pi;
            }
        }

        if (CGAL::orientation(q, q_up, pi) == CGAL::LEFT_TURN) {
            Sl.push_back(pi);
        } else if (pi != q && CGAL::orientation(q, q_up, pi) != CGAL::LEFT_TURN) {
            Sr.push_back(pi);
        }
    }

    return {s, t};
}

template <typename Matrix>
void print_2d(const Matrix& m) {
    for (const auto& row : m) {
        for (const auto& x : row) {
            cout << x << " ";
        }
        cout << "\n";
    }
}

template<class RandomIt>
int bin_search(RandomIt first, RandomIt last, int target) {
    RandomIt l = first;
    RandomIt r = last - 1;
    while (l <= r) {
        RandomIt mid = l + (r - l) / 2;
        if (*mid == target) {
            return mid - first;
        }
        if (*mid < target) {
            l = mid + 1;
        }
        else {
            r = mid - 1;
        }
    }
    return -1;
}

template<class RandomIt>
pair<Point_2, Point_2> get_neighbors(RandomIt first, RandomIt last, RandomIt curr) {
    pair<Point_2, Point_2> result;
    if (curr == first) {
        result.first = *(last - 1);
    } else {
        result.first = *(first + (curr - first - 1));
    }
    if (curr == last - 1) {
        result.second = *(first);
    } else {
        result.second = *(first + (curr - first + 1));
    }
    return result;
}

template<class RandomIt>
pair<Point_2, Point_2> get_tangent_points_linear(RandomIt first, RandomIt last, Point_2 p) {
    pair<Point_2, Point_2> result;
    RandomIt curr = first;
    Point_2 prev;
    Point_2 next;
    while (curr != last) {
        if (pair<Point_2, Point_2> neighbors = get_neighbors(first, last, curr);
            orientation(p, *curr, neighbors.first) == CGAL::LEFT_TURN
            and orientation(p, *curr, neighbors.second) == CGAL::LEFT_TURN) {
            result.first = *curr;
            break;
        }
        curr += 1;
    }
    curr = first;
    while (curr != last) {
        if (pair<Point_2, Point_2> neighbors = get_neighbors(first, last, curr);
            orientation(p, *curr, neighbors.first) == CGAL::RIGHT_TURN
            and orientation(p, *curr, neighbors.second) == CGAL::RIGHT_TURN) {
            result.second = *curr;
            break;
        }
        curr += 1;
    }
    return result;
}

// Requires hull to be in counter-clockwise order.
template<class RandomIt>
RandomIt get_tangent_point(RandomIt beginning, RandomIt end, RandomIt l, RandomIt r, Point_2 p, int target_orient) {
    // cout << "l index: " << l - beginning << endl;
    // cout << "r index: " << r - beginning << endl;
    if (l > r) {
        // cout << "Couldn't find it!" << endl;
        return r;
    }
    RandomIt mid = l + (r - l) / 2;

    const int mid_orient_id = get_orient_id(beginning, end, mid, p);
    const int first_orient_id = get_orient_id(beginning, end, l, p);
    const int last_orient_id = get_orient_id(beginning, end, r, p);
    if (mid_orient_id & target_orient) {
        // cout << "Found it!" << endl;
        return mid;
    }
    if (first_orient_id <= mid_orient_id) {
        if (first_orient_id <= target_orient && target_orient < mid_orient_id) {
            return get_tangent_point(beginning, end, l, mid, p, target_orient);
        }
        return get_tangent_point(beginning, end, mid + 1, r, p, target_orient);
    }
    if (mid_orient_id < target_orient && target_orient <= last_orient_id) {
        return get_tangent_point(beginning, end, mid + 1, r, p, target_orient);
    }
    return get_tangent_point(beginning,end, l, mid, p, target_orient);
}

template<class RandomIt>
int get_orient_id(RandomIt beginning, RandomIt end, RandomIt on_hull, Point_2 p) {
    pair<Point_2, Point_2> neighbors = get_neighbors(beginning, end, on_hull);
    // cout << "prev neighbor" << neighbors.first << endl;
    // cout << "next neighbor" << neighbors.second << endl;
    const int op = orientation(p, *on_hull, neighbors.first);
    const int on = orientation(p, *on_hull, neighbors.second);
    // cout << "op: " << op << endl;
    // cout << "on: " << on << endl;
    if (op == CGAL::LEFT_TURN) {
        if (on == CGAL::RIGHT_TURN) {
            return LR;
        }
        if (on == CGAL::LEFT_TURN) {
            return LL;
        }
        return LS;
    }
    if (op == CGAL::RIGHT_TURN) {
        if (on == CGAL::LEFT_TURN) {
            return RL;
        }
        if (on == CGAL::RIGHT_TURN) {
            return RR;
        }
        return RS;
    }
    if (on == CGAL::LEFT_TURN) {
        return SL;
    }
    return SR;
}

template<class RandomIt>
pair<Point_2, Point_2> get_tangent_points(RandomIt first, RandomIt last, Point_2 p) {
    pair<Point_2, Point_2> result;
    RandomIt l = first;
    RandomIt r = last - 1;
    int n = last - first;
    while (l <= r) {
        RandomIt mid = l + (r - l) / 2;
        Point_2 q = *mid;
        pair<Point_2, Point_2> neighbors = get_neighbors(first, last, mid);
        if (orientation(p, q, neighbors.first) == CGAL::LEFT_TURN) {
            r = mid - 1;
        }
        else if (orientation(p, q, neighbors.second) == CGAL::LEFT_TURN) {
            l = mid + 1;
        } else {
            result.first = q;
            break;
        }
    }
    // cout << "done getting the first" << endl;
    l = first;
    r = last - 1;
    while (l <= r) {
        RandomIt mid = l + (r - l) / 2;
        // cout << mid - first << endl;
        Point_2 q = *mid;
        Point_2 prev = *(first + (mid - first - 1) % n);
        Point_2 next = *(first + (mid - first + 1) % n);
        // cout << "q: " << q << endl;
        // cout << "prev: " << prev << endl;
        // cout << "next: " << next << endl;
        // cout << "\n\n" << endl;
        if (orientation(p, q, prev) == CGAL::RIGHT_TURN) {
            r = mid - 1;
        }
        else if (orientation(p, q, next) == CGAL::RIGHT_TURN) {
            l = mid + 1;
        } else {
            result.second = q;
            // cout << "second x:" << result.second.x() << endl;
            break;
        }
    }
    // cout << "l: " << l - first << endl;
    // cout << "r: " << r - first << endl;
    // cout << "done getting the last" << endl;
    // cout << result.first << " " << result.second << endl;
    return result;
}

template<class InputIt, class OutputIt>
bool conditional_hull(InputIt first, InputIt last, int h_star, OutputIt out) {
    const int k = ceil(static_cast<double>(distance(first, last)) / h_star);
    cout << k << endl;
    vector<vector<Point_2>> disjoint_P;
    for (int i = 0; i < k; i++) {
        disjoint_P.emplace_back(first + i * h_star, std::min(first + i * h_star +h_star, last));
    }
    vector<vector<Point_2>> convex_hulls;
    for (const auto& P : disjoint_P) {
        vector<Point_2> hull;
        graham(P.begin(), P.end(), back_inserter(hull));
        convex_hulls.push_back(hull);
    }
    vector<Point_2> result;
    result.emplace_back(-numeric_limits<double>::infinity(), 0);
    result.push_back(get_lowest(first, last));
    for (int i = 0; i < h_star - 1; i++) {
        vector<pair<Point_2, Point_2>> tangent_points;

    }
    print_2d(disjoint_P);
    // cout << disjoint_P << endl;
    // for (int j = 0; j < k; j++) {
    //
    // }
    // vector<Point_2> Hj
    return true;
}


std::vector<Point_2> test_cgal_graham(std::vector<Point_2> pts) {
    volatile std::size_t sink = 0;
    std::vector<Point_2> hull;
    for (int i = 0; i < 100; i++) {
        hull.clear();
        CGAL::ch_graham_andrew(pts.begin(), pts.end(), std::back_inserter(hull));
        sink += hull.size();
    }
    return hull;
}
