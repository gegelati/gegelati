#include "tpg/keyed/tpgEdgeKeyed.h"

uint64_t TPG::TPGEdgeKeyed::getLock() const
{
    return lock;
}

void TPG::TPGEdgeKeyed::setLock(uint64_t newLock)
{
    this->lock = newLock;
}

bool TPG::TPGEdgeKeyed::isUnlockedByKey(uint64_t key) const
{
    // Always unlocked if the lock is 1 (default value)
    if (lock == 1) {
        return true;
    }

    if (key == 1) {
        return false; // A key of 1 cannot unlock any lock greater than 1
    }

    // Check if the lock is a multiple of the key
    return lock % key == 0;
}
