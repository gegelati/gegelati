#include "OneLeaf.h"
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

    if (argc == 1) {
        expectedVal = -1;
        in1[0] = 4.5;
        in1[1] = 2.8;
        in1[2] = 3.4;
        in1[3] = 1.3;
        in1[4] = 2.25;
        in1[5] = 3.2;
    }
    else {
        expectedVal = atoi(argv[1]);
        for (int i = 2, cpt = 0; i < argc && cpt < 7; ++cpt, ++i) {
            tab[cpt] = atof(argv[i]);
        }
    }
    action = executeFromVertex(root);
    printf("action : %d\n", action);
    if (expectedVal != -1 && action != expectedVal) {
        return ERROR_INFERENCE;
    }

    reset();
    action = executeFromVertex(root);
    printf("action : %d\n", action);
    if (expectedVal != -1 && action != expectedVal) {
        return ERROR_RESET;
    }

    return 0;
}
