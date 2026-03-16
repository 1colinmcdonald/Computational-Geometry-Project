#include <iostream>
#include <random>
#include <cstdlib>
#include <iomanip>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <num_points>\n";
        return 1;
    }
    std::size_t n = std::stoull(argv[1]);
    std::mt19937_64 rng(std::random_device{}());
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    std::cout << std::setprecision(17);
    for (std::size_t i = 0; i < n; ++i) {
        std::cout << dist(rng) << " " << dist(rng) << "\n";
    }
    return 0;
}