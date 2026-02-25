# Justfile for Computational Geometry Project
default: configure build

# Default build type (can override: `just configure release`)
build_type := "Release"

# Build directory
build_dir := "bin"

# -----------------------------
# Configure CMake
# -----------------------------
configure type=build_type:
    cmake -S . -B {{build_dir}} -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows -DCMAKE_BUILD_TYPE={{type}}
# -----------------------------
# Build
# -----------------------------
build:
    cmake --build {{build_dir}}

# -----------------------------
# Clean build directory
# -----------------------------
clean:
    rm -rf {{build_dir}}
    rm -rf runner/.venv

# -----------------------------
# Run Python runner
# -----------------------------
run:
    uv run runner/run_convex_hull.py bin/Debug/points.exe