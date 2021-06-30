#include "TicTacToeBest_TPG.h"
#include <stdio.h>
double* in1;

int main(){
    in1 = (double*)(malloc(9* sizeof(double)));
    in1[0] = -1;
    in1[1] = -1;
    in1[2] = -1;
    in1[3] = -1;
    in1[4] = -1;
    in1[5] = -1;
    in1[6] = -1;
    in1[7] = -1;
    in1[8] = -1;
    int action = executeFromVertex(root);
    printf("action : %d\n", action);
    reset();
    int action2 = executeFromVertex(root);
    printf("action : %d\n", action2);
    if(action2 != action){
        return -1;
    }
    free(in1);
    return action;
}

