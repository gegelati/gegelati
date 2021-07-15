#include "TwoLeaves.h"
#include <stdio.h>
#include <stdlib.h>
double* in1;

int main()
{
    in1 = (double*)(malloc(4 * sizeof(double)));
    in1[0] = 0;
    in1[1] = 1.5;
    in1[2] = 2.4;
    in1[3] = 2.4;

    int action = executeFromVertex(root);
    printf("action : %d\n", action);
    reset();
    action = executeFromVertex(root);
    printf("action : %d\n", action);
    return action;
}
