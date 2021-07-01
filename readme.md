# Edmonds–Karp and Dinitz–Cherkassky maximum flow algorithms in Modern C++20

Implementation in C++20 of the [Edmonds-Karp's algorithm](https://en.wikipedia.org/wiki/Edmonds%E2%80%93Karp_algorithm) and the [Dinitz's algorithm](https://en.wikipedia.org/wiki/Dinic%27s_algorithm) (following the Boris V. Cherkassky's recommentations) for computing the maximum flow of a given flow network in O(nm²) and O(n²m) respectively. This implementation was made for educational purposes.

## To-Do

* Implementation of a performant push-relabel algorithm.
* A small function to compute the minimum cut with the final residual network.

## How to run it?

This project use CMake. To run the executable you need to pass a flow network instance .max file:

````
$ maxflow ../maxflow_instances/BVZ-tsukuba0.max

Network instance: "../maxflow_instances/BVZ-tsukuba0.max - |V| = 110594, |E| = 514483

Algorithm: "Dinitz-Cherkassky"
Maximum flow value: 34669
Duration: 532ms

Algorithm: "Edmonds-Karp"
Maximum flow value: 34669
Duration: 15312ms
````

## References

1. Dinitz Y. (2006) [_Dinitz’ Algorithm: The Original Version and Even’s Version_](https://www.cs.bgu.ac.il/~dinitz/Papers/Dinitz_alg.pdf). In: Goldreich O., Rosenberg A.L., Selman A.L. (eds) Theoretical Computer Science. Lecture Notes in Computer Science, vol 3895. Springer, Berlin, Heidelberg. https://doi.org/10.1007/11685654_10 
