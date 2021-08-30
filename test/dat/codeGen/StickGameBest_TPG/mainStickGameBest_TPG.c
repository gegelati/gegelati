///doc in ../README.md
#include <stdio.h>

#include "StickGameBest_TPG.h"

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
        //        expectedVal = atoi(argv[1]);
        int cpt = 0, i = 1;
        for (; i < argc && cpt < 3; ++cpt, ++i) {
            hints[cpt] = atoi(argv[i]);
        }
        remainingSticks[0] = atoi(argv[i]);
    }

    action = executeFromVertex(root);
    return action;
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
