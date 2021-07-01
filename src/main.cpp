
#include <iostream>
#include <chrono>
#include <filesystem>
#include "maxflow.hpp"
#include "instance_reader.hpp"


struct Timer {
    decltype(std::chrono::steady_clock::now()) time_start;
    Timer(): time_start{std::chrono::steady_clock::now()} {}
    ~Timer() {
        auto duration = duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - time_start).count();
        std::cout << "Duration: " << duration << "ms\n";
    }
};


auto minimal_example()
{
    FlowNetwork network{.n = 6, .m = 9, .source = 0, .sink = 5, .arcs = {
        {0, 1, 10}, {0, 2, 10}, {1, 2, 2}, {1, 3, 4}, {1, 4, 8},
        {2, 4, 9}, {3, 5, 10}, {4, 3, 6}, {4, 5, 10}
    }}; // maximum flow value of 19

    auto maximum_flow = edmonds_karp(network); // or = DinitzCherkassky{network}();

    std::cout << "Maximum flow value: " << maximum_flow.value << '\n';
    std::cout << "Arcs flow/capacity:\n";
    for (size_t a = 0; a < network.m; ++a) {
        auto const& [u, v, capacity] = network.arcs[a];
        auto const flow = maximum_flow.flow_arcs[a];
        std::cout << "    * (" << u << ',' << v << ") " << flow << '/' << capacity << '\n';
    }    
}


int main(int argc, char* argv[])
{
    // minimal_example();

    if (argc != 2) throw std::runtime_error("No input file.");
    
    auto network = read_maxflow_instance(argv[1]);
    std::cout << "\nNetwork instance: \"" << argv[1] << " - |V| = " << network.n << ", |E| = " << network.m << '\n';

    std::cout << "\nAlgorithm: \"Dinitz-Cherkassky\"\n";
    {
        Timer t;
        std::cout << "Maximum flow value: " << DinitzCherkassky{network}().value << '\n';
    }

    std::cout << "\nAlgorithm: \"Edmonds-Karp\"\n";
    {
        Timer t;
        std::cout << "Maximum flow value: " << edmonds_karp(network).value << '\n';
    }
}