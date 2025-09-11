

#ifndef COUNTER_RESET_H
#define COUNTER_RESET_H

#include "tpg/tpgEdge.h"
#include "tpg/tpgVertex.h"
#include "program/program.h"

/**
 * \brief Struct to reset static counters in classes.
 */
struct CounterReset
{
    /**
     * \brief Reset all the static counters used in the library.
     */
    static void counterReset();
};


#endif // COUNTER_RESET_H