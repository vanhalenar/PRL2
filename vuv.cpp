/*
VUT FIT - PRL projekt 2
Timotej Halenár - xhalen00
29.4.2024
*/

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>
#include "mpi.h"

using namespace std;

int main(int argc, char *argv[])
{
    cout << "hi, this is the input: " << argv[1] << endl;

    MPI_Init(&argc, &argv);

    MPI_Finalize();
}
