#include "tpg/keyed/tpgTeamKeyed.h"

uint64_t TPG::TPGTeamKeyed::getKey() const
{
    return this->key;
}

void TPG::TPGTeamKeyed::setKey(uint64_t newKey)
{
    this->key = newKey;
}
