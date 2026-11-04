#ifndef EVOLUTION_BASIC_EVOLUTION_H
#define EVOLUTION_BASIC_EVOLUTION_H

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "evaluation/evaluator.h"
#include "individual.h"
#include "metrics/metricMap.h"
#include "evolution/module.h"
#include "evolution/steps/logStep.h"
#include "fitnessAssignment/assigner.h"
#include "graphBased/genotypeGenerator.h"
#include "metrics/metric.h"
#include "mutation/mutator.h"
#include "reproduction/reproducer.h"
#include "selection/selector.h"

namespace Evolution {

    /**
     * \brief Settings of the classic evolutionary flow of one population, building the corresponding steps.
     *
     * \code
     * Evolution::BasicEvolution basic;
     * basic.representation = representation;
     * basic.problem = problem;
     * basic.sizePopulation = 100;
     * basic.nbOffspring = 100;
     * std::shared_ptr<Evolution::Module> population = basic.build();
     * auto mutation = population->find<MutateStep>("Mutation");
     * Evolution::EvolutionData data = population->evolve(100, seed);
     * \endcode
     *
     * Initialization steps: InitPopulation, InitialEvaluation, TrainingFitness (and ParentFitness).
     * Generation steps: ParentSelection, Reproduction, Mutation, TrainingEvaluation, TrainingFitness (and ParentFitness),
     * SurvivorSelection, then ValidationEvaluation and ValidationFitness if validation iterations are requested.
     * The steps of the generation are a module named by names.evolution, whose initialization is the initialization steps
     * (see build). It can be edited, repeated or run every few generations, and added to the steps of other evolutions.
     *
     * Null components are replaced by the defaults: random parent selection, truncation survival,
     * replication, point mutation, and the average of the rewards (or the accuracy) as fitness.
     */
    struct BasicEvolution {
        /**
         * \brief Names of the data and of the steps of the evolution, all of them changeable before build.
         *
         * The data are identified by their names (see Key), the steps are labelled by theirs (summaries, Module::find).
         * The default names are generic ("Population", "Parents"...). To run several evolutions together, give each one its own
         * names, and change any of them as needed:
         * \code
         * second.names.population = "SecondPopulation";
         * second.names.parents = "SecondParents";
         * second.names.mutationStep = "Variation";
         * \endcode
         */
        struct Names {

            /// Data: the individuals of the population.
            std::string population = "Population";

            /// Data: the parents selected at each generation.
            std::string parents = "Parents";

            /// Data: the offspring created at each generation.
            std::string offspring = "Offspring";

            /// Data: the metrics of the training evaluations.
            std::string trainingMetrics = "trainingMetrics";

            /// Data: the metrics of the validation evaluations.
            std::string validationMetrics = "validationMetrics";

            /// Data: the fitness of the training.
            std::string trainingFitness = "trainingFitness";

            /// Data: the fitness of the parent selection, used when a parentAssigner is set.
            std::string parentFitness = "parentFitness";

            /// Data: the fitness of the validation.
            std::string validationFitness = "validationFitness";

            /// Data: the generator of the genotypes, created by CreateGeneratorStep and read by the steps creating and mutating the population.
            std::string generator = "generator";

            /// Module of the evolution, holding the steps of a generation and the initialization.
            std::string evolution = "Evolution";

            /// Module holding the steps of the initialization.
            std::string initialization = "Initialization";

            /// Step creating the individuals.
            std::string initPopulationStep = "InitPopulation";

            /// Step evaluating the individuals created.
            std::string initialEvaluationStep = "InitialEvaluation";

            /// Step computing the training fitness.
            std::string trainingFitnessStep = "TrainingFitness";

            /// Step computing the fitness of the parent selection.
            std::string parentFitnessStep = "ParentFitness";

            /// Step selecting the parents.
            std::string parentSelectionStep = "ParentSelection";

            /// Step creating the offspring.
            std::string reproductionStep = "Reproduction";

            /// Step mutating the offspring.
            std::string mutationStep = "Mutation";

            /// Step evaluating the individuals and the offspring in training mode.
            std::string trainingEvaluationStep = "TrainingEvaluation";

            /// Step selecting the survivors.
            std::string survivorSelectionStep = "SurvivorSelection";

            /// Step evaluating the individuals in validation mode.
            std::string validationEvaluationStep = "ValidationEvaluation";

            /// Step computing the validation fitness.
            std::string validationFitnessStep = "ValidationFitness";

            /// Step creating the generator of the genotypes.
            std::string createGeneratorStep = "CreateGenerator";

            /// Step writing the log of the evolution.
            std::string logStep = "Log";
        };

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

        /// Names of the data and of the steps, changeable before build.
        Names names;

        /// Number of evaluations of each individual in validation mode, zero to disable the validation.
        size_t nbValidationIterations = 0;

        /// Number of generations between two validations (see EvolutionData::isUpToDate).
        size_t validationPeriod = 1;

        /// Stream receiving the log of the evolution at each generation (see logColumns), the console by default, null for no log. It must outlive the evolution.
        std::ostream* logOutput = &std::cout;

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

        /// Generator of the genotypes, the naive generator of the constraint of the representation if null. It is created once by the first step of the initialization.
        std::shared_ptr<GraphBased::GenotypeGenerator> generator = nullptr;

        /// Metrics measured during the evaluations in addition to the one of the fitness. Each evaluation step uses an empty clone.
        std::vector<std::shared_ptr<const Metrics::Metric>> metrics;

        /// Problem of the validation, the training problem if null.
        std::shared_ptr<Evaluation::Problem> validationProblem = nullptr;

        /// Fitness assignment method of the validation, the assigner if null.
        std::shared_ptr<FitnessAssignment::Assigner> validationAssigner = nullptr;

        /// Default polymorphic destructor.
        virtual ~BasicEvolution() = default;

        /**
         * \brief Build the module of the evolution: the steps of a generation, with the initialization.
         *
         * It goes in two steps. First the settings are read to create what the modules need: the data
         * (from the names), the metrics, the evaluator and the default components. Then the modules are
         * built: the initialization steps, and the steps of a generation which has them as its initialization.
         * The result can be edited, evolved, or added to the modules of other evolutions (see Module::evolve).
         *
         * \return the module, named by names.evolution.
         * \throw std::runtime_error if the representation or the problem is null, if a size is zero, if they are not compatible,
         * if a problem is neither a prediction nor a reinforcement problem, or if the validation period is zero.
         */
        virtual std::shared_ptr<Module> build() const;

        /**
         * \brief Columns of the log of the evolution, made from its names and settings.
         *
         * They show the size of the population, its training fitness and, if validation is requested, its validation fitness.
         * Add, remove or replace columns to customize the log, then give them to a LogStep.
         *
         * \return the columns.
         */
        virtual std::vector<LogStep::Column> logColumns() const;

      protected:
        /// Throw if a required setting is missing or the settings are not compatible.
        void checkSettings() const;

        /// Add the step writing the log of the evolution, the last step of a generation, if logOutput is set.
        void addLogStep(Module& step) const;

        /// Return the metrics measured in a problem, and the hash of the metric of the fitness, which depends on the kind of problem.
        std::pair<std::shared_ptr<Metrics::MetricMapTemplate>, uint64_t> makeMetrics(const Evaluation::Problem& evaluatedProblem) const;

        /// Return the evaluator running the evaluations, parallel if several threads are authorized.
        std::shared_ptr<Evaluation::Evaluator> makeEvaluator() const;

        /// Return the assigner of the fitness, the default one if none is set.
        std::shared_ptr<FitnessAssignment::Assigner> usedAssigner() const;

        /**
         * \brief Add the steps computing the training fitness (and the parent fitness) from the training metrics.
         *
         * \param[in,out] step the step receiving the steps.
         * \param[in] metricHash hash of the metric from which the fitness is computed.
         */
        void addFitnessSteps(Module& step, uint64_t metricHash) const;

        /**
         * \brief Add the steps selecting the parents and reproducing them.
         *
         * \param[in,out] step the step receiving the steps.
         */
        void addReproductionSteps(Module& step) const;

        /**
         * \brief Add the step selecting the survivors among the population and its offspring.
         *
         * \param[in,out] step the step receiving the step.
         */
        void addSurvivalStep(Module& step) const;

        /**
         * \brief Add the steps of the validation, if requested.
         *
         * \param[in,out] step the step receiving the steps.
         * \param[in] evaluator evaluator running the evaluations.
         */
        void addValidationSteps(Module& step, std::shared_ptr<Evaluation::Evaluator> evaluator) const;
    };

}; // namespace Evolution

#endif // EVOLUTION_BASIC_EVOLUTION_H
