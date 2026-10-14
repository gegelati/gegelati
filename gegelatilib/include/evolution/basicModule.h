#ifndef EVOLUTION_BASIC_MODULE_H
#define EVOLUTION_BASIC_MODULE_H

#include "evolution/module.h"

#include "evaluation/parallelEvaluator.h"

#include "evaluation/predictionProblem.h"
#include "evaluation/reinforcementProblem.h"
#include "metrics/predictionMetric.h"
#include "metrics/rewardsMetric.h"

#include "fitnessAssignment/defaultAssigner.h"
#include "selection/randomSelector.h"
#include "selection/truncationSelector.h"
#include "reproduction/replicator.h"
#include "mutation/pointMutator.h"

#include "log/basiclogger.h"


namespace Evolution {

    /**
     * \brief Class for creating the default evolutionnary module.
     * 
     * A default module is composed of:
     *  - A representation
     *  - An evolutionnary problem
     *  - A fitness assignement method (default is basic score: Sum of rewards for RL, prediction accuracy for classification etc)
     *  - A selector for parent selection (default is random)
     *  - A selector for surviving selection (default is truncation)
     *  - A breeder (default is replication)
     *  - A mutator (default is point mutation)
     */
    class BasicModule: public Module {

        protected:

            /// @brief TODO
            std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> population;

            /// @brief TODO
            std::map<std::shared_ptr<const Individual>, double, SharedLess<Individual>> individualFitness;

            /// @brief TODO
            std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>> populationMetrics;

            /// @brief TODO
            const Representations::Representation& representation;

            /// @brief TODO
            Evaluation::Problem& problem;

            /// \brief Number of individual kept in the population
            size_t sizePopulation;

            /// \brief number of offspring generated and evaluated per generation
            size_t nbOffspring;

            /// \brief number of evaluation iteration perform on training mode on the individuals
            size_t nbTrainingIterations;

            /// \brief Number of threads autorized
            size_t nbThreads;

            /// \brief seed of the evolution
            uint64_t seed;

            /// @brief TODO
            std::unique_ptr<Evaluation::Evaluator> evaluator;

            /// @brief TODO
            std::shared_ptr<FitnessAssignment::Assigner> assigner;

            /// @brief TODO
            std::shared_ptr<Selection::Selector> parentSelection;

            /// @brief TODO
            std::shared_ptr<Selection::Selector> survivingSelection;

            /// @brief TODO
            std::shared_ptr<Reproduction::Reproducer> breeder;

            /// @brief TODO
            std::shared_ptr<Mutation::Mutator> mutation;

            /// @brief TODO
            std::unique_ptr<GraphBased::GenotypeGenerator> generator;

            /// @brief Todo
            std::shared_ptr<Metrics::MetricMapTemplate> metricTemplate;

            /// @brief Todo
            uint64_t fitnessMetricHash;

            /// @brief Default loggers
            std::vector<std::shared_ptr<Log::Basiclogger>> basicloggers;

        public:

            /// @brief Random number generator
            RNG::RNG rng;

            /**
             * \brief Default constructor of module
             */
            explicit BasicModule(
                const Representations::Representation& representation, 
                Evaluation::Problem& problem,
                size_t sizePopulation, size_t nbOffspring, size_t nbTrainingIterations, uint64_t seed = 0, size_t nbThreads = 1,
                std::shared_ptr<FitnessAssignment::Assigner> assigner = nullptr,
                std::shared_ptr<Selection::Selector> parentSelection = nullptr,
                std::shared_ptr<Selection::Selector> survivingSelection = nullptr,
                std::shared_ptr<Reproduction::Reproducer> breeder = nullptr,
                std::shared_ptr<Mutation::Mutator> mutation = nullptr
            );

            /// @brief print the summary of the current evolution 
            virtual std::string summary() const override;

            /**
             * \brief Initialize the evolution
             */
            virtual void initialize() override;

            /**
             * \brief Method used during initialize to create the column header of the basicLoggers
             */
            virtual void setBasicLoggerHeader();

            /**
             * \brief initialize the metric template based on the evaluation problem
             */
            void initializeMetricTemplate();

            /**
             * \brief add a metric to measure for evaluation.
             */
            virtual void addMetricToTemplate(std::unique_ptr<Metrics::Metric> metric);

            /**
             * \brief Perform one generation
             * 
             * \param[in] generationNumber the index of the generation
             */
            virtual void doGeneration(size_t generationNumber) override;

            /**
             * \brief Method used during doGeneration to add a raw to the basicLoggers
             * 
             * \param[in] generationNumber the index of the generation
             */
            virtual void setNewBasicLoggerRow(size_t generationNumber);

            /**
             * \brief Default logger
             */
            virtual void addBasiclogger(std::ostream& output = std::cout);
    };
};

#endif // EVOLUTION_BASIC_MODULE_H