#ifndef EVOLUTION_POPULATION_H
#define EVOLUTION_POPULATION_H

#include <memory>
#include <string>

#include "evolution/evolutionState.h"
#include "representations/representation.h"

namespace Evolution {

    /**
     * \brief Description of a population evolved by a Module (see Module::addPopulation).
     *
     * The population is a handle: its individuals and its data (metrics, fitness...) are entries of the
     * EvolutionState, under names after the population. Steps receive the population they work on.
     *
     * The population keeps its representation alive, since its individuals only reference it.
     */
    class Population {
        protected:
            /// Name of the population, unique in its module.
            std::string populationName;

            /// Representation of the individuals.
            std::shared_ptr<const Representations::Representation> representation;

            /// Number of individuals of the population.
            size_t size;

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] representation representation of the individuals.
             * \param[in] size number of individuals of the population.
             * \param[in] name name of the population.
             * \throw std::runtime_error if the representation is null or the name is empty.
             */
            Population(std::shared_ptr<const Representations::Representation> representation, size_t size, std::string name);

            /// Return the name of the population.
            const std::string& name() const { return this->populationName; }

            /// Return the representation of the individuals.
            const std::shared_ptr<const Representations::Representation>& getRepresentation() const { return this->representation; }

            /// Return the number of individuals of the population.
            size_t getSize() const { return this->size; }

            /// Return the name of the individuals of the population (a list of individuals), named after the population.
            std::string individuals() const;

            /**
             * \brief Return the name of a data of the population, named "population.name".
             *
             * \param[in] name name of the data (e.g. "trainingFitness").
             */
            std::string data(const std::string& name) const;
    };

}; // namespace Evolution

#endif // EVOLUTION_POPULATION_H
