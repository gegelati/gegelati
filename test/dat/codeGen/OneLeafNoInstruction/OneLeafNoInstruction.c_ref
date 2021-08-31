#include "OneLeafNoInstruction.h"
#include "OneLeafNoInstruction_program.h"
#include <limits.h> 
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#define stackSize  8

Edge* callStack[stackSize];
uint32_t top = 0;

int inferenceTPG(){
	reset();
	return executeFromVertex(root);
}

int executeFromVertex(void*(*ptr_f)(int*action)){
	void*(*f)(int*action) = ptr_f;
	int action = INT_MIN;
	while (f!=NULL){
		f= (void*(*)(int*)) (f(&action));
	}
	return action;
}

void* executeTeam(Edge* e, int nbEdge){
	int idxNext = execute(e, nbEdge); 
	if(idxNext != -1) {
		e[idxNext].visited = 1;
		push(&e[idxNext]);
		return e[idxNext].ptr_vertex;
	}
	return NULL;
}

int execute(Edge* e, int nbEdge){
	double bestResult = e[0].ptr_prog();
	int idxNext = 0;
	int idx;
	double r;
	while (e[idxNext].visited == 1){
		idxNext++;
		if(idxNext>= nbEdge){
			printf("Error all the edges of the team are already visited\n");
			return -1;
		}
		bestResult = e[idxNext].ptr_prog();
	}
	idx = idxNext+1;
	//check if there exist another none visited edge with a better result
	while(idx < nbEdge){
		r = e[idx].ptr_prog();
		if(e[idx].visited == 0 &&  r >= bestResult){
			bestResult =r;
			idxNext = idx;
		}
		idx++;
	}
	return idxNext;
}

void push( Edge* e){
	if(top == stackSize) {
		fprintf(stderr, "Call stack of size %d is too small for the iteration of this TPG", stackSize);
	}
	callStack[top] = e;
	top++;}

Edge* pop(){
	Edge* edge = NULL;
if(top > 0){
		top--;
		edge = callStack[top];
	}
	return edge;
}

void reset(){
	while (top > 0) {
		pop()->visited = 0;
	}
}

void* A1(int* action){
	*action = 1;
	return NULL;
}

void* T0(int* action){
	static Edge e[] = {
			{0,P0,A1}
	};
	int nbEdge = 1;
	return executeTeam(e,nbEdge);
}

void* (*root)(int* action) = T0;
