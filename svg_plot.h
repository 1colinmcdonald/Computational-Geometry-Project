#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <limits>
#include <algorithm>

struct SvgPlot {
  struct Pt { double x, y; double r; std::string stroke, fill; };
  struct Seg { double x1,y1,x2,y2; double w; std::string stroke; };
  struct Poly { std::vector<std::pair<double,double>> pts; double w; std::string stroke; bool closed; };

  std::vector<Pt> points;
  std::vector<Seg> segs;
  std::vector<Poly> polys;

  void add_point(double x, double y,
                 double r=3.0,
                 std::string stroke="black",
                 std::string fill="black")
  { points.push_back({x,y,r,std::move(stroke),std::move(fill)}); }

  void add_segment(double x1,double y1,double x2,double y2,
                   double w=2.0,
                   std::string stroke="red")
  { segs.push_back({x1,y1,x2,y2,w,std::move(stroke)}); }

  void add_polyline(const std::vector<std::pair<double,double>>& pts,
                    double w=2.0,
                    std::string stroke="blue",
                    bool closed=true)
  { polys.push_back({pts,w,std::move(stroke),closed}); }

  void write(const std::string& filename,
             int width=900, int height=700,
             int pad=30) const
  {
    // Compute bounds over everything
    auto inf = std::numeric_limits<double>::infinity();
    double minx= inf, miny= inf, maxx=-inf, maxy=-inf;

    auto upd = [&](double x, double y){
      minx = std::min(minx, x); miny = std::min(miny, y);
      maxx = std::max(maxx, x); maxy = std::max(maxy, y);
    };

    for (auto& p: points) upd(p.x,p.y);
    for (auto& s: segs) { upd(s.x1,s.y1); upd(s.x2,s.y2); }
    for (auto& pl: polys) for (auto& q: pl.pts) upd(q.first,q.second);

    if (!std::isfinite(minx)) { // nothing to draw
      std::ofstream out(filename);
      out << "<svg xmlns='http://www.w3.org/2000/svg' width='"<<width<<"' height='"<<height<<"'></svg>\n";
      return;
    }

    // Avoid zero ranges
    double dx = (maxx-minx); if (dx==0) dx=1;
    double dy = (maxy-miny); if (dy==0) dy=1;

    // Map world -> SVG pixels (flip y so “up” is up)
    double sx = (width  - 2.0*pad) / dx;
    double sy = (height - 2.0*pad) / dy;
    double s  = std::min(sx, sy);

    auto X = [&](double x){ return pad + (x - minx)*s; };
    auto Y = [&](double y){ return height - pad - (y - miny)*s; };

    std::ofstream out(filename);
    out << "<svg xmlns='http://www.w3.org/2000/svg' width='"<<width<<"' height='"<<height<<"'>\n";
    out << "<rect x='0' y='0' width='100%' height='100%' fill='white'/>\n";

    // segments
    for (auto& sgm: segs) {
      out << "<line x1='"<<X(sgm.x1)<<"' y1='"<<Y(sgm.y1)
          <<"' x2='"<<X(sgm.x2)<<"' y2='"<<Y(sgm.y2)
          <<"' stroke='"<<sgm.stroke<<"' stroke-width='"<<sgm.w<<"'/>\n";
    }

    // polylines
    for (auto& pl: polys) {
      out << "<polyline fill='none' stroke='"<<pl.stroke<<"' stroke-width='"<<pl.w<<"' points='";
      for (auto& q: pl.pts) out << X(q.first) << "," << Y(q.second) << " ";
      if (pl.closed && !pl.pts.empty())
        out << X(pl.pts.front().first) << "," << Y(pl.pts.front().second) << " ";
      out << "'/>\n";
    }

    // points (draw last so they sit on top)
    for (auto& p: points) {
      out << "<circle cx='"<<X(p.x)<<"' cy='"<<Y(p.y)<<"' r='"<<p.r
          <<"' stroke='"<<p.stroke<<"' fill='"<<p.fill<<"'/>\n";
    }

    out << "</svg>\n";
  }
};
