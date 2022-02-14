
#include "tpg/instrumented/tpgVertexInstrumentation.h"

uint64_t TPG::TPGVertexInstrumentation::getNbVisits() const
{
    return this->nbVisits;
}

void TPG::TPGVertexInstrumentation::incrementNbVisits() const
{
    this->nbVisits++;
}

void TPG::TPGVertexInstrumentation::reset() const
{
    this->nbVisits = 0;
}