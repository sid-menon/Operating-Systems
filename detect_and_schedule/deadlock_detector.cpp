//resources used included at the bottom

#include "deadlock_detector.h"
#include "common.h"

#include <iostream>
#include <vector>
#include <string>

class Graph {
public:
    std::vector<std::vector<int>> adj_list;
    std::vector<int> out_counts;
} graph;

int counter = 0;
std::unordered_map<int, std::string> in1;
std::unordered_map<std::string, int> in2;

/// this is the function you need to (re)implement
///
/// parameter edges[] contains a list of req- and assment- edges
///   example of a req edge, process "p1" resource "r1"
///     "p1 -> r1"
///   example of an assment edge, process "XYz" resource "XYz"
///     "XYz <- XYz"
///
/// You need to process edges[] one edge at a time, and run a deadlock
/// detection after each edge. As soon as you detect a deadlock, your function
/// needs to stop processing edges and return an instance of Result structure
/// with edge_index set to the index that caused the deadlock, and dl_procs set
/// to contain with names of processes that are in the deadlock.
///
/// To indicate no deadlock was detected after processing all edges, you must
/// return Result with edge_index = -1 and empty dl_procs[].
///

int get(const std::string& w) {
    auto f = in2.find(w);

    if (f == in2.end()) {
        in1[counter] = w;
        in2[w] = counter;
        counter++;
        return counter - 1;
    } else {
        return in2[w];
    }
}

std::string get(int t) {
    if (t < 0 || t > int(in1.size())) {
        return "";
    }
    return in1[t];
}

size_t sizer(int counter) {
    return size_t(counter);
}

Result detect_deadlock(const std::vector<std::string>& edges)
{

    Result result;
    const std::string req = "->";
    const std::string ass = "<-";


    // Handle edges
    for (int t = 0; t < int(edges.size()); t++) {
        std::string edge = edges[t];
        std::vector<std::string> tokens = split(edge);

        // [0] are processes
        // [1] are reqs
        int t1 = get("{process}" + tokens[0]); 
        int t2 = get("{req}" + tokens[2]); 
        int from, to;
        std::string sign = tokens[1];

        if (sign == req) {
            from = t1;
            to = t2;
        } else if (sign == ass) {
            from = t2;
            to = t1;
        } else {
            continue;
        }

        // resize to fit right nodes
        size_t size = sizer(counter);
        graph.out_counts.resize(size);
        graph.adj_list.resize(size);


        graph.adj_list[to].push_back(from);
        graph.out_counts[from] ++;

        std::vector<int> out_counts = graph.out_counts;
        std::vector<int> temp_zero;
        for (int t = 0; t < int(graph.out_counts.size()); t++) {
            if (graph.out_counts[t] == 0) {
                temp_zero.push_back(t);
            }
        }

        while (!temp_zero.empty()) { //getting rid of nodes with no out degrees
            int node = temp_zero.back();
            temp_zero.pop_back();
            for (int adj : graph.adj_list[node]) {
                out_counts[adj]--;

                if (out_counts[adj] == 0) {
                    temp_zero.push_back(adj);
                }
            }
        }

        for (int t = 0; t < int(out_counts.size()); t++) { //all the dl_procs
            if (out_counts[t] != 0) {
                std::string node = get(t);
                if (node.find("{process}") != std::string::npos) {
                    node.replace(0, 9, ""); //get rid of {process}
                    result.dl_procs.push_back(node);
                }
            }
        }

        // If there's dl_procs
        if (!result.dl_procs.empty()) {
            result.edge_index = t;
            break;
        }
    }

    // If no dl_procs
    if (result.dl_procs.empty()) {
        result.edge_index = -1; //set edge index as -1
    }


    return result;
}
