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

#include "inferenceCSV.h"
extern double* in1;

void errorPrint(int action, int expect, CsvRow* row, double* tab)
{
    const char** rowFields = CsvParser_getFields(row);
    printf("action : %d but expect %d for data : ", action, expect);
    for (int i = 1; i < CsvParser_getNumFields(row); i++) {
        printf(" %s(%lf)", rowFields[i], tab[i - 1]);
    }
    printf("\n");
}

int inferenceCSV(char* filename, int (*inferenceTPG)(void))
{
    double* tab = in1;
    int expectedVal;
    int action;
    CsvParser* csvparser = CsvParser_new(filename, " ", 0);
    CsvRow* row;

    while ((row = CsvParser_getRow(csvparser))) {
        const char** rowFields = CsvParser_getFields(row);
        expectedVal = strtol(rowFields[0], NULL, 10);

        for (int i = 1; i < CsvParser_getNumFields(row); i++) {
            tab[i - 1] = strtod(rowFields[i], NULL);
        }

        action = inferenceTPG();
#ifdef DEBUG
        printf("action : %d\n", action);
#endif // DEBUG
        if (action != expectedVal) {
            errorPrint(action, expectedVal, row, tab);
            return ERROR_INFERENCE;
        }

        action = inferenceTPG();
        if (action != expectedVal) {
            errorPrint(action, expectedVal, row, tab);
            return ERROR_RESET;
        }

        CsvParser_destroy_row(row);
    }
    CsvParser_destroy(csvparser);
    return 0;
}
