#ifndef EVOLUTION_STEPS_TPG_STEPS_H
#define EVOLUTION_STEPS_TPG_STEPS_H

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "dimensions/requirement.h"
#include "evaluation/dataset.h"
#include "evaluation/evaluator.h"
#include "evaluation/predictionProblem.h"
#include "individual.h"
#include "metrics/metricMap.h"
#include "evolution/module.h"
#include "mutation/mutator.h"
#include "mutation/nodeTransferMutation.h"
#include "representations/TPG.h"
#include "representations/representation.h"

namespace Evolution {

    /**
     * \brief Step mutating TPG vertices with the generator found in the data.
     *
     * Added nodes are copies of the nodes of the current roots: the step gives the roots to its NodeTransferMutation
     * before mutating, without copying the mutation. The generator is a data of the evolution, built again from the current
     * vertices by a previous step (see OriginTPGEvolution).
     */
    class MutateVerticesStep : public Module {
        protected:
            /// Name of the mutated vertices, a list of mutable individuals.
            std::string individuals;

            /// Name of the current roots, a list of individuals.
            std::string roots;

            /// Name of the generator of the genotypes.
            std::string generator;

            /// Mutation of the vertices, transferring the nodes of the roots.
            std::shared_ptr<Mutation::NodeTransferMutation> mutator;

            /// Maximum number of threads used to mutate.
            size_t nbThreads;

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] individuals name of the mutated vertices, a list of mutable individuals (e.g. offspring).
             * \param[in] roots name of the current roots, a list of individuals (e.g. the individuals of a population).
             * \param[in] generator name of the generator of the genotypes.
             * \param[in] mutator mutation of the vertices, its potential nodes are set by the step.
             * \param[in] nbThreads maximum number of threads used to mutate.
             * \param[in] name name of the step.
             * \throw std::runtime_error if the mutator is null.
             */
            MutateVerticesStep(std::string individuals, std::string roots, std::string generator,
                               std::shared_ptr<Mutation::NodeTransferMutation> mutator, size_t nbThreads = 1, std::string name = "MutateVertices");

            /// Inherited from Module
            virtual std::string details() const override;
            /// Inherited from Module
            virtual void apply(EvolutionData& state) override;
    };

    /**
     * \brief Step caching the outputs of the members on the samples of a dataset.
     *
     * The members of the individuals about to be evaluated are evaluated once on the samples of the evaluation,
     * and their outputs are cached in the members (Individual::cacheOutput), so that a member shared by many vertices
     * is executed only once per sample. This is only an optimization: results are unchanged.
     *
     * The cached members are a data of the state, and their outputs are cleared at the next run of the step,
     * so the cache only holds the samples of the current generation.
     * Outputs are indexed by the address of the samples: the dataset of the problem must outlive the individuals.
     */
    class CacheMemberOutputsStep : public Module {
        protected:
            /// Names of the lists of individuals about to be evaluated.
            std::vector<std::string> evaluated;

            /// Name of the cached members, a list of individuals.
            std::string cache;

            /// Problem in which members are evaluated.
            std::shared_ptr<Evaluation::PredictionProblem> problem;

            /// Evaluator running the evaluations.
            std::shared_ptr<const Evaluation::Evaluator> evaluator;

            /// Number of samples of the evaluation.
            size_t nbIterations;

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] evaluated names of the lists of individuals about to be evaluated (list of individuals or list of mutable individuals).
             * \param[in] cache name of the cached members, a list of individuals.
             * \param[in] problem problem in which members are evaluated.
             * \param[in] evaluator evaluator running the evaluations.
             * \param[in] nbIterations number of samples of the evaluation.
             * \param[in] name name of the step.
             * \throw std::runtime_error if the problem is null.
             */
            CacheMemberOutputsStep(std::vector<std::string> evaluated, std::string cache, std::shared_ptr<Evaluation::PredictionProblem> problem,
                                   std::shared_ptr<const Evaluation::Evaluator> evaluator, size_t nbIterations, std::string name = "CacheMemberOutputs");

            /// Inherited from Module
            virtual std::string details() const override;
            /// Inherited from Module
            virtual void apply(EvolutionData& state) override;
    };

    /**
     * \brief Step creating an empty archive of inputs and of the answers of the members.
     *
     * It must be run at the beginning of the initialization, before the steps creating members from the archive.
     */
    class CreateArchiveStep : public Module {
        protected:
            /// Name of the created archive, an Evaluation::DataSet.
            std::string archive;

            /// Requirements of the inputs stored in the archive.
            std::vector<Dimensions::Requirement> inputRequirements;

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] archive name of the created archive, an Evaluation::DataSet.
             * \param[in] inputRequirements requirements of the inputs stored in the archive.
             * \param[in] name name of the step.
             */
            CreateArchiveStep(std::string archive, std::vector<Dimensions::Requirement> inputRequirements, std::string name = "CreateArchive")
                : Module(std::move(name)), archive{std::move(archive)}, inputRequirements{std::move(inputRequirements)}
            {
                this->provide(this->archive);
            }

            /// Inherited from Module
            virtual void apply(EvolutionData& state) override;
    };

    /**
     * \brief Step maintaining the archive of inputs and of the answers of the members of the roots on these inputs.
     *
     * The archive is used to measure the novelty of mutated members: a member must answer differently
     * from the existing ones on the archive. The step must be run during the initialization and at each generation.
     *
     * Inputs are sampled from the InputMetric of the metrics of the roots. If the archive is fixed, it is only sampled
     * during the initialization, and only the answers are updated afterwards: the evaluations of the generations
     * do not need to measure the InputMetric.
     */
    class UpdateArchiveStep : public Module {
        protected:
            /// Name of the roots whose members answer, a list of individuals.
            std::string roots;

            /// Name of the metrics from which inputs are sampled, a metrics map.
            std::string metrics;

            /// Name of the updated archive, an Evaluation::DataSet.
            std::string archive;

            /// Maximum number of samples stored in the archive.
            size_t archiveSize;

            /// Indicates if the inputs of the archive are fixed after the initialization.
            bool isArchiveFixed;

            /**
             * \brief Compute the answer of members on inputs.
             *
             * \param[in] inputs the inputs of the members.
             * \param[in] members the members answering.
             * \return an array containing the answer of each member.
             */
            static Data::DataValue computeMemberAnswer(const std::vector<Data::DataValue>& inputs, const std::vector<std::shared_ptr<const Individual>>& members);

            /// Sample new inputs from the metrics of the roots, and compute the answers of the members.
            void updateArchive(EvolutionData& state, Evaluation::DataSet& archive) const;

            /// Update the answers of the members on the inputs already in the archive.
            void updateArchiveOutputs(EvolutionData& state, Evaluation::DataSet& archive) const;

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] roots name of the roots whose members answer, a list of individuals.
             * \param[in] metrics name of the metrics from which inputs are sampled, a metrics map. Only the metrics of the roots are used.
             * \param[in] archive name of the updated archive, created by a CreateArchiveStep.
             * \param[in] archiveSize maximum number of samples stored in the archive.
             * \param[in] isArchiveFixed indicates if the inputs of the archive are fixed after the initialization.
             * \param[in] name name of the step.
             */
            UpdateArchiveStep(std::string roots, std::string metrics, std::string archive, size_t archiveSize, bool isArchiveFixed, std::string name = "UpdateArchive")
                : Module(std::move(name)), roots{std::move(roots)}, metrics{std::move(metrics)}, archive{std::move(archive)},
                  archiveSize{archiveSize}, isArchiveFixed{isArchiveFixed}
            {
                this->require(this->roots);
                this->require(this->metrics);
                this->require(this->archive);
                this->provide(this->archive);
            }

            /// Inherited from Module
            virtual std::string details() const override;
            /// Inherited from Module
            virtual void apply(EvolutionData& state) override;
    };

    /**
     * \brief Step removing from the roots of TPG graphs the vertices referenced by other roots.
     *
     * A vertex referenced by a tangled connection of a root is not a root anymore: it is protected, and removed
     * from the roots as long as one of its aggregators is still reachable from the roots. When its last aggregator
     * disappears, it comes back in the roots.
     *
     * The protected vertices and their aggregators are data of the state, created empty when the step runs
     * during the initialization. The step keeps nothing, so the evolution can be resumed from its state.
     */
    class ProtectAggregatedStep : public Module {
        protected:
            /// Name of the roots, a list of individuals.
            std::string roots;

            /// Name of the protected vertices, indexed by ID.
            std::string protectedVertices;

            /// Name of the IDs of the aggregators of each protected vertex.
            std::string aggregators;

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] roots name of the roots, a list of individuals (e.g. the individuals of a population).
             * \param[in] protectedVertices name of the protected vertices, created by the step during the initialization.
             * \param[in] aggregators name of the aggregators of the protected vertices, created by the step during the initialization.
             * \param[in] name name of the step.
             */
            ProtectAggregatedStep(std::string roots, std::string protectedVertices,
                                  std::string aggregators, std::string name = "ProtectAggregated")
                : Module(std::move(name)), roots{std::move(roots)}, protectedVertices{std::move(protectedVertices)}, aggregators{std::move(aggregators)}
            {
                this->require(this->roots);
                this->provide(this->roots);
                this->provide(this->protectedVertices);
                this->provide(this->aggregators);
            }

            /// Inherited from Module
            virtual void apply(EvolutionData& state) override;
    };

}; // namespace Evolution

#endif // EVOLUTION_STEPS_TPG_STEPS_H
