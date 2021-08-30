#ifndef C_OneLeafNoInstruction_H
#define C_OneLeafNoInstruction_H

#include <stdlib.h>

typedef struct Edge {
	int visited;
	double (*ptr_prog)(void);
	void* (*ptr_vertex)(int* action);
}Edge;

int inferenceTPG();
int executeFromVertex(void*(*)(int*action));
void* executeTeam(Edge* e, int nbEdge);
int execute(Edge* e, int nbEdge);
void push(Edge* e);
Edge* pop();
void reset();

void* A1(int* action);
void* T0(int* action);

extern void* (*root)(int* action);

#endif
