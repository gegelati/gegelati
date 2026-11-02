#ifndef EVOLUTION_BASIC_EVOLUTION_H
#define EVOLUTION_BASIC_EVOLUTION_H

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "evaluation/evaluator.h"
#include "evolution/module.h"
#include "evolution/population.h"
#include "fitnessAssignment/assigner.h"
#include "mutation/mutator.h"
#include "reproduction/reproducer.h"
#include "selection/selector.h"
#include "metrics/metric.h"

namespace Evolution {

    /**
     * \brief Settings of the classic evolutionary flow of one population, building the corresponding module.
     *
     * \code
     * Evolution::BasicEvolution basic;
     * basic.representation = representation;
     * basic.problem = problem;
     * basic.sizePopulation = 100;
     * basic.nbOffspring = 100;
     * Evolution::Module module = basic.buildModule();
     * auto mutation = module.generationSteps().find<MutateStep>("Mutation");
     * \endcode
     *
     * Initialization steps: InitPopulation, InitialEvaluation, TrainingFitness (and ParentFitness).
     * Generation steps: ParentSelection, Reproduction, Mutation, TrainingEvaluation, TrainingFitness (and ParentFitness),
     * SurvivorSelection, then ValidationEvaluation and ValidationFitness if validation iterations are requested.
     * The data exchanged by the steps are data of the population, whose names are settings (e.g. trainingFitnessName),
     * so other steps can read them: population.data(basic.trainingFitnessName).
     *
     * Null components are replaced by the defaults: random parent selection, truncation survival,
     * replication, point mutation, and the average of the rewards (or the accuracy) as fitness.
     */
    struct BasicEvolution {
        /// Representation of the evolved individuals, required.
        std::shared_ptr<const Representations::Representation> representation = nullptr;

        /// Problem in which the individuals are evaluated, required.
        std::shared_ptr<Evaluation::Problem> problem = nullptr;

        /// Number of individuals kept in the population, required.
        size_t sizePopulation = 0;

        /// Number of offspring created and evaluated per generation, required.
        size_t nbOffspring = 0;

        /// Number of evaluations of each individual in training mode.
        size_t nbTrainingIterations = 1;

        /// Name of the population.
        std::string populationName = "Population";

        /// Name of the selected parents, a data of the population.
        std::string parentsName = "parents";

        /// Name of the offspring, a data of the population.
        std::string offspringName = "offspring";

        /// Name of the metrics of the training evaluations, a data of the population.
        std::string trainingMetricsName = "trainingMetrics";

        /// Name of the fitness of the training, a data of the population.
        std::string trainingFitnessName = "trainingFitness";

        /// Name of the fitness of the parent selection when a parentAssigner is set, a data of the population.
        std::string parentFitnessName = "parentFitness";

        /// Name of the metrics of the validation evaluations, a data of the population.
        std::string validationMetricsName = "validationMetrics";

        /// Name of the fitness of the validation, a data of the population.
        std::string validationFitnessName = "validationFitness";

        /// Seed of the module built by buildModule.
        uint64_t seed = 0;

        /// Number of evaluations of each individual in validation mode, zero to disable the validation.
        size_t nbValidationIterations = 0;

        /// Number of generations between two validations (see EvolutionState::isUpToDate).
        size_t validationPeriod = 1;

        /// Number of threads authorized.
        size_t nbThreads = 1;

        /// Fitness assignment method, DefaultAssigner if null. Used for the survival, and for the parents without parentAssigner.
        std::shared_ptr<FitnessAssignment::Assigner> assigner = nullptr;

        /// Fitness assignment method of the parent selection (e.g. novelty), the assigner if null.
        std::shared_ptr<FitnessAssignment::Assigner> parentAssigner = nullptr;

        /// Selector of the parents, RandomSelector if null.
        std::shared_ptr<Selection::Selector> parentSelection = nullptr;

        /// Selector of the survivors, TruncationSelector if null.
        std::shared_ptr<Selection::Selector> survivingSelection = nullptr;

        /// Reproduction method, Replicator if null.
        std::shared_ptr<Reproduction::Reproducer> breeder = nullptr;

        /// Mutator of the offspring, PointMutator(0.2, 0.5, 0.5) if null.
        std::shared_ptr<Mutation::Mutator> mutation = nullptr;

        /// Metrics measured during the evaluations in addition to the one of the fitness. Each evaluation step uses an empty clone.
        std::vector<std::shared_ptr<const Metrics::Metric>> metrics;

        /// Problem of the validation, the training problem if null.
        std::shared_ptr<Evaluation::Problem> validationProblem = nullptr;

        /// Fitness assignment method of the validation, the assigner if null.
        std::shared_ptr<FitnessAssignment::Assigner> validationAssigner = nullptr;

        /// Default polymorphic destructor.
        virtual ~BasicEvolution() = default;

        /**
         * \brief Build a module with the population and the steps of the evolution.
         *
         * \return the module, with the seed of the settings.
         * \throw std::runtime_error if a required setting is missing or the settings are not compatible (see addTo).
         */
        Module buildModule() const;

        /**
         * \brief Add the population and the steps of the evolution to a module, e.g. next to other populations.
         *
         * \param[in,out] module the module receiving the population and the steps.
         * \throw std::runtime_error if the representation or the problem is null, if a size is zero, if they are not compatible,
         * if a problem is neither a prediction nor a reinforcement problem, if the validation period is zero,
         * or if the module already has a population with the same name.
         */
        virtual void addTo(Module& module) const;

      protected:
        /// Throw if a required setting is missing or the settings are not compatible.
        void checkSettings() const;

        /// Return the metrics measured in a problem, and the hash of the metric of the fitness, which depends on the kind of problem.
        std::pair<std::shared_ptr<Metrics::MetricMapTemplate>, uint64_t> makeMetrics(const Evaluation::Problem& evaluatedProblem) const;

        /// Return the evaluator running the evaluations, parallel if several threads are authorized.
        std::shared_ptr<Evaluation::Evaluator> makeEvaluator() const;

        /// Return the assigner of the fitness, the default one if none is set.
        std::shared_ptr<FitnessAssignment::Assigner> usedAssigner() const;

        /**
         * \brief Add the steps computing the training fitness (and the parent fitness) from the training metrics.
         *
         * \param[in,out] module the module receiving the steps.
         * \param[in] population the evolved population.
         * \param[in] metricHash hash of the metric from which the fitness is computed.
         * \param[in] atInitialization indicates if the steps are added to the initialization, or to each generation.
         */
        void addFitnessSteps(Module& module, const Population& population, uint64_t metricHash, bool atInitialization) const;

        /**
         * \brief Add the steps selecting the parents and reproducing them, at the end of each generation.
         *
         * \param[in,out] module the module receiving the steps.
         * \param[in] population the evolved population.
         */
        void addReproductionSteps(Module& module, const Population& population) const;

        /**
         * \brief Add the step selecting the survivors among the population and its offspring, at the end of each generation.
         *
         * \param[in,out] module the module receiving the step.
         * \param[in] population the evolved population.
         */
        void addSurvivalStep(Module& module, const Population& population) const;

        /**
         * \brief Add the steps of the validation, if requested, at the end of each generation.
         *
         * \param[in,out] module the module receiving the steps.
         * \param[in] population the evolved population.
         * \param[in] evaluator evaluator running the evaluations.
         */
        void addValidationSteps(Module& module, const Population& population, std::shared_ptr<Evaluation::Evaluator> evaluator) const;
    };

}; // namespace Evolution

#endif // EVOLUTION_BASIC_EVOLUTION_H
