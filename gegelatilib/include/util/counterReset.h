

#ifndef COUNTER_RESET_H
#define COUNTER_RESET_H

#include "graphBased/gpNode.h"
#include "individual.h"
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