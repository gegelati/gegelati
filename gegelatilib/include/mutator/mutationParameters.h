/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2023) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2023)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

#ifndef MUTATION_PARAMETERS_H
#define MUTATION_PARAMETERS_H

#include <cstddef>
#include <cstdint>

namespace Mutator {
    /**
     * \brief Structure holding all parameters affecting mutations of TPGGraph.
     */
    typedef struct TPGParameters
    {

        /// JSon comment
        inline static const std::string nbRootsComment =
            "// Number of root TPGTeams to maintain when populating the "
            "TPGGraph\n"
            "// \"nbRoots\" : 100, // Default value";
        /// Number of root TPGTeams to maintain when populating the TPGGraph
        size_t nbRoots = 100;

        /// JSon comment
        inline static const std::string initNbRootsComment =
            "// Number of root TPGTeams at the initialisation of a "
            "TPGGraph.\n"
            "// If 0, if will be init to the number of surviving roots\n"
            "// \"nbRoots\" : 0, // Default value";
        /// Number of root TPGTeams at the initialisation of a TPGGraph
        /// If 0, if will be init to the number of surviving roots
        size_t initNbRoots = 0;

        /// JSon comment
        inline static const std::string maxInitOutgoingEdgesComment =
            "// Maximum number of TPGEdge connected to each TPGTeam of the "
            "TPGGraph when\n"
            "// initialized.\n"
            "// \"maxInitOutgoingEdges\" : 3, // Default value";
        /// Maximum number of TPGEdge connected to each TPGTeam of the TPGGraph
        /// when initialized.
        size_t maxInitOutgoingEdges = 3;

        /// JSon comment
        inline static const std::string maxOutgoingEdgesComment =
            "// Maximum number of outgoing edge during TPGGraph mutations.\n"
            "// \"maxOutgoingEdges\" : 5, // Default value";
        /// Maximum number of outgoing edge during TPGGraph mutations.
        size_t maxOutgoingEdges = 5;

        /// JSon comment
        inline static const std::string pEdgeDeletionComment =
            "// Probability of deleting an outgoing Edge of a Team.\n"
            "// \"pEdgeDeletion\" : 0.7, // Default value";
        /// Probability of deleting an outgoing TPGEdge of a TPGTeam.
        double pEdgeDeletion = 0.7;

        /// JSon comment
        inline static const std::string pEdgeAdditionComment =
            "// Probability of adding an outgoing Edge to a Team.\n"
            "// \"pEdgeAddition\" : 0.7, // Default value";
        /// Probability of adding an outgoing TPGEdge to a TPGTeam.
        double pEdgeAddition = 0.7;

        /// JSon comment
        inline static const std::string pProgramMutationComment =
            "// Probability of mutating the Program of an outgoing Edge.\n"
            "// \"pProgramMutation\" : 0.2, // Default value";
        /// Probability of mutating the Program of an outgoing TPGEdge.
        double pProgramMutation = 0.2;

        /// JSon comment
        inline static const std::string
            forceProgramBehaviorChangeOnMutationComment =
                "// When a Program is mutated, makes sure its behavior is no "
                "longer the same.\n"
                "// \"forceProgramBehaviorChangeOnMutation\" : false, // "
                "Default value";
        /// When a Program is mutated, makes sure its behavior is no longer the
        /// same. (This possibility does not exists in Kelly's work, where only
        /// the archive is used for this purpose, which is far from 100%
        /// accurate.)
        bool forceProgramBehaviorChangeOnMutation = false;

        /// JSon comment
        inline static const std::string pEdgeDestinationChangeComment =
            "// Probability of changing the destination of an Edge.\n"
            "// \"pEdgeDestinationChange\" : 0.1, // Default value";
        /// Probability of changing the destination of a TPGEdge.
        double pEdgeDestinationChange = 0.1;

        /// JSon comment
        inline static const std::string pEdgeDestinationIsActionComment =
            "// Probability of the new destination of an Edge to be an "
            "Action.\n"
            "// \"pEdgeDestinationIsAction\" : 0.5, // Default value";
        /// Probability of the new destination of a TPGEdge to be a TPGAction.
        double pEdgeDestinationIsAction = 0.5;
    } TPGParameters;

    /**
     * \brief Structure holding all parameters affecting mutations of Program.
     */
    typedef struct ProgramParameters
    {
        /// JSon comment
        inline static const std::string maxProgramSizeComment =
            "// Maximum number of Line within the Program of the TPG.\n"
            "// \"maxProgramSize\" : 96, // Default value";
        /// Maximum number of Line within the Program of the TPGGraph.
        size_t maxProgramSize = 96;

        /// JSon comment
        inline static const std::string pDeleteComment =
            "// Probability of deleting a line of the Program.\n"
            "// \"pDelete\" : 0.5, // Default value";
        /// Probability of deleting a line of the Program.
        double pDelete = 0.5;

        /// JSon comment
        inline static const std::string pAddComment =
            "// Probability of inserting a line in the Program.\n"
            "// \"pAdd\" : 0.5, // Default value";
        /// Probability of inserting a line in the Program
        double pAdd = 0.5;

        /// JSon comment
        inline static const std::string pMutateComment =
            "// Probability of altering a line of the Program.\n"
            "// \"pMutate\" : 1.0, // Default value";
        /// Probability of altering a line of the Program.
        double pMutate = 1.0;

        /// JSon comment
        inline static const std::string pSwapComment =
            "// Probability of swapping two lines of the Program.\n"
            "// \"pSwap\" : 1.0, // Default value";
        /// Probability of swapping two lines of the Program.
        double pSwap = 1.0;

        /// JSon comment
        inline static const std::string pConstantMutationComment =
            "// Probability of each constant to be mutated.\n"
            "// \"pConstantMutation\" : 0.5, // Default value";
        /// Probability of each constant to be mutated
        double pConstantMutation = 0.5;

        /// JSon comment
        inline static const std::string pNewProgramComment =
            "// Probability of creating a new program.\n"
            "// \"pNewProgram\" : 0.0, // Default value";
        /// Probability of creating a new program
        double pNewProgram = 0.0;

        /// JSon comment
        inline static const std::string minConstValueComment =
            "// Minimum constant value possible.\n"
            "// \"minConstValue\" : -10, // Default value";
        /// Minimum constant value possible
        int32_t minConstValue = -100;

        /// JSon comment
        inline static const std::string maxConstValueComment =
            "// Maximum constant value possible.\n"
            "// \"maxConstValue\" : 100, // Default value";
        /// Maximum constant value possible
        int32_t maxConstValue = 100;
    } ProgramParameters;

    /**
     * \brief Structure holding all parameters affecting stochastic mutations.
     */
    typedef struct MutationParameters
    {
        /// Parameters for TPGMutator
        TPGParameters tpg;
        /// Parameters for ProgramMutator
        ProgramParameters prog;
    } MutationParameters;
} // namespace Mutator

#endif
