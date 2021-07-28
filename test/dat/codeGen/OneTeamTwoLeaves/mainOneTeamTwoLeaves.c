#include <stdio.h>
#include <stdlib.h>

#include "OneTeamTwoLeaves.h"
#include "csvparser.h"
#include "inferenceCSV.h"

double* in1;

int main(int argc, char* argv[])
{
    double tab[2];
    in1 = tab;

    if (argc != 2) {
        fprintf(stderr, "error the program only require one parameter : the "
                        "filename of the data.\n");
        return 3;
    }

    return inferenceCSV(argv[1], executeFromVertex, root, reset);
}
