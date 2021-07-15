#include "OneTeamOneLeaf.h"
#include <stdio.h>
#include <stdlib.h>
double* in1;

int main()
{
    in1 = (double*)(malloc(8 * sizeof(double)));
    in1[0] = 4.5;
    in1[1] = 6.8;
    in1[2] = 9.4;
    in1[3] = 7.3;
    in1[4] = 5.25;
    in1[5] = 3.2;
    in1[6] = 2.7;
    in1[7] = 6;

    int action = executeFromVertex(root);
    printf("action : %d\n", action);
    reset();
    action = executeFromVertex(root);
    printf("action : %d\n", action);
    return action;
}
