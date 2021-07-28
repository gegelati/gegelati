//
// Created by tbourgoi on 28/07/2021.
//

#ifndef INFERENCECSV_H
#define INFERENCECSV_H

#include <stdio.h>
#include <stdlib.h>

#include "csvparser.h"

#define ERROR_INFERENCE 1
#define ERROR_RESET 2

void errorPrint(int action, int expect, CsvRow* row, double* tab);

int inferenceCSV(char* filename,
                 int (*executeFromVertex)(void* (*)(int* action)),
                 void* (*root)(int*), void (*reset)());

#endif // INFERENCECSV_H
