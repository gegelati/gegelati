#ifndef EVOLUTION_ORIGIN_TPG_EVOLUTION_H
#define EVOLUTION_ORIGIN_TPG_EVOLUTION_H

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "evaluation/dataset.h"
#include "evolution/basicEvolutionConfig.h"
#include "evolution/steps/logStep.h"
#include "mutation/nodeTransferMutation.h"
#include "representations/TPG.h"
#include "dimensions/dataValueGenerator.h"
#include "graphBased/genotypeGenerator.h"
#include "individual.h"
#include "mutation/mutator.h"

namespace Evolution {

    /**
     * \brief Return the vertices directly referenced by a TPG vertex: the referenced individuals of its representation (see Util::getMemberIndividuals).
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
     * \brief Return the members used by TPG vertices: the individuals of the member representation they reference.
     *
     * \param[in] vertices the TPG vertices.
     * \param[in] memberRepresentation the representation of the members.
     * \return the members, ordered by ID and without duplicates.
     */
    std::vector<std::shared_ptr<const Individual>> getMembersOf(const std::vector<std::shared_ptr<const Individual>>& vertices, const Representations::Representation& memberRepresentation);

    /**
     * \brief Settings of the evolution of Tangled Program Graphs the way the original TPG algorithm does, building the corresponding steps.
     *
     * Vertices are TPG individuals (BasicEvolutionConfig::representation must be a Representations::TPG) bidding through members
     * of another representation (e.g. LGP). The flow is the one of BasicEvolutionConfig, with the following changes:
     *  - Vertices are mutated by a NodeTransferMutation transferring nodes of the current population, and the generator of the
     *    genotypes depends on the evolution: tangled connections towards any current vertex are proposed, and members are mutated
     *    with the member mutation. The range of the number of nodes is the one of the TPG representation, and new vertices start
     *    with the minimum. BasicEvolutionConfig::mutation is replaced by the nodeTransfer.
     *  - CacheMemberOutputs (after Mutation), only for prediction problems, executes each member once per sample.
     *  - CacheVertexOutputs (after CacheMemberOutputs), only for prediction problems, executes once per sample each vertex shared
     *    by several individuals, layer by layer from the lowest vertices.
     *  - If the validation is used and its problem is a prediction problem, ValidationCacheMemberOutputs and
     *    ValidationCacheVertexOutputs do the same before the validation evaluation, with the same period, for the population.
     *  - ProtectAggregated (after TrainingFitness at initialization, after SurvivorSelection at each generation) removes from
     *    the population the vertices referenced by others.
     *  - If the archive size is positive, CreateArchive (first step of the initialization) creates an archive on which mutated
     *    members must answer differently from existing ones, and UpdateArchive (after the protection) maintains it from the inputs
     *    measured by the training evaluations. A fixed archive only samples them during the initialization.
     */
    struct OriginTPGEvolutionConfig : public BasicEvolutionConfig {
        /// Representation of the members, its output must be a double, required.
        std::shared_ptr<const Representations::Representation> memberRepresentation = nullptr;

        /// Maximum number of samples of the archive, zero to disable the archive.
        size_t archiveSize = 50;

        /// Indicates if the inputs of the archive are fixed after the initialization (e.g. for classification).
        bool isArchiveFixed = false;

        /// Indicates if the vertices referenced by others are removed from the population.
        bool protectAggregated = true;

        /// Mutator of the vertices, NodeTransferMutation(0.1, 0.7, 0.7) if null.
        std::shared_ptr<Mutation::NodeTransferMutation> nodeTransfer = nullptr;

        /// Mutator of the members, PointMutator(0.2, 0.5, 0.5) if null.
        std::shared_ptr<Mutation::Mutator> memberMutation = nullptr;

        /**
         * \brief Names of the data and of the steps added by the TPG evolution, all of them changeable before build.
         *
         * Like for BasicEvolutionConfig::Names, give each evolution its own names to run several together.
         */
        struct TPGNames {
            /// Data: the archive of inputs and of the answers of the members.
            std::string archive = "memberArchive";

            /// Data: the members whose outputs are cached.
            std::string memberOutputCache = "memberOutputCache";

            /// Data: the vertices whose outputs are cached.
            std::string vertexOutputCache = "vertexOutputCache";

            /// Data: the members whose outputs are cached for the validation.
            std::string validationMemberOutputCache = "validationMemberOutputCache";

            /// Data: the vertices whose outputs are cached for the validation.
            std::string validationVertexOutputCache = "validationVertexOutputCache";

            /// Data: the vertices removed from the roots by the protection.
            std::string protectedVertices = "protectedVertices";

            /// Data: the aggregators of the protected vertices.
            std::string aggregators = "aggregators";

            /// Step creating the archive.
            std::string createArchiveStep = "CreateArchive";

            /// Step caching the outputs of the members.
            std::string cacheMemberOutputsStep = "CacheMemberOutputs";

            /// Step caching the outputs of the vertices shared by several individuals.
            std::string cacheVertexOutputsStep = "CacheVertexOutputs";

            /// Steps caching the outputs of the members for the validation.
            std::string validationCacheMemberOutputsStep = "ValidationCacheMemberOutputs";
            /// Steps caching the outputs of the shared vertices for the validation.
            std::string validationCacheVertexOutputsStep = "ValidationCacheVertexOutputs";

            /// Step protecting the vertices referenced by others.
            std::string protectAggregatedStep = "ProtectAggregated";

            /// Step updating the archive.
            std::string updateArchiveStep = "UpdateArchive";

            /// Step building the generator of the genotypes of the vertices.
            std::string updateGeneratorStep = "UpdateGenerator";
        };

        /// Names of the data and of the steps added by the TPG evolution, changeable before build.
        TPGNames tpgNames;

        /**
         * \brief Inherited from BasicEvolutionConfig, builds the module of the TPG evolution.
         *
         * Like for BasicEvolutionConfig, the null components are replaced by the defaults and stored in the data of the module, then the
         * modules are built: the initialization creates the archive and the vertices, a generation protects the aggregated vertices and
         * updates the archive. The vertex mutation is stored under its name as a Mutation::Mutator, the member representation and mutation under their names.
         *
         * \throw std::runtime_error if a required setting is missing, if the representation is not a TPG,
         * or if the representations and the problem are not compatible.
         */
        virtual std::shared_ptr<Module> buildModule() override;

        /// Inherited from BasicEvolutionConfig, adds the number of vertices and of members of the graphs of the population.
        virtual std::shared_ptr<LogModule> buildLogModule() const override;

      protected:
        /**
         * \brief Inherited from BasicEvolutionConfig, also checks the settings of the TPG evolution and replaces its null components by the defaults.
         *
         * The mutation of the evolution is the nodeTransfer, which creates and mutates the vertices.
         *
         * \throw std::runtime_error if the settings are missing or not compatible.
         */
        virtual void updateEmptyComponents() override;

        /**
         * \brief Return the number of vertices of the graphs of a population (see getAllVertices).
         *
         * \param[in] state the data of the evolution.
         * \param[in] roots name of the roots of the graphs, the individuals of the population.
         */
        static double getNbVertices(const EvolutionData& state, const std::string& roots);

        /**
         * \brief Return the number of members used by the graphs of a population (see getMembersOf).
         *
         * \param[in] state the data of the evolution.
         * \param[in] roots name of the roots of the graphs, the individuals of the population.
         * \param[in] memberRepresentation name of the representation of the members, a component of the data.
         */
        static double getNbMembers(const EvolutionData& state, const std::string& roots, const std::string& memberRepresentation);

        /// Inherited from BasicEvolutionConfig, caches the outputs of the members and of the shared vertices before the validation evaluation (prediction problems only).
        virtual std::shared_ptr<Module> makeValidationModule(const std::string& name, size_t period) const override;
    };

}; // namespace Evolution

#endif // EVOLUTION_ORIGIN_TPG_EVOLUTION_H
