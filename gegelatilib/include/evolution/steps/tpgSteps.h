#ifndef EVOLUTION_STEPS_TPG_STEPS_H
#define EVOLUTION_STEPS_TPG_STEPS_H

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "dimensions/requirement.h"
#include "evaluation/dataset.h"
#include "evaluation/evaluate.h"
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
     * \brief Execute individuals once on the samples of an evaluation and cache their outputs in them (Individual::cacheOutput).
     *
     * Individuals whose own sub-individuals are already cached run faster, so callers order them accordingly.
     *
     * \param[in] individuals the individuals executed.
     * \param[in,out] problem the problem giving the samples.
     * \param[in] nbIterations number of samples of the evaluation.
     * \param[in] generation the current generation.
     * \param[in] mode evaluation mode, given to the problem.
     * \param[in] nbThreads maximum number of threads executing the individuals (see Evaluation::evaluateIndividuals).
     */
    void cacheOutputsOf(const std::vector<std::shared_ptr<const Individual>>& individuals, Evaluation::PredictionProblem& problem,
                        size_t nbIterations, uint64_t generation, Evaluation::Mode mode, size_t nbThreads);

    /**
     * \brief Gather the individuals of lists of the data.
     *
     * \param[in] state the data of the evolution.
     * \param[in] keys names of the lists of individuals (list of individuals or list of mutable individuals).
     */
    std::vector<std::shared_ptr<const Individual>> gatherIndividuals(const EvolutionData& state, const std::vector<std::string>& keys);

    /**
     * \brief Clear the outputs cached at the previous run of a caching step: they are for samples of a previous generation.
     *
     * \param[in] state the data of the evolution.
     * \param[in] cache name of the cached individuals, a list of individuals, ignored if it does not exist yet.
     */
    void clearPreviousCache(const EvolutionData& state, const std::string& cache);

    /**
     * \brief Step mutating TPG vertices with the generator found in the data.
     *
     * Added nodes are copies of the nodes of the current roots: the step gives the roots to its NodeTransferMutation
     * before mutating, without copying the mutation. The generator is a data of the evolution, built again from the current
     * vertices by a previous step (see OriginTPGEvolutionConfig).
     */
    class MutateVerticesStep : public Module {
        protected:
            /// Name of the mutated vertices, a list of mutable individuals.
            std::string individuals;

            /// Name of the current roots, a list of individuals.
            std::string roots;

            /// Name of the generator of the genotypes.
            std::string generator;

            /// Name of the mutation of the vertices, a component of the data (Mutation::Mutator, a Mutation::NodeTransferMutation).
            std::string mutator;

            /// Maximum number of threads used to mutate.
            size_t nbThreads;

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] individuals name of the mutated vertices, a list of mutable individuals (e.g. offspring).
             * \param[in] roots name of the current roots, a list of individuals (e.g. the individuals of a population).
             * \param[in] generator name of the generator of the genotypes.
             * \param[in] mutator name of the mutation of the vertices, stored in the data as a Mutation::Mutator that is a Mutation::NodeTransferMutation. Its potential nodes are set by the step.
             * \param[in] nbThreads maximum number of threads used to mutate.
             * \param[in] name name of the step.
             */
            MutateVerticesStep(std::string individuals, std::string roots, std::string generator,
                               std::string mutator, size_t nbThreads = 1, std::string name = "MutateVertices");

            /// Inherited from Module
            virtual std::string details() const override;
            /// Inherited from Module
            virtual void apply(EvolutionData& state) override;
    };

    /**
     * \brief Step building the generator of the genotypes of TPG vertices, stored in the data.
     *
     * The member of a node is created with the member mutator and, if there is an archive, must answer differently from the
     * existing members on it. Its destination is an action or, if the current vertices are given, sometimes one of them.
     * New vertices start with the minimum number of nodes of the TPG representation.
     */
    class UpdateGeneratorStep : public Module {
        protected:
            /// Name of the built generator, a GraphBased::GenotypeGenerator.
            std::string generator;

            /// Name of the representation of the vertices, a component of the data (const Representations::Representation, a Representations::TPG).
            std::string representation;

            /// Name of the representation of the members, a component of the data (const Representations::Representation).
            std::string memberRepresentation;

            /// Name of the mutator of the members, a component of the data (Mutation::Mutator).
            std::string memberMutator;

            /// Name of the archive (an Evaluation::DataSet), empty for none.
            std::string archive;

            /// Name of the roots whose vertices can be destinations (a list of mutable individuals), empty for actions only.
            std::string vertices;

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] generator name of the built generator.
             * \param[in] representation name of the representation of the vertices, a Representations::TPG.
             * \param[in] memberRepresentation name of the representation of the members.
             * \param[in] memberMutator name of the mutator of the members.
             * \param[in] archive name of the archive on which new members must answer differently, empty for none.
             * \param[in] vertices name of the roots whose vertices can be destinations, empty for actions only.
             * \param[in] name name of the step.
             */
            UpdateGeneratorStep(std::string generator, std::string representation, std::string memberRepresentation, std::string memberMutator,
                                std::string archive = "", std::string vertices = "", std::string name = "UpdateGenerator");

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

            /// Name of the problem, a component of the data (Evaluation::Problem, a Evaluation::PredictionProblem).
            std::string problem;

            /// Name of the representation of the members, a component of the data (const Representations::Representation).
            std::string memberRepresentation;

            /// Number of samples of the evaluation.
            size_t nbIterations;

            /// Evaluation mode, given to the problem: the outputs are cached for the samples of this mode.
            Evaluation::Mode mode;

            /// Maximum number of threads evaluating the individuals.
            size_t nbThreads;

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] evaluated names of the lists of individuals about to be evaluated (list of individuals or list of mutable individuals).
             * \param[in] cache name of the cached members, a list of individuals.
             * \param[in] problem name of the problem in which members are evaluated, stored in the data as an Evaluation::Problem that is a Evaluation::PredictionProblem.
             * \param[in] memberRepresentation name of the representation of the members, stored in the data.
             * \param[in] nbIterations number of samples of the evaluation.
             * \param[in] mode evaluation mode, given to the problem.
             * \param[in] nbThreads maximum number of threads evaluating the individuals (see Evaluation::evaluateIndividuals).
             * \param[in] name name of the step.
             */
            CacheMemberOutputsStep(std::vector<std::string> evaluated, std::string cache, std::string problem, std::string memberRepresentation, size_t nbIterations, Evaluation::Mode mode = Evaluation::Mode::TRAINING, size_t nbThreads = 1, std::string name = "CacheMemberOutputs");

            /// Inherited from Module
            virtual std::string details() const override;
            /// Inherited from Module
            virtual void apply(EvolutionData& state) override;
    };

    /**
     * \brief Step caching the outputs of the vertices shared by several individuals on the samples of a dataset.
     *
     * A vertex referenced by several vertices (or by a vertex and the evaluated individuals) would be executed several times
     * for the same sample. Its output, the action it leads to, is cached in the vertex (Individual::cacheOutput) after a single execution.
     * Vertices executed once per sample are not cached, since there would be no gain.
     *
     * Vertices are executed layer by layer, from the ones pointing only to actions to the ones with the highest sub-graphs
     * (see getSharedVerticesByHeight), so that cached vertices are used by the layers above and each vertex costs only the bids of its own nodes.
     * Ordering the vertices is linear in the number of vertices and edges, negligible compared to the executions on the samples.
     * Run CacheMemberOutputsStep first, so that bids are cached as well. This is only an optimization: results are unchanged.
     *
     * The cached vertices are a data of the state, and their outputs are cleared at the next run of the step.
     * Outputs are indexed by the address of the samples: the dataset of the problem must outlive the individuals.
     */
    class CacheVertexOutputsStep : public Module {
        protected:
            /// Names of the lists of individuals about to be evaluated.
            std::vector<std::string> evaluated;

            /// Name of the cached vertices, a list of individuals.
            std::string cache;

            /// Name of the problem, a component of the data (Evaluation::Problem, a Evaluation::PredictionProblem).
            std::string problem;

            /// Number of samples of the evaluation.
            size_t nbIterations;

            /// Evaluation mode, given to the problem: the outputs are cached for the samples of this mode.
            Evaluation::Mode mode;

            /// Maximum number of threads evaluating the individuals.
            size_t nbThreads;

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] evaluated names of the lists of individuals about to be evaluated (list of individuals or list of mutable individuals).
             * \param[in] cache name of the cached vertices, a list of individuals.
             * \param[in] problem name of the problem in which vertices are evaluated, stored in the data as an Evaluation::Problem that is a Evaluation::PredictionProblem.
             * \param[in] nbIterations number of samples of the evaluation.
             * \param[in] mode evaluation mode, given to the problem.
             * \param[in] nbThreads maximum number of threads evaluating the individuals (see Evaluation::evaluateIndividuals).
             * \param[in] name name of the step.
             */
            CacheVertexOutputsStep(std::vector<std::string> evaluated, std::string cache, std::string problem,
                                   size_t nbIterations, Evaluation::Mode mode = Evaluation::Mode::TRAINING, size_t nbThreads = 1,
                                   std::string name = "CacheVertexOutputs");

            /**
             * \brief Return the vertices executed several times per sample, grouped by height.
             *
             * The height of a vertex is 0 if it points only to actions, else one more than the highest of its sub-vertices.
             * A vertex is executed several times if it is evaluated and referenced, or referenced by several vertices.
             *
             * \param[in] evaluated the individuals about to be evaluated.
             * \return the layers of vertices, from height 0 up; a layer can be empty.
             * \throw std::runtime_error if the graph has a cycle.
             */
            static std::vector<std::vector<std::shared_ptr<const Individual>>> getSharedVerticesByHeight(const std::vector<std::shared_ptr<const Individual>>& evaluated);

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
     * during the initialization, and only the answers are updated afterwards.
     */
    class UpdateArchiveStep : public Module {
        protected:
            /// Name of the roots whose members answer, a list of individuals.
            std::string roots;

            /// Name of the metrics from which inputs are sampled, a metrics map.
            std::string metrics;

            /// Name of the updated archive, an Evaluation::DataSet.
            std::string archive;

            /// Name of the representation of the members, a component of the data (const Representations::Representation).
            std::string memberRepresentation;

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
             * \param[in] memberRepresentation name of the representation of the members, stored in the data.
             * \param[in] archiveSize maximum number of samples stored in the archive.
             * \param[in] isArchiveFixed indicates if the inputs of the archive are fixed after the initialization.
             * \param[in] name name of the step.
             */
            UpdateArchiveStep(std::string roots, std::string metrics, std::string archive, std::string memberRepresentation, size_t archiveSize, bool isArchiveFixed, std::string name = "UpdateArchive")
                : Module(std::move(name)), roots{std::move(roots)}, metrics{std::move(metrics)}, archive{std::move(archive)},
                  memberRepresentation{std::move(memberRepresentation)}, archiveSize{archiveSize}, isArchiveFixed{isArchiveFixed}
            {
                this->require(this->roots);
                this->require(this->metrics);
                this->require(this->archive);
                this->require(this->memberRepresentation);
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
