#include "tpg/instrumented/tpgEdgeInstrumented.h"

uint64_t TPG::TPGEdgeInstrumented::getNbVisits() const
{
    return this->nbVisits;
}

void TPG::TPGEdgeInstrumented::incrementNbVisits() const
{
    this->nbVisits++;
}

uint64_t TPG::TPGEdgeInstrumented::getNbTraversal() const
{
    return this->nbTraversal;
}

void TPG::TPGEdgeInstrumented::incrementNbTraversal() const
{
    this->nbTraversal++;
}

void TPG::TPGEdgeInstrumented::reset() const
{
    this->nbTraversal = 0;
    this->nbVisits = 0;
}
