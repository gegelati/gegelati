#ifndef EVOLUTION_AGENT_H
#define EVOLUTION_AGENT_H

#include <memory>


#include "evolution/population.h"
#include "evolution/representation.h"
#include "evolution/mutation.h"

#include "mutator/rng.h"

namespace Evolution {

    /**
     * \brief Class for evolutionnary algorithm.
     */
    class EvolutionAlgorithm
    {
      protected: 

        /// Representation of the EA
        std::unique_ptr<Representation> representation;

        /// Population of the EA
        std::unique_ptr<Population> population;

        /// Mutation of the EA
        std::unique_ptr<Mutation> mutation;

        /// Random Number Generator for this EA
        RNG::RNG rng;

      public:
        virtual ~EvolutionAlgorithm() = default;

        /**
         * \brief Constructor of EvolutionAlgorithm class
         * 
         * \param[in] representation the representation of the individuals.
         * \param[in] seed seed used for the random number generator.
         */
        EvolutionAlgorithm(const Representation& representation, size_t seed = 0)
        : representation(std::make_unique<Representation>(representation)), 
          population(std::make_unique<Population>()), 
          mutation(std::make_unique<Mutation>()), 
          rng{seed} {};

        /**
         * \brief Creates individuals of the first generation.
         */
        void initializePopulation();

        /// @brief Return the population of the EA
        const Population& getPopulation();

        /// @brief Return the representation of the EA
        const Representation& getRepresentation();

    };
}; // namespace Evolution

#endif // EVOLUTION_AGENT_H