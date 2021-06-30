//#include "3team.h"
#include "StickGameBest_TPG.h"
#include <stdio.h>
int* in1;
int* in2;
int main(){
    int* remainingSticks = (int*)(malloc(sizeof(int)));
    int* hints = (int*)(malloc(4*sizeof(int)));
    remainingSticks[0] = 21;
    hints[0] = 1;
    hints[1] = 2;
    hints[2] = 3;
    hints[3] = 4;

    in1 = hints;
    in2 = remainingSticks;

    int action = executeFromVertex(root);
    printf("action : %d\n", action);
    reset();
    int action2 = executeFromVertex(root);
    printf("action2 : %d\n", action2);
    if(action2 != action){
        return -1;
    }
    free(in1);
    free(in2);
    return action;
}

