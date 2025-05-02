/*
VUT FIT - PRL projekt 2
Timotej Halenár - xhalen00
29.4.2024
*/

#include <stdio.h>
#include <iostream>
#include <iterator>
#include <vector>
#include "mpi.h"
#include <map>

using namespace std;

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

struct list_el
{
    edge e;
    edge rev;
    list_el *next;
};

struct NodeLevel
{
    char node;
    int level;
};

void print_edge(edge e)
{
    cout << "|" << e.from << "," << e.to << e.forward << "| ";
}

map<char, list_el *> create_adj_list(const string &tree)
{
    map<char, list_el *> adj;

    for (int i = 0; i < tree.size(); ++i)
    {
        char parent = tree[i];
        int left = 2 * i + 1;
        int right = 2 * i + 2;

        if (left < tree.size())
        {
            char child = tree[left];

            edge forward_edge = {parent, child, true};
            edge reverse_edge = {child, parent, false};

            list_el *el1 = new list_el{forward_edge, reverse_edge, adj[parent]};
            adj[parent] = el1;

            list_el *el2 = new list_el{reverse_edge, forward_edge, adj[child]};
            adj[child] = el2;
        }

        if (right < tree.size())
        {
            char child = tree[right];

            edge forward_edge = {parent, child, true};
            edge reverse_edge = {child, parent, false};

            list_el *el1 = new list_el{forward_edge, reverse_edge, adj[parent]};
            adj[parent] = el1;

            list_el *el2 = new list_el{reverse_edge, forward_edge, adj[child]};
            adj[child] = el2;
        }
    }

    return adj;
}

list_el *find_next(map<char, list_el *> adj, edge e)
{
    for (auto &[v, head] : adj)
    {
        for (list_el *p = head; p != nullptr; p = p->next)
        {
            if (p->e == e)
                return p->next;
        }
    }

    return nullptr;
}

int suffix_sum_etour(edge *etour, edge e, int size)
{
    bool found = false;

    int sum = 0;

    for (int i = 0; i < size; i++)
    {
        if (found)
        {
            if (etour[i].forward)
                sum -= 1;
            else
                sum += 1;
        }
        else
        {
            if (etour[i] == e)
                found = true;
        }
    }
    return sum;
}

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
int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    MPI_Datatype MPI_EDGE;
    int lengths[4] = {1, 1, 1, 1};
    MPI_Aint displacements[4];
    MPI_Datatype types[4] = {MPI_CHAR, MPI_CHAR, MPI_CXX_BOOL, MPI_INT};

    edge dummy;
    MPI_Aint base;
    MPI_Get_address(&dummy, &base);
    MPI_Get_address(&dummy.from, &displacements[0]);
    MPI_Get_address(&dummy.to, &displacements[1]);
    MPI_Get_address(&dummy.forward, &displacements[2]);
    MPI_Get_address(&dummy.weight, &displacements[3]);

    for (int i = 0; i < 4; ++i)
        displacements[i] -= base;

    MPI_Type_create_struct(4, lengths, displacements, types, &MPI_EDGE);
    MPI_Type_commit(&MPI_EDGE);

    int rank, size;
    MPI_Status status;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    auto adj = create_adj_list(argv[1]);
    vector<list_el *> edges;
    vector<list_el *> next_erev;
    for (auto &[v, head] : adj)
    {
        for (list_el *p = head; p != nullptr; p = p->next)
        {
            edges.push_back(p);
            next_erev.push_back(find_next(adj, {p->e.to, p->e.from, !p->e.forward}));
        }
    }

    list_el *my_el = nullptr;
    my_el = edges[rank];
    if (my_el->e.forward)
        my_el->e.weight = -1;
    else
        my_el->e.weight = 1;

    edge *etour = nullptr;

    if (rank == 0)
    {
        etour = new edge[size];
    }

    edge my_etour;
    if (next_erev[rank] != nullptr)
    {
        my_etour = next_erev[rank]->e;
    }
    else
    {
        my_etour = adj[edges[rank]->e.to][0].e;
    }

    MPI_Gather(&my_etour, 1, MPI_EDGE, etour, 1, MPI_EDGE, 0, MPI_COMM_WORLD);
    // print numbers from one process

    edge *proper_etour;
    proper_etour = new edge[size];
    if (rank == 0)
    {
        // all threads need this

        edge cur = edges[0]->e;
        int ind = 0;
        for (int i = 0; i < size; i++)
        {
            // print_edge(cur);
            proper_etour[i] = cur;
            cur = etour[ind];
            ind = get_edge_ind(edges, cur, size);
        }

        /* cout << "testing suffix sum" << endl;
        print_edge(edges[3]->e);
        int w = suffix_sum_etour(proper_etour, edges[3]->e, size);
        cout << "sum: " << w << endl; */
        /* for (int i = 0; i < size; i++)
        {

            cout << i << ": ";
            cout << "my edge: ";
            print_edge(edges[i]->e);
            cout << "my etour: ";
            print_edge(etour[i]);
            cout << endl;
            // cout << ": edge (" << etour[i].from << "," << etour[i].to << ")" << endl;
        }
        printf("\n"); */
    }
    MPI_Bcast(proper_etour, size, MPI_EDGE, 0, MPI_COMM_WORLD);

    int w = suffix_sum_etour(proper_etour, my_el->e, size);
    /* cout << "sum " << rank << ": ";
    print_edge(my_el->e);
    cout << w << endl; */

    NodeLevel my_node_level;

    NodeLevel *all_node_levels = nullptr;
    if (rank == 0)
    {
        all_node_levels = new NodeLevel[size];
    }

    if (my_el->e.forward)
    {
        my_node_level = {my_el->e.to, w};
    }

    MPI_Gather(&my_node_level, sizeof(NodeLevel), MPI_BYTE,
               all_node_levels, sizeof(NodeLevel), MPI_BYTE,
               0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        // Use a map to store the levels of each node
        map<char, int> node_levels;
        for (int i = 0; i < size; i++)
        {
            node_levels[all_node_levels[i].node] = all_node_levels[i].level;
        }

        // Print the levels in the order of the input string
        // cout << "Node levels in order:" << endl;
        std::string tree_string = argv[1];
        for (int i = 0; i < tree_string.size(); ++i) // Assuming argv[1] contains the input tree string
        {
            cout << tree_string[i] << ":" << node_levels[tree_string[i]];
            if (i != tree_string.size() - 1) // Check if it's not the last node
            {
                cout << ",";
            }
        }
        cout << endl;

        delete[] all_node_levels; // Free memory
    }

    MPI_Type_free(&MPI_EDGE);
    MPI_Finalize();
}
