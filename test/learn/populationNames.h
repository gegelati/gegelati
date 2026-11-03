#ifndef POPULATION_NAMES_H
#define POPULATION_NAMES_H

#include <string>

#include "evolution/basicEvolution.h"

/// Names of a population for the tests: the default names of the data, prefixed by the name of the population.
inline Evolution::BasicEvolution::Names namesOf(const std::string& population)
{
    Evolution::BasicEvolution::Names names;
    names.population = population;
    names.parents = population + ".parents";
    names.offspring = population + ".offspring";
    names.trainingMetrics = population + ".trainingMetrics";
    names.validationMetrics = population + ".validationMetrics";
    names.trainingFitness = population + ".trainingFitness";
    names.parentFitness = population + ".parentFitness";
    names.validationFitness = population + ".validationFitness";
    names.evolution = population;
    return names;
}

#endif // POPULATION_NAMES_H
