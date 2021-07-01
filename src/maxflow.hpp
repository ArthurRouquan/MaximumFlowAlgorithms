#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <ranges>
#include <span>
#include <vector>


/* For educational purposes, there is no need to use templates here. Let's keep the code simple. */
using size_t = std::size_t;
using node_t = std::uint32_t; // nodes are represented with an unique unsigned integer between 0 and n = |V| (excluded).
using flow_t = std::uint32_t;


struct CapacityArc
{
    node_t tail, head;
    flow_t capacity;
};


/* Represents a flow network as a set of arcs with capacity. It is the input data structure fed to
the maximum flow algorithms. */
struct FlowNetwork
{
    size_t n, m; // number of vertices (resp. arcs)
    node_t source, sink;
    std::vector<CapacityArc> arcs;
};


/* This structure is associated to a FlowNetwork, it stores the flow value for each arc and the glo
-bal flow value. It is the ouput data structure of the maximum flow algorithms. */
struct Flow
{
    flow_t value;
    std::vector<flow_t> flow_arcs;
};


/* Represents an arc in a residual network. The residual capacity conceptually represents the amount
of flow that can be pushed on this arc. A pointer to the arc in the oppposite direcion is also sto-
red for quick flow modifications. */
struct ResidualArc
{
    node_t head;
    flow_t residual_capacity; 
    ResidualArc* twin;

    inline node_t tail() const { return twin->head; }
    inline bool is_saturated() const { return residual_capacity == 0; }
    inline bool is_residual()  const { return !is_saturated(); }
    inline void push_flow(flow_t flow) {
        residual_capacity -= flow;
        twin->residual_capacity += flow;
    }
};


/* Represents a residual network with contiguous memory adjacency lists (glued together in a vector,
and accessed individually with a std::span). Neighbors queries should be fast. */
struct ResidualNetwork
{
    size_t n, m; // number of vertices (resp. arcs)
    node_t source, sink;
    std::vector<ResidualArc> adjlist; // "glued" adjacency lists
    std::vector<std::span<ResidualArc>> arcs_out_span;

    auto nodes() const { return std::ranges::iota_view{node_t{0}, n}; }
    auto arcs_out(node_t node) const { return arcs_out_span[node]; }
    auto degree_out(node_t node) const { return std::size(arcs_out_span[node]); }

    ResidualNetwork(FlowNetwork const& network) : n(network.n), m(2 * network.m),
        source(network.source), sink(network.sink), adjlist(m), arcs_out_span(n)
    {
        // compute the out degree for all nodes
        std::vector<node_t> degree_out(n, 0);
        for (auto const& [u, v, capacity] : network.arcs) { ++degree_out[u]; ++degree_out[v]; }
        
        // set up first iterator in the glued adjacency lists for all nodes
        std::vector<decltype(adjlist)::iterator> adjlist_iter(n + 1);
        adjlist_iter[0] = begin(adjlist);
        for (auto u : nodes()) {
            adjlist_iter[u + 1] = adjlist_iter[u] + degree_out[u];
            arcs_out_span[u] = std::span(adjlist_iter[u], adjlist_iter[u + 1]);
        }

        // finally fill the adjacency lists
        for (auto const& [u, v, capacity] : network.arcs) {
            *adjlist_iter[u] = {v, capacity, &*adjlist_iter[v]};
            *adjlist_iter[v] = {u, 0, &*adjlist_iter[u]};
            ++adjlist_iter[u]; ++adjlist_iter[v];
        }
    }

    auto print() const {
        std::printf("Residual Network G = (V, A) - |V| = %zu, |A| = %zu\n", n, m);
        for (auto u : nodes()) {
            std::printf("  [%u] : ", u);
            for (auto const& [v, rc, d] : arcs_out(u))
                std::printf("%u (%u) \t", v, rc);
            std::puts("");
        }
    }
};


/* Retrieves the flow value of each arc in the network given an associated residual network in O(m),
since the original arc can't be differentiated from its reverse arc in the residual network. */
auto get_flow_arcs(FlowNetwork const& network, ResidualNetwork const& rnetwork) {
    std::vector<flow_t> flow_arcs;
    flow_arcs.reserve(network.m);
    std::vector<std::span<ResidualArc>::iterator> current_arc(network.n);
    for (auto u : rnetwork.nodes()) current_arc[u] = begin(rnetwork.arcs_out(u));
    for (auto const& [u, v, capacity] : network.arcs) {
        flow_arcs.push_back(current_arc[u]->twin->residual_capacity);
        ++current_arc[u]; ++current_arc[v];
    }
    return flow_arcs;
} 


/* Edmonds-Karp algorithm for computing the maximum flow of the given flow network in O(nm²). */
Flow edmonds_karp(FlowNetwork const& network)
{
    ResidualNetwork rnetwork(network);
    
    std::vector<node_t> bfs_ordering(rnetwork.n); // a "queue-less" BFS is implemented
    bfs_ordering[0] = rnetwork.source;
    std::vector<ResidualArc*> pred(rnetwork.n); // stores an s-t augmenting path

    /* Performs a Breath-First Search on the residual network starting from the source and stop as
    soon as the sink is reached. Return true if the sink is reachable, false otherwise. Since it'll
    be called multiple times throughout the algorithm, we avoid unnecessary allocations by keeping
    bfs_ordering and pred in memory. */
    auto bfs_find_augmenting_path = [&]() {
        std::ranges::fill(pred, nullptr);
        auto last = begin(bfs_ordering) + 1;
        for (auto u = begin(bfs_ordering); u != last; ++u)
            for (auto& arc : rnetwork.arcs_out(*u))
                if (!pred[arc.head] and arc.is_residual() and arc.head != rnetwork.source) {
                    pred[arc.head] = &arc;
                    if (arc.head == rnetwork.sink) return true; 
                    *(last++) = arc.head;
                }
        return false;
    };

    flow_t maxflow = 0;
    /* Ford-Fulkerson method: While an augmenting s-t path exists in the residual network (Edmonds-
    Karp algorithm chooses the shortest one)... */
    while (bfs_find_augmenting_path())
    {
        // find the bottleneck residual capacity along that path
        flow_t min_residual_capacity = pred[rnetwork.sink]->residual_capacity;
        for (auto arc = pred[rnetwork.sink]; arc != nullptr; arc = pred[arc->tail()])
            min_residual_capacity = std::min(min_residual_capacity, arc->residual_capacity);
        
        // update the arcs along that path by that amount of flow
        for (auto arc = pred[rnetwork.sink]; arc != nullptr; arc = pred[arc->tail()])
            arc->push_flow(min_residual_capacity);

        maxflow += min_residual_capacity;
    }

    return {maxflow, get_flow_arcs(network, rnetwork)};
}


/* Dinitz algorithm for computing the maximum flow of the given flow network in O(n²m) implemented
as recommended by Boris V. Cherkassky. Cherkassky's implementation shares actually many features of
the push-relabel algorithm of Goldberg and Tarjan. */
struct DinitzCherkassky
{
    FlowNetwork const& network;
    ResidualNetwork rnetwork;
    std::vector<std::span<ResidualArc>::iterator> current_arc; // keeps track of visited arcs in the DFS phase loop
    std::vector<node_t> rank; // rank(v) is the distance of node v to the sink
    std::vector<node_t> bfs_ordering; // used for the "queue-less" BFS

    static constexpr auto Unreached = std::numeric_limits<node_t>::max(); // a symbolic +∞ rank value
    static constexpr auto InfiniteFlow = std::numeric_limits<flow_t>::max(); // a symbolic +∞ flow value

    DinitzCherkassky(FlowNetwork const& network) : network(network), rnetwork(network),
        current_arc(network.n), rank(network.n), bfs_ordering(network.n) {
        bfs_ordering[0] = rnetwork.sink;
    }

    /* Algorithm's main loop. */
    Flow operator()() {
        flow_t maxflow = 0;
        while (bfs_compute_rank()) {
            reset_current_arc();
            for (flow_t df = InfiniteFlow; df; maxflow += df)
                df = dfs_phase_loop(rnetwork.source, InfiniteFlow);
        }
        return {maxflow, get_flow_arcs(network, rnetwork)};
    }

    void reset_current_arc() {
        for (auto u : rnetwork.nodes())
            current_arc[u] = begin(rnetwork.arcs_out(u));
    }

    /* The phase is conducted by a single DFS from source. Any satured arc or arc not going from a
    node of rank i to a node of rank i + 1 is skipped (instead of using the lists of edges in the
    layered network). No edge removal is needed. If DFS backtracks on some edge, that edge will not
    participate in the remaining part of DFS (thanks to current_arc). */
    flow_t dfs_phase_loop(node_t u, flow_t flow) {
        if (flow == 0 or u == rnetwork.sink) return flow;
        for (; current_arc[u] != end(rnetwork.arcs_out(u)); ++current_arc[u]) {
            ResidualArc& arc = *current_arc[u];
            if (rank[u] == rank[arc.head] + 1 and arc.is_residual()) {
                if (auto df = dfs_phase_loop(arc.head, std::min(flow, arc.residual_capacity)); df > 0) {
                    arc.push_flow(df);
                    return df;
                }
            }
        }
        return 0;
    }

    /* No layered network should be built. It is sufficient to compute the layer number ("rank")
    dist(v, sink) for every node. This is done by a single run of a BFS from the sink on the unsatu
    -rated arcs, in the reverse arc direction. */
    bool bfs_compute_rank() {
        std::ranges::fill(rank, Unreached);
        rank[rnetwork.sink] = 0;
        auto last = begin(bfs_ordering) + 1;
        for (auto u = begin(bfs_ordering); u != last; ++u)
            for (auto& arc : rnetwork.arcs_out(*u))
                if (rank[arc.head] == Unreached and arc.twin->is_residual()) {
                    rank[arc.head] = rank[*u] + 1;
                    *(last++) = arc.head;
                }
        return rank[rnetwork.source] != Unreached;
    }
};