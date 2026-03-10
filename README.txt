# Configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build

# Usage Examples
./build/benchmark jarvis 100000 irregular_hull
./build/benchmark graham 100000 irregular_hull
./build/benchmark jarvis 100000 uniform 
./build/benchmark graham 100000 uniform
./build/benchmark jarvis 100000 many_hull
./build/benchmark graham 100000 many_hull

# Currently only calculates the upper hull
./build/benchmark my_graham 63 random_points

# To view the hull:
open debug.svg
