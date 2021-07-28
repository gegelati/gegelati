#include "ThreeTeamsOneCycleThreeLeaves.h"
#include "csvparser.h"
#include "inferenceCSV.h"
#include <stdio.h>
#include <stdlib.h>
#define ERROR_INFERENCE 1
#define ERROR_RESET 2

double* in1;

int main(int argc, char* argv[])
{
    double tab[7];
    in1 = tab;
    int expectedVal;
    int action;

    if (argc != 2) {
        fprintf(stderr, "error the program only require one parameter : the "
                        "filename of the data.\n");
        return 3;
    }

    return inferenceCSV(argv[1], executeFromVertex, root, reset);
}
