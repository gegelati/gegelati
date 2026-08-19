#ifndef EVOLUTION_AGENT_H
#define EVOLUTION_AGENT_H

#include <memory>


#include "evaluation/evaluationAgent.h"

#include "evolution/population.h"
#include "evolution/representation.h"
#include "evolution/mutation.h"
#include "evolution/survivingSelection.h"

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

        /// evaluation agent of the EA
        std::unique_ptr<Evaluation::EvaluationAgent> evaluation;

        /// Surviving Selection agent of the EA
        std::unique_ptr<SurvivingSelection> survivingSelection;

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
          survivingSelection(std::make_unique<SurvivingSelection>()), 
          evaluation(std::make_unique<Evaluation::EvaluationAgent>(le, std::move(evalParams), evalSeed)){
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
        const Evaluation::EvaluationAgent& getEvaluation();

        /// @brief Return the selector of the EA
        const SurvivingSelection& getSelector();

        /// @brief Return the Random Number Generator of the EA 
        RNG::RNG& getRNG();

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
         * \param[in] parents the set of selected parents
         */
        virtual std::set<std::unique_ptr<Individual>, UniqueLess<Individual>> reproduceParents(
          std::vector<std::reference_wrapper<const Individual>> parents
        );

        /**
         * \brief mutate the specified offspring based on the genotypeTemplate of the representation.
         * 
         * \param[in] offspring the set of offspring to mutate
         */
        virtual void mutateOffspring(const std::set<std::unique_ptr<Individual>, UniqueLess<Individual>>& offspring);

        
        /**
         * \brief Evaluate the current individuals of the population if they need to, and the offspring.
         * 
         * \param[in] offspring the set of offspring to evaluate
         * \param[in] generationNumber the integer number of the current
         * generation.
         * \param[in] mode the LearningMode to use during the evaluation.
         */
        std::map<std::reference_wrapper<const Individual>, std::shared_ptr<Evaluation::EvaluationResult>> evaluatePopulation(
                                const std::set<std::unique_ptr<Individual>, UniqueLess<Individual>>& offspring,
                                size_t generationNumber, Learn::LearningMode mode);

        /**
         * \brief select the suvivor individual from the evaluation batch.
         * 
         * \param[in] scores score achieved by the individuals.
         * 
         * \return the list of looser individuals
         */
        virtual std::map<std::reference_wrapper<const Individual>, bool> selectSurvivors(std::map<std::reference_wrapper<const Individual>, std::shared_ptr<Evaluation::EvaluationResult>>& scores);

        /**
         * \brief perform the replacement of the population based on the select survivors.
         * 
         * \param[in] offspring offspring generated.
         * \param[in] selectionResult Individuals from either the population or the offspring that are selected or not.
         */
        virtual void replacePopulation(
          std::set<std::unique_ptr<Individual>, UniqueLess<Individual>>& offspring, 
          std::map<std::reference_wrapper<const Individual>, bool>& selectionResult);
    };
}; // namespace Evolution

#endif // EVOLUTION_AGENT_H