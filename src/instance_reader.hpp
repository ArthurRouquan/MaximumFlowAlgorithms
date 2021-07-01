#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include "maxflow.hpp"


FlowNetwork read_maxflow_instance(std::string_view filepath) {
    FlowNetwork instance;
    std::ifstream file;
    file.open(filepath.data());

    for (std::string line; std::getline(file, line);) {
        std::istringstream input(line);
        switch (line[0]) {
            case 'p':
                input.ignore(6);
                input >> instance.n >> instance.m;
                instance.arcs.reserve(instance.m);
                break;
            case 'n':
                input.ignore(2);
                node_t node; char node_type;
                input >> node >> node_type;
                if (node_type == 's') instance.source = node - 1;
                else instance.sink = node - 1;
                break;
            case 'a':
                input.ignore(2);
                CapacityArc arc;
                input >> arc.tail >> arc.head >> arc.capacity;
                --arc.tail; --arc.head;
                instance.arcs.push_back(std::move(arc));
                break;
            default:
                break;
        }
    }

    file.close();
    return instance;
}


flow_t read_maxflow_instance_solution(std::string_view filepath) {
    flow_t maxflow = 0;
    std::ifstream file;
    file.open(filepath.data());

    for (std::string line; std::getline(file, line);) {
        std::istringstream input(line);
        if (line[0] == 's') {
            input.ignore(2);
            input >> maxflow;
        }
    }

    file.close();
    return maxflow;
}
