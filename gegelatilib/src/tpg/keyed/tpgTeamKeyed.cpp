#include "tpg/keyed/tpgTeamKeyed.h"

const std::set<uint64_t>& TPG::TPGTeamKeyed::getKeys() const
{
    return this->keys;
}

void TPG::TPGTeamKeyed::addKey(uint64_t newKey)
{
    this->keys.insert(newKey);
}
