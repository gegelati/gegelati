#include "tpg/keyed/tpgEdgeKeyed.h"

uint64_t TPG::TPGEdgeKeyed::getLock() const
{
    return lock;
}

void TPG::TPGEdgeKeyed::setLock(uint64_t newLock)
{
    this->lock = newLock;
}
