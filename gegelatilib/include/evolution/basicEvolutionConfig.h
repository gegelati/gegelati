#ifndef EVOLUTION_BASIC_EVOLUTION_H
#define EVOLUTION_BASIC_EVOLUTION_H

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "evaluation/problem.h"
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
     * Evolution::BasicEvolutionConfig basic;
     * basic.representation = representation;
     * basic.problem = problem;
     * basic.sizePopulation = 100;
     * basic.nbOffspring = 100;
     * std::shared_ptr<Evolution::Module> population = basic.buildModule();
     * std::shared_ptr<Evolution::MutateStep> mutation = population->find<Evolution::MutateStep>("Mutation");
     * Evolution::EvolutionData& data = population->evolve(100, seed);
     * \endcode
     *
     * Initialization steps: InitPopulation, InitialEvaluation, TrainingFitness (then InitValidation).
     * Generation steps: (ParentFitness), ParentSelection, Reproduction, Mutation, TrainingEvaluation, TrainingFitness,
     * SurvivorSelection, then Validation (ValidationEvaluation and ValidationFitness) if validation iterations are requested, and Log.
     * The steps of the generation are a module named by names.evolution, whose initialization is the initialization steps
     * (see buildModule). It can be edited, repeated or run every few generations, and added to the steps of other evolutions.
     *
     * The components (problem, representation, assigners, selectors, breeder, mutation) are stored in the data of the module under their
     * name (Util::Named::getName), the generator and the metric templates under names.generator and names.*MetricTemplate. Two different
     * components of the same evolution must therefore have different names (e.g. a validation problem of the same class as the training problem).
     *
     * Null components are replaced by the defaults when the module is built: random parent selection, truncation survival,
     * replication, point mutation, and the default metric of the problem (rewards or predictions, see Evaluation::Problem::getDefaultFitnessMetric)
     * averaged by a DefaultAssigner as fitness.
     */
    struct BasicEvolutionConfig {
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

            /// Component: the generator of the genotypes, read by the steps creating and mutating the population.
            std::string generator = "generator";

            /// Component: the metrics measured by the training evaluations (trainingMetricTemplate).
            std::string trainingMetricTemplate = "trainingMetricTemplate";

            /// Component: the metrics measured by the validation evaluations (validationMetricTemplate).
            std::string validationMetricTemplate = "validationMetricTemplate";

            /// Module of the evolution, holding the steps of a generation and the initialization.
            std::string evolution = "Evolution";

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

            /// Step writing the log of the evolution.
            std::string logStep = "Log";
        };

        /// Names of the data and of the steps, changeable before build.
        Names names;

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

        /// Number of evaluations of each individual in validation mode, zero to disable the validation.
        size_t nbValidationIterations = 0;

        /// Number of generations between two validations (see EvolutionData::isUpToDate).
        size_t validationPeriod = 1;

        /// Stream receiving the log of the evolution at each generation (see logColumns), the console by default, null for no log. It must outlive the evolution.
        std::ostream* logOutput = &std::cout;

        /// Number of threads authorized.
        size_t nbThreads = 1;

        /// Metrics measured during the training evaluations, empty if null. The default metric of the problem is added if an assigner reads it, the metrics read by the other assigners must be in it.
        std::shared_ptr<Metrics::MetricMapTemplate> trainingMetricTemplate = nullptr;

        /// Fitness assignment method, DefaultAssigner of the default metric of the problem if null. Used for the survival, and for the parents without parentAssigner.
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

        /// Generator of the genotypes, the naive generator of the constraint of the representation if null.
        std::shared_ptr<GraphBased::GenotypeGenerator> generator = nullptr;

        /// Problem of the validation, optional: the validation is evaluated on the training problem if null (it then stays null).
        std::shared_ptr<Evaluation::Problem> validationProblem = nullptr;

        /// Fitness assignment method of the validation, optional: the training assigner is used if null (it then stays null).
        std::shared_ptr<FitnessAssignment::Assigner> validationAssigner = nullptr;

        /// Metrics measured during the validation evaluations, empty if null. The default metric is added like for the training.
        std::shared_ptr<Metrics::MetricMapTemplate> validationMetricTemplate = nullptr;

        /// Default polymorphic destructor.
        virtual ~BasicEvolutionConfig() = default;

        /**
         * \brief Build the module of the evolution: the steps of a generation, with the initialization.
         *
         * It goes in steps. First the null components of the settings are replaced by the defaults (the settings are updated,
         * see updateEmptyComponents), then they are stored in the data of the module, then the modules are built:
         * the initialization steps, and the steps of a generation which has them as its initialization.
         * The result can be edited, evolved, or added to the modules of other evolutions (see Module::evolve).
         *
         * \return the module, named by names.evolution.
         * \throw std::runtime_error if the representation or the problem is null, if a size is zero, if they are not compatible,
         * if the validation period is zero while validating, if a metric template lacks the metric of its assigner,
         * or if two components have the same name.
         */
        virtual std::shared_ptr<Module> buildModule();

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
        /**
         * \brief Check the settings and replace the null components by the defaults, the generator and the mutation excepted.
         *
         * The default metrics of the problems are added to the metric templates when an assigner reads them.
         *
         * \throw std::runtime_error if the settings are missing or not compatible (see buildModule).
         */
        virtual void updateEmptyComponents();

        /**
         * \brief Store the components shared by the evolutions in the data of the module, the generator and the mutation excepted.
         *
         * \param[in,out] module the module receiving the components in its data.
         */
        void addComponents(Module& module) const;

        /**
         * \brief Return a module evaluating the population in validation mode and computing its validation fitness.
         *
         * \param[in] name name of the module.
         * \param[in] period number of generations between two runs of the module.
         */
        virtual std::shared_ptr<Module> makeValidationModule(const std::string& name, size_t period) const;

        /**
         * \brief Check that a problem can evaluate the individuals of a representation.
         *
         * \param[in] representation the representation of the individuals.
         * \param[in] problem the problem in which they are evaluated.
         * \throw std::runtime_error if the input or output requirements of the representation and of the problem are not compatible.
         */
        static void checkCompatible(const Representations::Representation& representation, const Evaluation::Problem& problem);
    };

}; // namespace Evolution

#endif // EVOLUTION_BASIC_EVOLUTION_H
