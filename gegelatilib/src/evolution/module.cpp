
#include "evolution/module.h"

std::string Evolution::Module::summary() const
{
    return "Not implemented";
}

void Evolution::Module::evolve(size_t nbGenerations)
{
    this->initialize();
    this->doGenerations(nbGenerations);
}

void Evolution::Module::doGenerations(size_t nbGenerations)
{
    for(size_t idx = 0; idx < nbGenerations; idx++) {
        this->doGeneration(idx);
    }
}