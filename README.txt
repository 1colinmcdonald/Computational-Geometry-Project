# Configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo

or for performance testing:
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build
