#include "tpg/instrumented/tpgEdgeInstrumented.h"

uint64_t TPG::TPGEdgeInstrumented::getNbVisits() const
{
    return this->nbVisits;
}

void TPG::TPGEdgeInstrumented::incrementNbVisits() const
{
    this->nbVisits++;
}

uint64_t TPG::TPGEdgeInstrumented::getNbTraversed() const
{
    return this->nbTraversed;
}

void TPG::TPGEdgeInstrumented::incrementNbTraversed() const
{
    this->nbTraversed++;
}

void TPG::TPGEdgeInstrumented::reset() const
{
    this->nbTraversed = 0;
    this->nbVisits = 0;
}
