
#include "tpg/instrumented/tpgGraphInstrumented.h"

TPG::TPGGraphInstrumented& TPG::TPGGraphInstrumented::operator=(TPGGraphInstrumented model)
{
    swap(*this, model);
    return *this;
}