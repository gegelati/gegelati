#ifndef EVOLUTION_AGENT_H
#define EVOLUTION_AGENT_H

#include <memory>


#include "evaluation/evaluationAgent.h"

#include "evolution/population.h"
#include "evolution/representation.h"
#include "evolution/mutation.h"

#include "mutator/rng.h"

#include "selector/selector.h"
#include "selector/selectorFactory.h"

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

        /// evaluation agent of the EA
        std::unique_ptr<Learn::EvaluationAgent> evaluation;

        /// Selection agent of the EA
        std::unique_ptr<Selector::Selector> selection;

        /// Random Number Generator for this EA
        RNG::RNG rng;

      public:
        virtual ~EvolutionAlgorithm() = default;

        /**
         * \brief Constructor of EvolutionAlgorithm class
         * 
         * \param[in] representation the representation of the individuals.
         * \param[in] le The LearningEnvironment for the TPG.
         * \param[in] evalParams The LearningParameters for the EvaluationAgent.
         * \param[in] evoSeed Seed for random number generator of the evolution.
         * \param[in] evalSeed seed used for the random number generator of the evaluation.
         */
        EvolutionAlgorithm(
          const Representation& representation, 
          Learn::LearningEnvironment& le, 
          std::unique_ptr<Learn::LearningParameters> evalParams = std::make_unique<Learn::LearningParameters>(),
          size_t evoSeed = 0,
          size_t evalSeed = 1)
        : representation(std::move(representation.cloneUniquePtr())), 
          population(std::make_unique<Population>()), 
          mutation(std::make_unique<Mutation>()), 
          evaluation(std::make_unique<Learn::EvaluationAgent>(le, std::move(evalParams), evalSeed)),
          selection{std::move(Selector::selectorFactory())} {
            rng.setSeed(evoSeed);
            this->representation->setInputDimensions(le.getDataSources());
          };

        /// @brief Return the population of the EA
        const Population& getPopulation();

        /// @brief Return the representation of the EA
        const Representation& getRepresentation();

        /// @brief Return the mutation agent of the EA
        const Mutation& getMutation();

        /// @brief Return the evaluation agent of the EA
        const Learn::EvaluationAgent& getEvaluation();

        /// @brief Return the selector of the EA
        const Selector::Selector& getSelector();


        /**
         * \brief Creates individuals of the first generation.
         */
        void initializePopulation();

        /**
         * \brief Select parents used to generate offspring.
         * 
         * \param[in] nbParents the number of parents to select.
         */
        virtual std::vector<std::reference_wrapper<const Individual>> selectParents(size_t nbParents);

        /**
         * \brief Generate offspring based on selected parents
         * 
         * \param[in] parents the vector of selected parents
         */
        virtual std::vector<std::reference_wrapper<const Individual>> reproduceParents(
          std::vector<std::reference_wrapper<const Individual>> parents
        );

        /**
         * \brief mutate the specified offspring based on the genotypeTemplate of the representation.
         * 
         * \param[in] offspring the vector of offspring to mutate
         */
        virtual void mutateOffspring(std::vector<std::reference_wrapper<const Individual>> offspring);

        
        /**
         * \brief Evaluate the current individuals of the population.
         * 
         * \param[in] generationNumber the integer number of the current
         * generation.
         * \param[in] mode the LearningMode to use during the evaluation.
         */
        std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                              std::reference_wrapper<const Individual>> evaluatePopulation(
                                size_t generationNumber, Learn::LearningMode mode);

        /**
         * \brief perform the replacement of the population based on a selection method.
         * 
         * \param[in] scores score achieved by the individuals of the population.
         */
        virtual void replacePopulation(std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                              std::reference_wrapper<const Individual>>& scores);
    };
}; // namespace Evolution

#endif // EVOLUTION_AGENT_H