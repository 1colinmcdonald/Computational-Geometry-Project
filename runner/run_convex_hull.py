#!/usr/bin/env python3
"""
Python script to interact with the convex hull C++ program.
Sends points, triggers computation with RUN, and reads results.
"""

import math
import subprocess
import sys
from pathlib import Path
from subprocess import Popen

from scipy.spatial import ConvexHull

class ConvexHullRunner:
    def __init__(self, executable_path: Path, algo: str):
        self.executable_path: Path = executable_path
        if not self.executable_path.exists():
            raise FileNotFoundError(f"Executable not found: {executable_path}")
        
        # Start the process
        self.process: Popen[str] = subprocess.Popen(
            [str(self.executable_path), algo],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            encoding='utf-8',
            bufsize=1
        )

    def _raise_if_exited(self, where: str):
        rc = self.process.poll()
        if rc is not None:
            err = ""
            if self.process.stderr is not None:
                err = self.process.stderr.read()
            raise RuntimeError(f"Child exited in {where} with code {rc}.\nStderr:\n{err}")

    
    def send_points(self, points: list[tuple[float, float]]):
        assert self.process.stdin is not None

        for i, (x, y) in enumerate(points):
            self._raise_if_exited(f"send_points before point {i}")
            try:
                _ = self.process.stdin.write(f"{x} {y}\n")
            except OSError as e:
                # Pull stderr to see the real cause
                err = ""
                if self.process.stderr is not None:
                    err = self.process.stderr.read()
                raise RuntimeError(f"Write failed at point {i}: {e}\nStderr:\n{err}") from e

        self.process.stdin.flush()
    
    def run_and_get_hull(self):
        # Send RUN command
        assert self.process.stdin is not None, "Process stdin is not available"

        _ = self.process.stdin.write("RUN\n")
        self.process.stdin.flush()
        

        assert self.process.stdout is not None, "Process stdout is not available"

        # Read hull points until we get an empty line
        hull: list[tuple[float, float]] = []
        while True:
            line = self.process.stdout.readline()
            if not line or line.strip() == "":
                break
            
            # Parse CGAL point format: "x y"
            parts = line.strip().split()
            if len(parts) >= 2:
                try:
                    x, y = float(parts[0]), float(parts[1])
                    hull.append((x, y))
                except ValueError:
                    continue
        
        return hull
    
    def close(self):
        """Close the process."""
        if self.process.stdin:
            self.process.stdin.close()
        _ = self.process.wait()


def convex_hull_same(hull1: list[tuple[float, float]], hull2: list[tuple[float, float]]) -> bool:
    rel_tol = 1e-4
    abs_tol = 1e-3

    for i, point in enumerate(hull2):
        if math.isclose(hull1[0][0], point[0], rel_tol=rel_tol, abs_tol=abs_tol) and math.isclose(hull1[0][1], point[1], rel_tol=rel_tol, abs_tol=abs_tol):
            start_index: int = i
            break
    else:
        return False
    
    for i in range(len(hull1)):
        hull1_point = hull1[i]
        hull2_point = hull2[(start_index + i) % len(hull2)]
        if not (math.isclose(hull1_point[0], hull2_point[0], rel_tol=rel_tol, abs_tol=abs_tol) and math.isclose(hull1_point[1], hull2_point[1], rel_tol=rel_tol, abs_tol=abs_tol)):
            return False
        
    return True


def scipy_convex_hull(points: list[tuple[float, float]]) -> list[tuple[float, float]]:
    hull = ConvexHull(points)  # pyright: ignore[reportUnknownVariableType]
    return [tuple(points[i]) for i in hull.vertices]  # pyright: ignore[reportUnknownMemberType, reportUnknownArgumentType, reportUnknownVariableType]

def main():
    # Get path from cli args
    exe_path = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).parent.parent / "bin" / "Debug" / "points.exe"
    
    if not exe_path.exists():
        print(f"Error: Could not find executable at {exe_path}", file=sys.stderr)
        sys.exit(1)
    
    # Create runner
    runner = ConvexHullRunner(exe_path, "jarvis")
    
    try:
        print("Example 1: Square")
        points = [(0., 0.), (1., 0.), (1., 1.), (0., 1.), (0.5, 0.5)]
        runner.send_points(points)
        hull = runner.run_and_get_hull()
        hull2_points = scipy_convex_hull(points)
        print("Hull from C++:", hull)
        print("Hull from scipy:", hull2_points)
        assert convex_hull_same(hull, hull2_points), "Hulls do not match!"


        print("Example 2: Random points")
        import random
        random_points = [(random.uniform(0, 10), random.uniform(0, 10)) for _ in range(100)]
        runner.send_points(random_points)
        hull = runner.run_and_get_hull()
        hull2_points = scipy_convex_hull(random_points)
        print("Hull from C++:", hull)
        print("Hull from scipy:", hull2_points)
        assert convex_hull_same(hull, hull2_points), "Hulls do not match!"

        
    finally:
        runner.close()


if __name__ == "__main__":
    main()
