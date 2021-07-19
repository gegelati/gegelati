#include "StickGameBest_TPG.h"
#include <stdio.h>
#define ERROR_INFERENCE 1
#define ERROR_RESET 2
int* in1;
int* in2;
int main(int argc, char* argv[])
{
    double tab[1];
    int remainingSticks[1];
    int hints[4];
    in1 = hints;
    in2 = remainingSticks;
    int expectedVal;
    int action;
    if (argc == 1) {
        expectedVal = -1;
        remainingSticks[0] = 21;
        hints[0] = 1;
        hints[1] = 2;
        hints[2] = 3;
        hints[3] = 4;
    }
    else {
        expectedVal = atoi(argv[1]);
        remainingSticks[0] = atoi(argv[2]);
        for (int cpt = 0, i = 3; i < argc && cpt < 4; ++cpt, ++i) {
            hints[cpt] = atoi(argv[i]);
        }
    }

    action = executeFromVertex(root);
    printf("action : %d\n", action);
    if (expectedVal != -1 && action != expectedVal) {
        return ERROR_INFERENCE;
    }

    reset();
    action = executeFromVertex(root);
    printf("action2 : %d\n", action);
    if (expectedVal != -1 && action != expectedVal) {
        return ERROR_RESET;
    }
    return 0;
}
