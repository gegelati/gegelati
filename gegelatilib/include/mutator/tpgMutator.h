/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2022) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
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

#ifndef TPG_MUTATOR_H
#define TPG_MUTATOR_H

#include <thread>

#include "archive.h"
#include "mutator/mutationParameters.h"
#include "tpg/tpgGraph.h"

namespace Mutator {
    namespace TPGMutator {
        /**
         * \brief Initialize a random TPGGraph.
         *
         * Following Stephen Kelly's PhD Thesis, the created TPGGraph will
         * contain:
         * - Exactly nbAction TPGAction vertices.
         * - Exactly nbAction TPGTeam vertices
         * - Exactly 2*nbAction Programs
         * - Between 2 and maxInitOutgoingEdges TPGEdge per TPGTeam, where
         *   - Each TPGEdge connects a TPGTeam with a TPGAction.
         *   - Each TPGTeam is connected to a TPGAction at most once.
         *   - Each TPGTeam is connected to at least 2 distinct TPGAction
         *   - Each Program is used at most once per TPGTeam.
         *   - Each Program always leads to the same TPGAction.
         *   - Each Program is approximately used the same number of time.
         * Hence, the maxInitOutgoingEdges value can not be greater than
         * nbAction.
         *
         * If the TPGGraph is not empty, all its vertices (and hence all its
         * edges) are removed before initialization.
         *
         * \param[in,out] graph the initialized TPGGraph.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         * \param[in] vectActions vector of actions that will be usable for
         * interacting with this LearningEnviromnent.
         * \throw std::runtime_error if maxInitOutgoingEdges exceeds nbAction.
         *        Or if nbAction is smaller than 1.
         */
        void initRandomTPG(TPG::TPGGraph& graph,
                           const MutationParameters& params, Mutator::RNG& rng,
                           std::vector<uint64_t> vectActions);

        /**
         * \brief Select a random outgoingEdge of the given TPGTeam and removes
         * it from the TPGGraph.
         *
         * \param[in,out] graph the TPGGraph within which the team is stored.
         * \param[in] team the TPGTeam whose outgoingEdges will be altered.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        void removeRandomEdge(TPG::TPGGraph& graph, const TPG::TPGTeam& team,
                              Mutator::RNG& rng);

        /**
         * \brief Add a new outgoing TPGEdge to the TPGTeam within the TPGGraph.
         *
         * This function adds a new outgoing TPGEdge to the TPGTeam by cloning
         * a preExisting TPGEdge of the TPGGraph. Since the graph may contain
         * TPGEdge from previous mutations, the function receives a list of
         * preExisting TPGEdge from which the TPGEdge to copy should be chosen
         * randomly. Any TPGEdge already connected to the TPGTeam is also
         * excluded from the candidates. If there is no valid TPGEdge candidate
         * this function will throw an exception (check code for more details).
         * The new TPGEdge will have the same destination TPGVertex and Program
         * as the cloned one, but its source will be the give TPGTeam.
         *
         * \param[in,out] graph the TPGGraph within which the team is stored.
         * \param[in] team the TPGTeam whose outgoingEdges will be altered.
         * \param[in] preExistingEdge the TPGEdge candidates for cloning.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        void addRandomEdge(
            TPG::TPGGraph& graph, const TPG::TPGTeam& team,
            const std::list<const TPG::TPGEdge*>& preExistingEdges,
            Mutator::RNG& rng);

        /**
         * \brief Change the destination of a TPGEdge to an randomly chosen
         * target.
         *
         * This function selects a random TPGVertex among given pre-existing
         * vector of TPGTeam and TPGAction.
         * The function randomly choses between a TPGAction and a TPGTeam, with
         * the probabilities within the given MutationParameters. No
         * verification is made on the content of pre-existing TPGVertex list.
         * If one of this list contains the team itself, a self-loop may be
         * created. A TPGVertex not belonging to the graph in these lists will
         * cause an exception within the TPGGraph class though. If the current
         * destination of the edge is among the candidates, the new destination
         * may be the same as the old.
         *
         * \param[in,out] graph the TPGGraph within which the team and edge are
         *                stored.
         * \param[in] edge the TPGEdge whose destination will be altered.
         * \param[in] preExistingTeams the TPGTeam candidates for destination.
         * \param[in] preExistingActions the TPGAction candidates for
         *            destination.
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        void mutateEdgeDestination(
            TPG::TPGGraph& graph, const TPG::TPGEdge* edge,
            const std::vector<const TPG::TPGTeam*>& preExistingTeams,
            const std::vector<const TPG::TPGAction*>& preExistingActions,
            const Mutator::MutationParameters& params, Mutator::RNG& rng);

        /**
         * \brief Prepares the mutation of a TPGEdge.
         *
         * This function creates a copy of the program associated to the TPGEdge
         * in preparation of its mutation. The methods also takes care of
         * stochastically mutating the destination of the TPGEdge. The copied
         * program are referenced in the newProgram list, and their behavior
         * must be mutated after this function to complete the mutation process.
         *
         * \param[in,out] graph the TPGGraph within which the team and edge are
         *                stored.
         * \param[in] edge the TPGEdge whose destination will be altered.
         * \param[in] preExistingTeams the TPGTeam candidates for destination.
         * \param[in] preExistingActions the TPGAction candidates for
         *            destination.
         * \param[in,out] newPrograms List of new Program created during
         *                mutations of the TPGTeam. The behavior of these
         *                Program must be mutated to complete the mutation
         *                process.
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        void mutateOutgoingEdge(
            TPG::TPGGraph& graph, const TPG::TPGEdge* edge,
            const std::vector<const TPG::TPGTeam*>& preExistingTeams,
            const std::vector<const TPG::TPGAction*>& preExistingActions,
            std::list<std::shared_ptr<Program::Program>>& newPrograms,
            const Mutator::MutationParameters& params, Mutator::RNG& rng);

        /**
         * \brief Mutates a TPGTeam by stochastically adding, deleting, and
         * mutating the Program and destination of outgoing TPGEdge.
         *
         * This function successively:
         * - removes outgoing TPGEdge from the TPGTeam,
         * - adds outgoing TPGEdge to the TPGTeam
         * - selects the Program whose behavior should be mutated, an mutates
         * destination of outgoing TPGEdge. Mutation of the Program behavior is
         * not performed in this function. Program to mutate are instead stored
         * in the newPrograms list for later mutation (potentially in parallel).
         * Probabilities in given MutationParameters are used to control
         * the application of previous mutations.
         *
         * \param[in,out] graph the TPGGraph within which the team and edge are
         *                stored.
         * \param[in] archive Archive used to assess the uniqueness of the
         *            mutated Program behavior.
         * \param[in] team the source TPGTeam of the edge.
         * \param[in] preExistingTeams the TPGTeam candidates for destination.
         * \param[in] preExistingActions the TPGAction candidates for
         *            destination.
         * \param[in,out] newPrograms List of new Program created during
         *                mutations of the TPGTeam. The behavior of these
         *                Program must be mutated to complete the mutation
         *                process.
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        void mutateTPGTeam(
            TPG::TPGGraph& graph, const Archive& archive,
            const TPG::TPGTeam& team,
            const std::vector<const TPG::TPGTeam*>& preExistingTeams,
            const std::vector<const TPG::TPGAction*>& preExistingActions,
            const std::list<const TPG::TPGEdge*>& preExistingEdges,
            std::list<std::shared_ptr<Program::Program>>& newPrograms,
            const Mutator::MutationParameters& params, Mutator::RNG& rng);

        /**
         * \brief Mutate the behavior of a Program and ensure its unicity
         * against the given Archive.
         *
         * \param[in,out] newProg Program whose behavior is being mutated.
         * \param[in] params Probability parameters for the mutation.
         * \param[in] archive Archive used to assess the uniqueness of the
         *            mutated Program behavior.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        void mutateProgramBehaviorAgainstArchive(
            std::shared_ptr<Program::Program>& newProg,
            const Mutator::MutationParameters& params, const Archive& archive,
            Mutator::RNG& rng);

        /**
         * \brief Function mutating the behavior of the given list of Program.
         *
         * \param[in] maxNbThreads Integer parameter controlling the number of
         *           threads used for parallel execution.Possible values are :
         *           -`0`and `1`: Do not use parallelism.
         *           -`n > 1`: Set the number of threads explicitly.
         * \param[in] newPrograms List of new Program to mutate.
         * \param[in] rng Random Number Generator used in the mutation process.
         * \param[in] params Probability parameters for the mutation.
         * \param[in] archive Archive used to assess the uniqueness of the
         * mutated Program behavior.
         */
        void mutateNewProgramBehaviors(
            const uint64_t& maxNbThreads,
            std::list<std::shared_ptr<Program::Program>>& newPrograms,
            Mutator::RNG& rng, const Mutator::MutationParameters& params,
            const Archive& archive);

        /**
         * \brief Create new root TPGTeam within the TPGGraph.
         *
         * This function create and add new root TPGTeam to the TPGGraph
         * until the targetted number of roots is reached. To create new root
         * TPGTeam, the function uses mutation operators on duplicates of
         * existing root TPGTeams of the TPGGraph.
         *
         * A few special cases are handled:
         * If the set of root vertices of the TPGGraph contains any TPGAction,
         * this TPGAction is ignored when selecting a candidate for duplication.
         * If the TPGGraph does not have any root TPGTeam, it is reinitialized
         * entirely with the initRandomTPG function.
         * If the given TPGGraph already has more root TPGVertex than the
         * targetted number of root teams, nothing happens.
         *
         * \param[in,out] graph the TPGGraph to mutate.
         * \param[in] archive Archive used to assess the uniqueness of the
         *            mutated Program behavior.
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         * \param[in] nbAction vector of actions that will be usable for
         * interacting with this LearningEnviromnent.
         * \param[in] maxNbThreads Integer parameter controlling the number of
         * threads used for parallel execution. Possible values are:
         *   - default:  Let the runtime decide using
         *               std::thread::hardware_concurrency().
         *   - `0` and `1`: Do not use parallelism.
         *   - `n > 1`: Set the number of threads explicitly.
         */
        void populateTPG(
            TPG::TPGGraph& graph, const Archive& archive,
            const Mutator::MutationParameters& params, Mutator::RNG& rng,
            std::vector<uint64_t> vectActions,
            uint64_t maxNbThreads = std::thread::hardware_concurrency());
    }; // namespace TPGMutator
};     // namespace Mutator

#endif
