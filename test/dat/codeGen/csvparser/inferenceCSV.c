

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

int inferenceCSV(char* filename,
                 int (*executeFromVertex)(void* (*)(int* action)),
                 void* (*root)(int*), void (*reset)())
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

        action = executeFromVertex(root);
#ifdef DEBUG
        printf("action : %d\n", action);
#endif // DEBUG
        if (action != expectedVal) {
            errorPrint(action, expectedVal, row, tab);
            return ERROR_INFERENCE;
        }

        reset();
        action = executeFromVertex(root);
        if (action != expectedVal) {
            errorPrint(action, expectedVal, row, tab);
            return ERROR_RESET;
        }

        CsvParser_destroy_row(row);
        reset();
    }
    CsvParser_destroy(csvparser);
    return 0;
}