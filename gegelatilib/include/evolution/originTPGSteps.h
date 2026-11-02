#ifndef EVOLUTION_ORIGIN_TPG_STEPS_H
#define EVOLUTION_ORIGIN_TPG_STEPS_H

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "dimensions/requirement.h"
#include "evaluation/dataset.h"
#include "evaluation/evaluator.h"
#include "evaluation/predictionProblem.h"
#include "evolution/evolutionState.h"
#include "evolution/population.h"
#include "evolution/step.h"
#include "mutation/nodeTransferMutation.h"
#include "representations/TPG.h"

/**
 * Evolution of Tangled Program Graphs, helpers and steps of the original TPG algorithm (see OriginTPGEvolution).
 *
 * A TPG vertex is an individual whose nodes are (member, destination) pairs.
 * A member is an individual of another representation bidding for the destination, and a destination is either
 * an action or another vertex (a tangled connection). The population of a TPG evolution contains the roots of the graphs.
 */
namespace Evolution {

    /**
     * \brief Return the vertices directly referenced as destination by the nodes of a TPG vertex.
     *
     * \param[in] vertex the TPG vertex.
     * \return the referenced vertices, ordered by ID and without duplicates.
     */
    std::vector<std::shared_ptr<const Individual>> getSubVertices(const Individual& vertex);

    /**
     * \brief Return the roots and all the vertices reachable from them through tangled connections.
     *
     * \param[in] roots the roots of the graphs.
     * \return the vertices, ordered by ID and without duplicates.
     */
    std::vector<std::shared_ptr<const Individual>> getAllVertices(const std::vector<std::shared_ptr<const Individual>>& roots);

    /**
     * \brief Return the members used by the nodes of TPG vertices.
     *
     * \param[in] vertices the TPG vertices.
     * \return the members, ordered by ID and without duplicates.
     */
    std::vector<std::shared_ptr<const Individual>> getMembersOf(const std::vector<std::shared_ptr<const Individual>>& vertices);

    /**
     * \brief Step creating the random vertices of a population of roots.
     *
     * New vertices have the minimum number of nodes of the TPG representation and no tangled connection.
     * Their members are created with the member mutator, and must answer differently from existing ones on the archive
     * if a previous step created it (see CreateArchiveStep).
     */
    class InitVerticesStep : public Step {
        protected:
            /// Created population, its representation is a TPG.
            std::shared_ptr<const Population> population;

            /// Mutator creating the random genotypes of the vertices.
            std::shared_ptr<const Mutation::Mutator> mutator;

            /// Representation of the members.
            std::shared_ptr<const Representations::Representation> memberRepresentation;

            /// Mutator creating the random genotypes of the members.
            std::shared_ptr<const Mutation::Mutator> memberMutator;

            /// Name of the archive, an Evaluation::DataSet, used if it exists.
            std::string archive;

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] population created population, its representation must be a TPG.
             * \param[in] mutator mutator creating the random genotypes of the vertices.
             * \param[in] memberRepresentation representation of the members.
             * \param[in] memberMutator mutator creating the random genotypes of the members.
             * \param[in] archive name of the archive, an Evaluation::DataSet, used if it exists.
             * \param[in] name name of the step.
             * \throw std::runtime_error if a component is null or the representation of the population is not a TPG.
             */
            InitVerticesStep(std::shared_ptr<const Population> population, std::shared_ptr<const Mutation::Mutator> mutator,
                             std::shared_ptr<const Representations::Representation> memberRepresentation, std::shared_ptr<const Mutation::Mutator> memberMutator,
                             std::string archive, std::string name = "InitVertices");

            /// Inherited from Step
            virtual std::set<std::string> providedData() const override;
            /// Inherited from Step
            virtual std::string details() const override;
            /// Inherited from Step
            virtual void apply(EvolutionState& state) override;
    };

    /**
     * \brief Step mutating TPG vertices.
     *
     * Added nodes are copies of the nodes of the current roots: the step gives the roots to its NodeTransferMutation
     * before mutating, without copying the mutation. Mutated nodes may get a tangled connection towards any current vertex,
     * and their members are mutated with the member mutator (and must answer differently from existing ones on the archive,
     * if a previous step created it).
     */
    class MutateVerticesStep : public Step {
        protected:
            /// Name of the mutated vertices, a list of mutable individuals.
            std::string individuals;

            /// Name of the current roots, a list of individuals.
            std::string roots;

            /// Name of the archive, an Evaluation::DataSet, used if it exists.
            std::string archive;

            /// Representation of the vertices.
            std::shared_ptr<const Representations::TPG> representation;

            /// Mutation of the vertices, transferring the nodes of the roots.
            std::shared_ptr<Mutation::NodeTransferMutation> mutator;

            /// Representation of the members.
            std::shared_ptr<const Representations::Representation> memberRepresentation;

            /// Mutator of the members.
            std::shared_ptr<const Mutation::Mutator> memberMutator;

            /// Maximum number of threads used to mutate.
            size_t nbThreads;

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] individuals name of the mutated vertices, a list of mutable individuals (e.g. offspring).
             * \param[in] roots name of the current roots, a list of individuals (e.g. the individuals of a population).
             * \param[in] archive name of the archive, an Evaluation::DataSet, used if it exists.
             * \param[in] representation representation of the vertices.
             * \param[in] mutator mutation of the vertices, its potential nodes are set by the step.
             * \param[in] memberRepresentation representation of the members.
             * \param[in] memberMutator mutator of the members.
             * \param[in] nbThreads maximum number of threads used to mutate.
             * \param[in] name name of the step.
             * \throw std::runtime_error if a component is null.
             */
            MutateVerticesStep(std::string individuals, std::string roots, std::string archive, std::shared_ptr<const Representations::TPG> representation,
                               std::shared_ptr<Mutation::NodeTransferMutation> mutator, std::shared_ptr<const Representations::Representation> memberRepresentation,
                               std::shared_ptr<const Mutation::Mutator> memberMutator, size_t nbThreads = 1, std::string name = "MutateVertices");

            /// Inherited from Step
            virtual std::set<std::string> requiredData() const override;
            /// Inherited from Step
            virtual std::string details() const override;
            /// Inherited from Step
            virtual void apply(EvolutionState& state) override;
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
    class CacheMemberOutputsStep : public Step {
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

            /// Inherited from Step
            virtual std::set<std::string> requiredData() const override;
            /// Inherited from Step
            virtual std::set<std::string> providedData() const override;
            /// Inherited from Step
            virtual std::string details() const override;
            /// Inherited from Step
            virtual void apply(EvolutionState& state) override;
    };

    /**
     * \brief Step removing from a list of roots the vertices referenced by other vertices.
     *
     * A vertex referenced by a tangled connection of a root is not a root anymore: it is protected, and removed
     * from the roots as long as one of its aggregators is still reachable from the roots. When its last aggregator
     * disappears, it comes back in the roots.
     */
    class ProtectAggregatedStep : public Step {
        protected:
            /// Name of the roots, a list of individuals.
            std::string roots;

            /// Protected vertices, indexed by ID.
            std::map<uint64_t, std::shared_ptr<const Individual>> protectedVertices;

            /// IDs of the aggregators of each protected vertex.
            std::map<uint64_t, std::set<uint64_t>> aggregators;

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] roots name of the roots, a list of individuals (e.g. the individuals of a population).
             * \param[in] name name of the step.
             */
            explicit ProtectAggregatedStep(std::string roots, std::string name = "ProtectAggregated") : Step(std::move(name)), roots{std::move(roots)} {}

            /// Return the number of protected vertices.
            size_t getNbProtected() const;

            /// Inherited from Step
            virtual std::set<std::string> requiredData() const override;
            /// Inherited from Step
            virtual std::set<std::string> providedData() const override;
            /// Inherited from Step
            virtual void apply(EvolutionState& state) override;
    };

    /**
     * \brief Step creating an empty archive of inputs and of the answers of the members.
     *
     * It must be run at the beginning of the initialization, before the steps creating members from the archive.
     */
    class CreateArchiveStep : public Step {
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
                : Step(std::move(name)), archive{std::move(archive)}, inputRequirements{std::move(inputRequirements)} {}

            /// Inherited from Step
            virtual std::set<std::string> providedData() const override;
            /// Inherited from Step
            virtual void apply(EvolutionState& state) override;
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
    class UpdateArchiveStep : public Step {
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
            void updateArchive(EvolutionState& state, Evaluation::DataSet& archive) const;

            /// Update the answers of the members on the inputs already in the archive.
            void updateArchiveOutputs(EvolutionState& state, Evaluation::DataSet& archive) const;

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
                : Step(std::move(name)), roots{std::move(roots)}, metrics{std::move(metrics)}, archive{std::move(archive)},
                  archiveSize{archiveSize}, isArchiveFixed{isArchiveFixed} {}

            /// Inherited from Step
            virtual std::set<std::string> requiredData() const override;
            /// Inherited from Step
            virtual std::set<std::string> providedData() const override;
            /// Inherited from Step
            virtual std::string details() const override;
            /// Inherited from Step
            virtual void apply(EvolutionState& state) override;
    };

}; // namespace Evolution

#endif // EVOLUTION_ORIGIN_TPG_STEPS_H
