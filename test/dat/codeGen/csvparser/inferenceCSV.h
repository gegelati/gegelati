#ifndef INFERENCE_CSV_H
#define INFERENCE_CSV_H

#include <stdio.h>
#include <stdlib.h>

#include "csvparser.h"

#define ERROR_INFERENCE 1
#define ERROR_RESET 2

/**
 * \brief Function which print an error message when testing a generated TPG.
 *
 * @param action integer returned at the end of the inference of the generated
 * TPG tested.
 * @param expect integer expected at the end of the inference of the generated
 * TPG tested.
 * @param row pointer to the CsvRow to retreive to input value given to execute
 * the TPG.
 * @param tab pointer to the double array given to the TPG, this is to check if
 * there is not cast error between the data read from the CSV and the data given
 * to the TPG.
 */
void errorPrint(int action, int expect, CsvRow* row, double* tab);

/**
 * \brief Function that read the CSV file filename and execute the TPG for each
 * line of the CSV.
 *
 * The first element of the CSV is the expected action return by the TPG. The
 * other value on the line correspond to the input data given to the TPG. For
 * each line two inferences are done to check if the reset function works
 * properly.
 *
 * @param filename char* holding the name of the csv file that contain the
 * exepected action and the input data for each inference.
 * @param executeFromVertex pointer to the function that execute the inference
 * of the generated TPG.
 * @param root pointer to the function that represent the root of the generated
 * TPG.
 * @param reset pointer to the function that reset the flag that indicates that
 * an edge has already been visited.
 * @return return 0 if all inference return the expected action. If an error
 * occurs return 1 if the error is at the first inference prior to the reset of
 * the TPG. If the error occurs at the second inference return 2. In this case
 * it means that the TPG has not been reset correctly
 */

int inferenceCSV(char* filename,
                 int (*executeFromVertex)(void* (*)(int* action)),
                 void* (*root)(int*), void (*reset)());

#endif // INFERENCE_CSV_H
