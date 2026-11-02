#ifndef EVOLUTION_ORIGIN_TPG_EVOLUTION_H
#define EVOLUTION_ORIGIN_TPG_EVOLUTION_H

#include <memory>
#include <string>
#include <vector>

#include "evolution/basicEvolution.h"
#include "evolution/originTPGSteps.h"
#include "evolution/steps/logStep.h"
#include "mutation/nodeTransferMutation.h"
#include "representations/TPG.h"

namespace Evolution {

    /**
     * \brief Settings of the evolution of Tangled Program Graphs the way the original TPG algorithm does, building the corresponding module.
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

        /// Name of the archive, a data of the population.
        std::string archiveName = "memberArchive";

        /// Indicates if the vertices referenced by others are removed from the population.
        bool protectAggregated = true;

        /// Name of the list of members whose outputs are cached, a data of the population.
        std::string cacheName = "memberOutputCache";

        /// Mutator of the vertices, NodeTransferMutation(0.1, 0.7, 0.7) if null.
        std::shared_ptr<Mutation::NodeTransferMutation> nodeTransfer = nullptr;

        /// Mutator of the members, PointMutator(0.2, 0.5, 0.5) if null.
        std::shared_ptr<Mutation::Mutator> memberMutation = nullptr;

        /**
         * \brief Inherited from BasicEvolution, adds the population of roots and the steps of the TPG evolution.
         *
         * \throw std::runtime_error if a required setting is missing, if the representation is not a TPG,
         * or if the representations and the problem are not compatible.
         */
        virtual void addTo(Module& module) const override;
    };

    /**
     * \brief Columns of LogStep showing the number of vertices and members of graphs.
     *
     * \param[in] roots name of the roots of the graphs (e.g. the individuals of a population).
     */
    std::vector<LogStep::Column> tpgLogColumns(const std::string& roots);

}; // namespace Evolution

#endif // EVOLUTION_ORIGIN_TPG_EVOLUTION_H
