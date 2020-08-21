/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019)
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

namespace Mutator {
    /**
     * \brief Structure holding all parameters affecting mutations of TPGGraph.
     */
    typedef struct TPGParameters
    {
        /// Number of TPGAction vertex of the initialized TPGGraph.
        size_t nbActions;
        /// Number of root TPGTeams to maintain when populating the TPGGraph
        size_t nbRoots;
        /// Maximum number of TPGEdge connected to each TPGTeam of the TPGGraph.
        size_t maxInitOutgoingEdges;
        /// Maximum number of outgoing edge during TPGGraph mutations0
        size_t maxOutgoingEdges;
        /// Probability of deleting an outgoing TPGEdge of a TPGTeam.
        double pEdgeDeletion;
        /// Probability of adding an outgoing TPGEdge to a TPGTeam.
        double pEdgeAddition;
        /// Probability of mutating the Program of an outgoing TPGEdge.
        double pProgramMutation;
        /// When a Program is mutated, makes sure its behavior is no longer the
        /// same. (This possibility does not exists in Kelly's work, where only
        /// the archive is used for this purpose, which is far from 100%
        /// accurate.)
        bool forceProgramBehaviorChangeOnMutation = false;
        /// Probability of changing the destination of a TPGEdge.
        double pEdgeDestinationChange;
        /// Probability of the new destination of a TPGEdge to be a TPGAction.
        double pEdgeDestinationIsAction;
    } TPGParameters;

    /**
     * \brief Structure holding all parameters affecting mutations of Program.
     */
    typedef struct ProgramParameters
    {
        /// Maximum number of Line within the Program of the TPGGraph.
        size_t maxProgramSize;
        /// Probability of deleting a line of the Program.
        double pDelete;
        /// Probability of inserting a line in the Program
        double pAdd;
        /// Probability of altering a line of the Program.
        double pMutate;
        /// Probability of swapping two lines of the Program.
        double pSwap;
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
