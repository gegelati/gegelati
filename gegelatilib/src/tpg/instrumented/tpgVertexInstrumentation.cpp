
#include "tpg/instrumented/tpgVertexInstrumentation.h"

size_t TPG::TPGVertexInstrumentation::getNbVisits() const
{
    return this->nbVisits;
}

void TPG::TPGVertexInstrumentation::incrementNbVisits()
{
    this->nbVisits++;
}

void TPG::TPGVertexInstrumentation::reset()
{
    this->nbVisits = 0;
}