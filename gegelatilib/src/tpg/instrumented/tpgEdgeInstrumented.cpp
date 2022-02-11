#include "tpg/instrumented/tpgEdgeInstrumented.h"

size_t TPG::TPGEdgeInstrumented::getNbVisits() const
{
    return this->nbVisits;
}

void TPG::TPGEdgeInstrumented::incrementNbVisits()
{
    this->nbVisits++;
}

size_t TPG::TPGEdgeInstrumented::getNbTraversed() const
{
    return this->nbTraversed;
}

void TPG::TPGEdgeInstrumented::incrementNbTraversed()
{
    this->nbTraversed++;
}

void TPG::TPGEdgeInstrumented::reset()
{
    this->nbTraversed = 0;
    this->nbVisits = 0;
}
