/**
 * @file vuv.cpp
 * @brief Binary tree node level calculator
 * @details VUT FIT - PRL projekt 2
 * @author Timotej Halenár - xhalen00
 * @date 5.5.2024
 */

#include <stdio.h>
#include <iostream>
#include <iterator>
#include <vector>
#include "mpi.h"
#include <map>
#include <unordered_map>

using namespace std;

/**
 * @brief Edge structure
 * @param from - parent node
 * @param to - child node
 * @param forward - true if the edge is going down the tree, false if going up
 * @param weight - weight of the edge, used for calculating the level of the node
 */
struct edge
{
    char from;
    char to;
    bool forward;
    int weight;

    bool operator==(const edge &other) const
    {
        return from == other.from &&
               to == other.to &&
               forward == other.forward;
    }
};

/**
 * @brief List element structure
 * @param e - edge
 * @param next - pointer to the next element in the list
 * This structure is used to create an adjacency list for the tree
 */
struct list_el
{
    edge e;
    list_el *next;
};

/**
 * @brief NodeLevel structure
 * @param node - node character
 * @param level - level of the node
 * This structure is used to store the level of each node in the tree, and it's used for the final output
 */
struct NodeLevel
{
    char node;
    int level;
};

/**
 * @brief Print edge
 * @param e - edge to be printed
 * This function is used for debugging
 */
void print_edge(edge e)
{
    cout << "|" << e.from << "," << e.to << e.forward << "| ";
}

/**
 * @brief Find the next edge in the adjacency list
 * @param adj - adjacency list
 * @param e - edge to find
 * @return pointer to the next edge in the list
 * This function is used to find the next edge in the adjacency list
 */
list_el *find_next(map<char, list_el *> adj, edge e)
{
    for (auto i : adj)
    {
        list_el *el = i.second;

        for (list_el *p = el; p != nullptr; p = p->next)
        {
            if (p->e == e)
            {
                return p->next;
            }
        }
    }

    return nullptr;
}

/**
 * @brief Create adjacency list from tree
 * @param tree - string representation of the tree
 * @param edges - vector of all edges (output param)
 * @param next_erev - vector of next edges of reverse edges (output param)
 * @return adjacency list
 * This function creates an adjacency list from the tree string representation
 */
map<char, list_el *> create_adj_list(const string &tree, vector<list_el *> *edges, vector<list_el *> *next_erev)
{
    // Create adjacency list
    map<char, list_el *> adj;

    for (int i = 0; i < tree.size(); ++i)
    {
        // Initialize the adjacency list for each node
        char parent = tree[i];
        int left = 2 * i + 1;
        int right = 2 * i + 2;

        // If left or right child is out of bounds, skip
        // I know that this is really ugly and there's probably a better way, but I don't have it in me to rework it :(
        if (left < tree.size())
        {
            char child = tree[left];

            edge fw_edge = {parent, child, true};

            edge rev_edge = {child, parent, false};

            list_el *el1 = new list_el{fw_edge, adj[parent]};
            adj[parent] = el1;
            edges->push_back(el1);
            next_erev->push_back(find_next(adj, {fw_edge.to, fw_edge.from, !fw_edge.forward}));
            list_el *el2 = new list_el{rev_edge, adj[child]};
            adj[child] = el2;
            edges->push_back(el2);
            next_erev->push_back(find_next(adj, {rev_edge.to, rev_edge.from, !rev_edge.forward}));
        }

        if (right < tree.size())
        {
            char child = tree[right];

            edge fw_edge = {parent, child, true};

            edge rev_edge = {child, parent, false};

            list_el *el1 = new list_el{fw_edge, adj[parent]};
            adj[parent] = el1;
            edges->push_back(el1);
            next_erev->push_back(find_next(adj, {fw_edge.to, fw_edge.from, !fw_edge.forward}));
            list_el *el2 = new list_el{rev_edge, adj[child]};
            adj[child] = el2;
            edges->push_back(el2);
            next_erev->push_back(find_next(adj, {rev_edge.to, rev_edge.from, !rev_edge.forward}));
        }
    }

    return adj;
}

/**
 * @brief Calculate the suffix sum of the euler tour
 * @param etour - euler tour
 * @param e - edge to find the suffix sum for
 * @param size - size of the euler tour
 * @return suffix sum of the euler tour
 * This function is used to calculate the suffix sum of the euler tour
 */
int suffix_sum_etour(vector<edge> etour, edge e)
{
    int sum = 0;

    for (int i = etour.size() - 1; i >= 0; i--)
    {
        if (etour[i].forward)
            sum -= 1;
        else
            sum += 1;

        if (etour[i] == e)
            break;
    }
    return sum;
}

/**
 * @brief Get the index of the edge in the list
 * @param edges - list of edges
 * @param e - edge to find
 * @param size - size of the list
 * @return index of the edge in the list
 * This function is used to get the index of the edge in the list
 */
int get_edge_ind(vector<list_el *> edges, edge e, int size)
{
    for (int i = 0; i < size; i++)
    {
        if (edges[i]->e == e)
        {
            return i;
        }
    }

    return -1;
}

/**
 * @brief Main function
 * @param argc - number of arguments
 * @param argv - arguments
 * This function is used to initialize MPI and call the other functions
 */
int main(int argc, char *argv[])
{
    // Initialize MPI
    MPI_Init(&argc, &argv);

    // *************************************************************
    // Create custom MPI data type for edge structure
    // Source: https://kindatechnical.com/parallel-computing-mpi/custom-mpi-data-types.html
    // *************************************************************
    MPI_Datatype MPI_EDGE;
    int lengths[4] = {1, 1, 1, 1};
    MPI_Aint offsets[4];
    MPI_Datatype types[4] = {MPI_CHAR, MPI_CHAR, MPI_CXX_BOOL, MPI_INT};

    offsets[0] = offsetof(edge, from);
    offsets[1] = offsetof(edge, to);
    offsets[2] = offsetof(edge, forward);
    offsets[3] = offsetof(edge, weight);

    MPI_Type_create_struct(4, lengths, offsets, types, &MPI_EDGE);
    MPI_Type_commit(&MPI_EDGE);
    // *************************************************************

    // Get rank, size
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    string tree = argv[1];

    // Create adjacency list
    // Create list of edges
    // Create list of next edges of reverse edges
    vector<list_el *> edges;
    vector<list_el *> next_erev;
    auto adj = create_adj_list(tree, &edges, &next_erev);

    // Assign the edges to the processes
    list_el *my_el = edges[rank];

    // Assign initial weight to the edge
    // If the edge is going down the tree, assign -1, else assign 1
    if (my_el->e.forward)
        my_el->e.weight = -1;
    else
        my_el->e.weight = 1;

    // Create list of next edges in euler tour
    // This will be used to reconstruct the euler tour
    vector<edge> next_edges(size);

    // Get the next edge in each process (euler tour algorithm)
    edge next_edge;
    if (next_erev[rank] != nullptr)
    {
        // If exists, get the next edge from the reverse edge
        next_edge = next_erev[rank]->e;
    }
    else
    {
        // Otherwise, get first edge from the adjacency list of node to which the edge is going
        next_edge = adj[edges[rank]->e.to][0].e;
    }

    // Gather the next edges in each process
    MPI_Gather(&next_edge, 1, MPI_EDGE, next_edges.data(), 1, MPI_EDGE, 0, MPI_COMM_WORLD);

    // Create euler tour vector
    vector<edge> etour(size);

    // Reconstruct euler tour in single process, this will be broadcasted later
    if (rank == 0)
    {
        edge cur = edges[0]->e;

        // Helper variable for indexing next_edges
        int ind = 0;

        // This thing is probably needlessly complicated, but it works
        for (int i = 0; i < size; i++)
        {
            // add current edge as next member of etour
            etour[i] = cur;
            // take the next edge from next_edges
            cur = next_edges[ind];
            // calculate index for next iteration
            ind = get_edge_ind(edges, cur, size);
        }
    }

    // Broadcast the euler tour to all processes
    MPI_Bcast(etour.data(), size, MPI_EDGE, 0, MPI_COMM_WORLD);

    // Calculate the suffix sum of the euler tour for each edge
    int weight = suffix_sum_etour(etour, my_el->e);

    // Create a struct to store the node and its level
    NodeLevel my_node_level;

    // Create a list of all node levels
    vector<NodeLevel> all_node_levels(size);

    // Only assign weight to the node if the edge is forward
    if (my_el->e.forward)
    {
        my_node_level = {my_el->e.to, weight + 1};
    }

    // Gather all the node levels
    MPI_Gather(&my_node_level, sizeof(NodeLevel), MPI_BYTE,
               all_node_levels.data(), sizeof(NodeLevel), MPI_BYTE,
               0, MPI_COMM_WORLD);

    // Print the node levels in single process
    if (rank == 0)
    {
        // Create a map to store the node levels in order to print them in order of the tree
        map<char, int> node_levels;
        for (int i = 0; i < size; i++)
        {
            node_levels[all_node_levels[i].node] = all_node_levels[i].level;
        }

        // Print in the specified format
        for (int i = 0; i < tree.size(); ++i)
        {
            cout << tree[i] << ":" << node_levels[tree[i]];
            // Do not print comma if on last element
            if (i != tree.size() - 1)
            {
                cout << ",";
            }
        }
        cout << endl;
    }

    // Free memory from adjacency list
    // It's still leaking somewhere though
    for (auto i : adj)
    {
        list_el *el = i.second;

        while (el != nullptr)
        {
            list_el *next = el->next;
            delete el;
            el = next;
        }
    }

    // Free the custom MPI data type
    MPI_Type_free(&MPI_EDGE);
    MPI_Finalize();
}
