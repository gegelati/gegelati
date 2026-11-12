
#include "util/counterReset.h"

void CounterReset::counterReset()
{
    GraphBased::GPNode::resetGPNodeIDCounter();
    Individual::resetIndividualIDCounter();
}