#ifndef EVOLUTION_ORIGIN_TPG_EVOLUTION_H
#define EVOLUTION_ORIGIN_TPG_EVOLUTION_H

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "evaluation/dataset.h"
#include "evolution/basicEvolution.h"
#include "evolution/steps/logStep.h"
#include "mutation/nodeTransferMutation.h"
#include "representations/TPG.h"
#include "dimensions/dataValueGenerator.h"
#include "graphBased/genotypeGenerator.h"
#include "individual.h"
#include "mutation/mutator.h"

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
     * \brief Settings of the evolution of Tangled Program Graphs the way the original TPG algorithm does, building the corresponding steps.
     *
     * Vertices are TPG individuals (BasicEvolution::representation must be a Representations::TPG) bidding through members
     * of another representation (e.g. LGP). The flow is the one of BasicEvolution, with the following changes:
     *  - Vertices are mutated by a NodeTransferMutation transferring nodes of the current population, and the generator of the
     *    genotypes depends on the evolution: tangled connections towards any current vertex are proposed, and members are mutated
     *    with the member mutation. The range of the number of nodes is the one of the TPG representation, and new vertices start
     *    with the minimum. BasicEvolution::mutation is not used.
     *  - CacheMemberOutputs (after Mutation), only for prediction problems, executes each member once per sample.
     *  - ProtectAggregated (after TrainingFitness at initialization, after SurvivorSelection at each generation) removes from
     *    the population the vertices referenced by others.
     *  - If the archive size is positive, CreateArchive (first step of the initialization) creates an archive on which mutated
     *    members must answer differently from existing ones, and UpdateArchive (after the protection) maintains it from the inputs
     *    measured by the training evaluations. A fixed archive only measures the inputs during the evaluation of the initialization.
     */
    struct OriginTPGEvolution : public BasicEvolution {
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
         * Like for BasicEvolution::Names, give each evolution its own names to run several together.
         */
        struct TPGNames {
            /// Data: the archive of inputs and of the answers of the members.
            std::string archive = "memberArchive";

            /// Data: the members whose outputs are cached.
            std::string memberOutputCache = "memberOutputCache";

            /// Data: the vertices removed from the roots by the protection.
            std::string protectedVertices = "protectedVertices";

            /// Data: the aggregators of the protected vertices.
            std::string aggregators = "aggregators";

            /// Data: the generator of the genotypes of the vertices, built again at each generation.
            std::string generator = "generator";

            /// Step creating the archive.
            std::string createArchiveStep = "CreateArchive";

            /// Step caching the outputs of the members.
            std::string cacheMemberOutputsStep = "CacheMemberOutputs";

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
         * \brief Inherited from BasicEvolution, builds the module of the TPG evolution.
         *
         * Like for BasicEvolution, the settings are read first to create what the modules need, then the modules are built: the
         * initialization creates the archive and the vertices, a generation protects the aggregated vertices and updates the archive.
         *
         * \throw std::runtime_error if a required setting is missing, if the representation is not a TPG,
         * or if the representations and the problem are not compatible.
         */
        virtual std::shared_ptr<Module> build() const override;

        /// Inherited from BasicEvolution, adds the number of vertices and of members of the graphs of the population.
        virtual std::vector<LogStep::Column> logColumns() const override;

      protected:
        /**
         * \brief Throw if the settings of the TPG evolution are missing or not compatible.
         *
         * \return the representation of the vertices.
         */
        std::shared_ptr<const Representations::TPG> checkTPGSettings() const;
    };

}; // namespace Evolution

#endif // EVOLUTION_ORIGIN_TPG_EVOLUTION_H
