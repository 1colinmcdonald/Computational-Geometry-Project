# Justfile for Computational Geometry Project
default: configure build

# Default build type (can override: `just configure release`)
build_type := "Release"

# Build directory
build_dir := "bin"


configure type=build_type:
    cmake -S . -B {{build_dir}} -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows -DCMAKE_BUILD_TYPE={{type}}

build:
    cmake --build {{build_dir}}


clean:
    rm -rf {{build_dir}}
    rm -rf runner/.venv

run:
    uv run runner/run_convex_hull.py bin/Debug/points.exe