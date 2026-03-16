# pyright: reportAny=false, reportExplicitAny=false, reportUnknownVariableType=false, reportUnknownParameterType=false, reportUnknownArgumentType=false, reportUnknownMemberType=false, reportMissingTypeArgument=false, reportUnusedCallResult=false, reportImplicitStringConcatenation=false
from __future__ import annotations

import argparse
import re
import subprocess
from dataclasses import dataclass
from pathlib import Path
from statistics import mean, stdev
import math

import matplotlib.pyplot as plt  # pyright: ignore[reportMissingImports]
import numpy as np
from scipy.spatial import ConvexHull, QhullError  # pyright: ignore[reportMissingImports]

TRIALS = 3
STEPS = 8
SEED = 42
JARVIS_MAX_N = 5000000
PROJECT_ROOT = Path(__file__).resolve().parents[1]
BENCHMARK_PATH = PROJECT_ROOT / "bin" / "benchmark"
INPUT_DIR = PROJECT_ROOT / "input"
PLOT_PATH = PROJECT_ROOT / "runner" / "results" / "plot.png"
DIST_PLOT_PATH = PROJECT_ROOT / "runner" / "results" / "distribution.png"

ALGO_COLORS: dict[str, str] = {
    "jarvis": "#3a66d6",
    "graham": "#e06c1a",
    "my_graham": "#21a366",
    "ray_shooting_quickhull": "#8e44ad",
    "chan": "#ff0000",
    "toussaint": "#10e422",
    "bykat": "#340c92",
    "cgal_jarvis": "#dee110",
    "melkman": "#1de9f4",
}

ALL_ALGOS = ["jarvis", "graham", "my_graham", "ray_shooting_quickhull", "chan", "toussaint", "bykat", "cgal_jarvis"]

ALL_DISTRIBUTIONS = ["circle", "uniform", "gaussian", "square"]
# Distributions that accept a target hull size
DISTRIBUTIONS_WITH_H: set[str] = {"circle"}


@dataclass(frozen=True)
class RunConfig:
    vary: str
    algorithms: list[str]
    distribution: str
    min_n: int
    max_n: int
    min_h: int
    max_h: int
    plot_path: Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Run convex hull benchmark varying n or h, output one plot."
    )
    parser.add_argument("--vary", choices=["n", "h"], required=True, help="Which parameter to vary")
    parser.add_argument(
        "--algorithms",
        nargs="+",
        default=ALL_ALGOS,
        choices=ALL_ALGOS,
        help="Algorithms to benchmark",
    )
    parser.add_argument(
        "--distribution",
        choices=ALL_DISTRIBUTIONS,
        default="circle",
        help="Point distribution (circle supports --vary h; others ignore target h)",
    )
    parser.add_argument("--min-n", type=int, default=1000, help="Minimum n")
    parser.add_argument("--max-n", type=int, default=50000, help="Maximum n")
    parser.add_argument("--min-h", type=int, default=100, help="Minimum target h")
    parser.add_argument("--max-h", type=int, default=20000, help="Maximum target h")
    parser.add_argument("--plot", type=Path, default=PLOT_PATH, help="Output plot path")
    return parser.parse_args()

def generate_circle_points_with_target_h(n: int, target_h: int, rng: np.random.Generator) -> np.ndarray:
    h = max(3, min(n, target_h))
    pts: list[tuple[float, float]] = []
    for i in range(h):
        angle = 2.0 * math.pi * i / h
        pts.append((4850.0 * math.cos(angle), 4850.0 * math.sin(angle)))
    for _ in range(n - h):
        pts.append((rng.uniform(-800.0, 800.0), rng.uniform(-800.0, 800.0)))
    return np.array(pts, dtype=float)


def generate_uniform_points(n: int, rng: np.random.Generator) -> np.ndarray:
    return rng.uniform(-5000.0, 5000.0, size=(n, 2))


def generate_gaussian_points(n: int, rng: np.random.Generator) -> np.ndarray:
    return rng.normal(0.0, 1500.0, size=(n, 2))


def generate_square_boundary_points(n: int, rng: np.random.Generator) -> np.ndarray:
    corners = np.array([
            [-5000.0, -5000.0],
            [ 5000.0, -5000.0],
            [ 5000.0,  5000.0],
            [-5000.0,  5000.0],
        ], dtype=float)
    rand_n = max(0, n - 4)
    # Generate random points in the interior
    pts: list[tuple[float, float]] = rng.uniform(-4000.0, 4000.0, size=(rand_n, 2)).tolist()
    corners = corners.tolist()
    pts.extend(corners)
    return np.array(pts, dtype=float)

def generate_points(
    distribution: str, n: int, target_h: int, rng: np.random.Generator
) -> np.ndarray:
    if distribution == "circle":
        return generate_circle_points_with_target_h(n, target_h, rng)
    if distribution == "uniform":
        return generate_uniform_points(n, rng)
    if distribution == "gaussian":
        return generate_gaussian_points(n, rng)
    if distribution == "square":
        return generate_square_boundary_points(n, rng)
    raise ValueError(f"Unknown distribution: {distribution}")


def write_points_file(points: np.ndarray, point_file: Path) -> None:
    point_file.parent.mkdir(parents=True, exist_ok=True)
    points_f = np.asarray(points, dtype=float)
    with point_file.open("w", encoding="utf-8", newline="") as f:
        for i in range(points_f.shape[0]):
            x = float(points_f[i, 0])
            y = float(points_f[i, 1])
            f.write(f"{x:.6f} {y:.6f}\n")


def parse_time_ms(stdout: str) -> int:
    match = re.search(r"Time:\s*(\d+)\s*ms", stdout)
    if not match:
        raise RuntimeError("Could not parse benchmark runtime from stdout")
    return int(match.group(1))


def run_benchmark(algorithm: str, n: int) -> int:
    cmd = [str(BENCHMARK_PATH), algorithm, str(n), "custom"]
    proc = subprocess.run(cmd, cwd=PROJECT_ROOT, capture_output=True, text=True, check=False)
    if proc.returncode != 0:
        raise RuntimeError(
            "Benchmark failed.\n"
            + f"Command: {' '.join(cmd)}\n"
            + f"stdout:\n{proc.stdout}\n"
            + f"stderr:\n{proc.stderr}"
        )
    return parse_time_ms(proc.stdout)


def build_config(args: argparse.Namespace) -> RunConfig:
    min_n, max_n = int(args.min_n), int(args.max_n)
    min_h, max_h = int(args.min_h), int(args.max_h)
    distribution = str(args.distribution)
    if min_n <= 0 or min_n > max_n:
        raise ValueError("Need 0 < min-n <= max-n")
    if min_h <= 0 or min_h > max_h:
        raise ValueError("Need 0 < min-h <= max-h")
    if distribution not in DISTRIBUTIONS_WITH_H and str(args.vary) == "h":
        raise ValueError(
            f"Distribution '{distribution}' does not support varying h. "
            "Use --vary n or choose the 'circle' distribution."
        )
    if not BENCHMARK_PATH.exists():
        raise FileNotFoundError(f"Benchmark not found: {BENCHMARK_PATH}")
    plot_path = args.plot if isinstance(args.plot, Path) else PLOT_PATH
    return RunConfig(
        vary=str(args.vary),
        algorithms=[str(a) for a in list(args.algorithms)],
        distribution=distribution,
        min_n=min_n,
        max_n=max_n,
        min_h=min_h,
        max_h=max_h,
        plot_path=plot_path,
    )


def sweep_values(min_val: int, max_val: int) -> list[int]:
    vals = np.linspace(min_val, max_val, num=STEPS)
    return sorted({int(round(v)) for v in vals})


def algorithms_for_n(config: RunConfig, n: int) -> list[str]:
    return [a for a in config.algorithms if not (a == "jarvis" and n > JARVIS_MAX_N)]


# (algo, x_value) -> list of (time_ms, hull_size)
Results = dict[str, dict[int, list[tuple[int, int]]]]


def grouped_stats(results: Results) -> dict[str, dict[int, dict[str, float]]]:
    stats: dict[str, dict[int, dict[str, float]]] = {}
    for algo, by_x in results.items():
        stats[algo] = {}
        for x, pairs in by_x.items():
            times = [t for t, _ in pairs]
            stats[algo][x] = {
                "time_mean": float(mean(times)),
                "time_std": float(stdev(times)) if len(times) > 1 else 0.0,
            }
    return stats


def plot_distribution(points: np.ndarray, distribution: str, path: Path) -> None:
    plt.style.use("seaborn-v0_8-whitegrid")
    fig, ax = plt.subplots(figsize=(6.5, 6.5), dpi=160)
    fig.patch.set_facecolor("#fbfbfd")
    max_pts = min(points.shape[0], 5_000)
    idx = np.random.default_rng(0).choice(points.shape[0], size=max_pts, replace=False) if points.shape[0] > max_pts else np.arange(points.shape[0])
    ax.scatter(points[idx, 0], points[idx, 1], s=8, alpha=0.6, linewidths=0, color="#3a66d6")
    # Draw convex hull
    try:
        hull = ConvexHull(points)
        hull_pts = np.append(hull.vertices, hull.vertices[0])  # close the loop
        ax.plot(points[hull_pts, 0], points[hull_pts, 1], color="#e03030", linewidth=2.5, zorder=5)
    except QhullError:
        pass
    ax.set_title(f"Point distribution: {distribution} (n={points.shape[0]:,})", fontsize=13, weight="bold")
    ax.set_xlabel("x")
    ax.set_ylabel("y")
    ax.set_aspect("equal", adjustable="datalim")
    fig.tight_layout()
    path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(path, bbox_inches="tight")
    plt.close(fig)


def make_plot(results: Results, vary: str, plot_path: Path, fixed_value: int, distribution: str) -> None:
    stats = grouped_stats(results)
    plt.style.use("seaborn-v0_8-whitegrid")
    fig, ax = plt.subplots(figsize=(10.5, 6.2), dpi=160)
    fig.patch.set_facecolor("#fbfbfd")

    for algo in ALL_ALGOS:
        if algo not in stats:
            continue
        algo_stats = stats[algo]
        xs = sorted(algo_stats.keys())
        time_mean = np.array([algo_stats[x]["time_mean"] for x in xs], dtype=float)
        time_std = np.array([algo_stats[x]["time_std"] for x in xs], dtype=float)
        color = ALGO_COLORS[algo]
        ax.plot(xs, time_mean, color=color, linewidth=2.5, marker="o", label=algo)
        ax.fill_between(
            xs,
            np.maximum(0.0, time_mean - time_std),
            time_mean + time_std,
            color=color,
            alpha=0.14,
        )

    if vary == "n":
        h_suffix = f", fixed h={fixed_value}" if distribution in DISTRIBUTIONS_WITH_H else ""
        ax.set_title(f"Runtime vs Input Size ({distribution}{h_suffix})", fontsize=14, weight="bold")
        ax.set_xlabel("Input size n")
    else:
        ax.set_title(
            f"Runtime vs Output Hull Size ({distribution}, fixed n={fixed_value})",
            fontsize=14,
            weight="bold",
        )
        ax.set_xlabel("Hull size h")
    ax.set_ylabel("Time (ms)")
    ax.legend(title="Algorithm", frameon=True, facecolor="white", edgecolor="#dddddd")
    fig.tight_layout()
    plot_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(plot_path, bbox_inches="tight")
    plt.close(fig)


def run_pipeline(config: RunConfig) -> None:
    if config.vary == "n":
        n_values = sweep_values(config.min_n, config.max_n)
        h_fixed = (config.min_h + config.max_h) // 2
        fixed_value = h_fixed
        work_items = [(n, h_fixed, i) for i, n in enumerate(n_values)]
    else:
        h_values = sweep_values(config.min_h, config.max_h)
        n_fixed = (config.min_n + config.max_n) // 2
        fixed_value = n_fixed
        work_items = [(n_fixed, h, i) for i, h in enumerate(h_values)]

    total_runs = sum(len(algorithms_for_n(config, n)) * TRIALS for n, _, _ in work_items)
    print(f"Plot output: {config.plot_path}")
    print(f"Mode:        vary {config.vary}")
    print(f"Algorithms:  {', '.join(config.algorithms)}")
    print(f"Runs:        {total_runs}\n")

    results: Results = {}
    run_idx = 0
    for n, target_h, value_idx in work_items:
        algos = algorithms_for_n(config, n)
        if not algos:
            continue
        if "jarvis" in config.algorithms and "jarvis" not in algos:
            print(f"Skipping jarvis at n={n} (>{JARVIS_MAX_N})")

        for trial in range(1, TRIALS + 1):
            trial_seed = SEED + value_idx * 100_003 + trial * 7_919 + n * 17 + target_h * 13
            rng = np.random.default_rng(trial_seed)
            points = generate_points(config.distribution, n, target_h, rng)
            plot_distribution(points, config.distribution, DIST_PLOT_PATH)
            points_file = INPUT_DIR / f"custom_{n}.txt"
            write_points_file(points, points_file)
            
            for algorithm in algos:
                run_idx += 1
                time_ms = run_benchmark(algorithm, n)
                x = n if config.vary == "n" else target_h
                results.setdefault(algorithm, {}).setdefault(x, []).append((time_ms, x))
                print(
                    f"[{run_idx:>3}/{total_runs}] algo={algorithm:<22} n={n:<8} "
                    f"target_h={target_h:<7} t={time_ms:>5} ms"
                )

    make_plot(results, config.vary, config.plot_path, fixed_value, config.distribution)
    print("\nDone.")


def main() -> None:
    args = parse_args()
    config = build_config(args)
    run_pipeline(config)


if __name__ == "__main__":
    main()
