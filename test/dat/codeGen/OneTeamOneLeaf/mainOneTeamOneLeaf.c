/// doc in ../README.md
#include <stdio.h>
#include <stdlib.h>

#include "OneTeamOneLeaf.h"

#define ERROR_INFERENCE 1
#define ERROR_RESET 2
double* in1;

int main(int argc, char* argv[])
{
    double tab[2];
    in1 = tab;
    int expectedVal;
    int action;

    if (argc == 1) {
        expectedVal = -1;
        in1[0] = 4.5;
        in1[1] = 6.8;
    }
    else {
        expectedVal = atoi(argv[1]);
        for (int i = 2, cpt = 0; i < argc && cpt < 2; ++cpt, ++i) {
            tab[cpt] = atof(argv[i]);
        }
    }

    action = executeFromVertex(root);
#ifdef DEBUG
    printf("action : %d\n", action);
#endif // DEBUG
    if (expectedVal != -1 && action != expectedVal) {
        return ERROR_INFERENCE;
    }

    reset();
    action = executeFromVertex(root);
    if (expectedVal != -1 && action != expectedVal) {
        return ERROR_RESET;
    }

    return 0;
}
