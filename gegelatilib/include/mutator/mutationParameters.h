/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2023)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2024 - 2025)
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
#include <string>

namespace Mutator {
    /**
     * \brief Structure holding all parameters affecting mutations of Graph.
     */
    typedef struct TPGParameters
    {

        /// JSon comment
        inline static const std::string nbRootsComment =
            "// Number of root TPGTeams to maintain when populating the "
            "Graph\n"
            "// \"nbRoots\" : 100, // Default value";
        /// Number of root TPGTeams to maintain when populating the Graph
        size_t nbRoots = 100;

        /// JSon comment
        inline static const std::string ratioTeamsOverActionsComment =
            "// Ratio of teams roots over the number of actions roots.\n"
            "// If set to 1, there will be no action roots, if set to 0, there "
            "will be no team root.\n"
            "// \"ratioTeamsOverActions\" : 1.0, // Default value";
        /// Ratio of teams roots over the number of actions roots.
        /// If set to 1, there will be no action roots, if set to 0, there will
        /// be no team root.
        double ratioTeamsOverActions = 1.0;

        /// JSon comment
        inline static const std::string maxInitOutgoingEdgesComment =
            "// Maximum number of Edge connected to each TPGTeam of the "
            "Graph when\n"
            "// initialized.\n"
            "// \"maxInitOutgoingEdges\" : 3, // Default value";
        /// Maximum number of Edge connected to each TPGTeam of the Graph
        /// when initialized.
        size_t maxInitOutgoingEdges = 3;

        /// JSon comment
        inline static const std::string pChangeActionClassComment =
            "//  Probablity of changing the action edge by any other action "
            "edge in the graph\n"
            "// \"pChangeActionClass\" : 0.1, // Default value";
        ///  Probablity of changing the action edge by any other action edge in
        ///  the graph
        double pChangeActionClass = 0.1;
        /// JSon comment
        inline static const std::string pActionEdgeDeletionComment =
            "// Probability of deleting an outgoing ActionEdge of a Action.\n"
            "// \"pActionEdgeDeletion\" : 0.7, // Default value";
        /// Probability of deleting an outgoing Edge of a TPGTeam.
        double pActionEdgeDeletion = 0.7;

        /// JSon comment
        inline static const std::string pActionEdgeAdditionComment =
            "// Probability of adding an outgoing Action Edge to a Action.\n"
            "// \"pActionEdgeAddition\" : 0.7, // Default value";
        /// Probability of adding an outgoing Edge to a TPGTeam.
        double pActionEdgeAddition = 0.7;

        /// JSon comment
        inline static const std::string pMutateActionProgramComment =
            "//  Probablity of mutation the program on the action edge\n"
            "// \"pMutateActionProgram\" : 0.1, // Default value";
        ///  Probablity of mutation the program on the action edge
        double pMutateActionProgram = 0.1;

        /// JSon comment
        inline static const std::string pSwapActionProgramComment =
            "//  Probablity of swapping two action edge within the same action "
            "vertex\n"
            "// \"pSwapActionProgram\" : 0.1, // Default value";
        ///  Probablity of swapping two action edge within the same action
        ///  vertex
        double pSwapActionProgram = 0.1;

        /// JSon comment
        inline static const std::string nbActionEdgeInitComment =
            "// Number of edge per action vertex at initialisation\n"
            "// \"nbActionEdgeInit\" : 1, // Default value";
        /// Init each team with one edge per action
        size_t nbActionEdgeInit = 1;

        /// JSon comment
        inline static const std::string maxOutgoingEdgesComment =
            "// Maximum number of outgoing edge during Graph mutations.\n"
            "// \"maxOutgoingEdges\" : 5, // Default value";
        /// Maximum number of outgoing edge during Graph mutations.
        size_t maxOutgoingEdges = 5;

        /// JSon comment
        inline static const std::string pEdgeDeletionComment =
            "// Probability of deleting an outgoing Edge of a Team.\n"
            "// \"pEdgeDeletion\" : 0.7, // Default value";
        /// Probability of deleting an outgoing Edge of a TPGTeam.
        double pEdgeDeletion = 0.7;

        /// JSon comment
        inline static const std::string pEdgeAdditionComment =
            "// Probability of adding an outgoing Edge to a Team.\n"
            "// \"pEdgeAddition\" : 0.7, // Default value";
        /// Probability of adding an outgoing Edge to a TPGTeam.
        double pEdgeAddition = 0.7;

        /// JSon comment
        inline static const std::string pProgramMutationComment =
            "// Probability of mutating the Program of an outgoing Edge.\n"
            "// \"pProgramMutation\" : 0.2, // Default value";
        /// Probability of mutating the Program of an outgoing Edge.
        double pProgramMutation = 0.2;

        /// JSon comment
        inline static const std::string probaContextOverActionProgramComment =
            "// When an edge is mutate, the choice between action and context "
            "program is based on this\n"
            "// \"probaContextOverActionProgram\" : 0.5, // Default value";
        ///  When an edge is mutate, the choice between action and context
        ///  program is based on this
        double probaContextOverActionProgram = 0.5;

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
        /// Probability of changing the destination of a Edge.
        double pEdgeDestinationChange = 0.1;

        /// JSon comment
        inline static const std::string pEdgeDestinationIsActionComment =
            "// Probability of the new destination of an Edge to be an "
            "Action.\n"
            "// \"pEdgeDestinationIsAction\" : 0.5, // Default value";
        /// Probability of the new destination of a Edge to be a Action.
        double pEdgeDestinationIsAction = 0.5;

        /// JSon comment
        inline static const std::string pCrossAgentsComment =
            "// Probability of doing a crossover with two MAPLE agents (edge "
            "crossover by default).\n"
            "// \"pCrossAgents\" : 0.0, // Default value";
        /// Probability of doing a crossover with two MAPLE agents (edge
        /// crossover by default).
        double pCrossAgents = 0.0;

        /// JSon comment
        inline static const std::string pCrossProgramsComment =
            "// When doing a crossover with two MAPLE agents, probability to "
            "do a program crossover instead of an edge crossover.\n"
            "// \"pCrossPrograms\" : 0.0, // Default value";
        /// When doing a crossover with two MAPLE agents, probability to do a
        /// program crossover instead of an edge crossover.
        double pCrossPrograms = 0.0;

        /// JSon comment
        inline static const std::string useActionProgramComment =
            "// Create action program, instead of using context program to "
            "select continuous action(s)"
            ".\n"
            "// \"useActionProgram\" : false, // Default value";
        /// Create action program, instead of using context program to select
        /// continuous action(s)
        bool useActionProgram = false;

        /// JSon comment
        inline static const std::string useMultiActionProgramComment =
            "// Create multiple action program, instead of one outputting "
            "multiple action in a single program"
            ".\n"
            "// \"useMultiActionProgram\" : false, // Default value";
        /// Create multiple action program, instead of one outputting multiple
        /// action in a single program
        bool useMultiActionProgram = false;

        /// JSon comment
        inline static const std::string teamAccessAllActionsComment =
            "// During mutation of a team, setting this parameter to true "
            "allow a team\n"
            "//to change destination toward any other actions. If set to "
            "false, it can only\n"
            "//change destination toward a root action.\n"
            "// \"teamAccessAllActions\" : true, // Default value";
        /// During mutation of a team, setting this parameter to true allow a
        /// team to change destination toward any other actions. If set to
        /// false, it can only change destination toward a root action.
        bool teamAccessAllActions = true;

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
        /// Maximum number of Line within the Program of the Graph.
        size_t maxProgramSize = 96;

        /// JSon comment
        inline static const std::string initMinProgramSizeComment =
            "// Initial minimum number of Line within the Program of the "
            "Graph.\n"
            "// \"initMinProgramSize\" : 1, // Default value";
        /// Initial minimum number of Line within the Program of the Graph.
        size_t initMinProgramSize = 1;

        /// JSon comment
        inline static const std::string initMaxProgramSizeComment =
            "// Initial maximum number of Line within the Program of the "
            "Graph.\n"
            "// \"initMaxProgramSize\" : 10, // Default value";
        /// Initial maximum number of Line within the Program of the Graph.
        size_t initMaxProgramSize = 10;

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
            "// \"minConstValue\" : -1.0, // Default value";
        /// Minimum constant value possible
        double minConstValue = -1.0;

        /// JSon comment
        inline static const std::string maxConstValueComment =
            "// Maximum constant value possible.\n"
            "// \"maxConstValue\" : 1.0, // Default value";
        /// Maximum constant value possible
        double maxConstValue = 1.0;

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
