/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2021 - 2022) :
 *
 * Mickaël Dardaillon <mdardail@insa-rennes.fr> (2022)
 * Thomas Bourgoin <tbourgoi@insa-rennes.fr> (2021)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

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

int inferenceCSV(char* filename, void (*inferenceTPG)(int* action));

#endif // INFERENCE_CSV_H
