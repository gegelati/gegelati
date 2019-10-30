#ifndef TPG_MUTATOR_H
#define TPG_MUTATOR_H

#include "mutator/mutationParameters.h"
#include "archive.h"
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
		* \throw std::runtime_error if maxInitOutgoingEdges exceeds nbAction.
		*        Or if nbAction is smaller than 1.
		*/
		void initRandomTPG(TPG::TPGGraph& graph, const MutationParameters& params);

		/**
		* \brief Select a random outgoingEdge of the given TPGTeam and removes
		* it from the TPGGraph.
		*
		* If there is a unique TPGAction among the destination of the available
		* outgoing TPGEdge of the TPGTeam, this edge will be avoided during the
		* random selection of the TPGEdge to remove.
		*
		* \param[in,out] graph the TPGGraph within which the team is stored.
		* \param[in] team the TPGTeam whose outgoingEdges will be altered.
		*/
		void removeRandomEdge(TPG::TPGGraph& graph, const TPG::TPGTeam& team);

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
		*/
		void addRandomEdge(TPG::TPGGraph& graph, const TPG::TPGTeam& team,
			const std::list<const TPG::TPGEdge*>& preExistingEdges);

		/**
		* \brief Change the destination of a TPGEdge to an randomly chosen
		* target.
		*
		* This function selects a random TPGVertex among given pre-existing
		* vector of TPGTeam and TPGAction.
		* If the edge is the only one within the team targetting a TPGAction,
		* then, the new destination will be a TPGAction also. Otherwise, the
		* function randomly choses between a TPGAction and a TPGTeam, with the
		* probabilities within the given MutationParameters.
		* No verification is made on the content of pre-existing TPGVertex 
		* list. If one of this list contains the team itself, a self-loop may 
		* be created. A TPGVertex not belonging to the graph in these lists
		* will cause an exception within the TPGGraph class tough.
		* If the current destination of the edge is among the candidates, the 
		* new destination may be the same as the old.
		*
		* \param[in,out] graph the TPGGraph within which the team and edge are
		*                stored.
		* \param[in] team the source TPGTeam of the edge.
		* \param[in] edge the TPGEdge whose destination will be altered.
		* \param[in] preExistingTeams the TPGTeam candidates for destination.
		* \param[in] preExistingActions the TPGAction candidates for
		*            destination.
		* \param[in] params Probability parameters for the mutation.
		*/
		void mutateEdgeDestination(TPG::TPGGraph& graph,
			const TPG::TPGTeam& team,
			const TPG::TPGEdge* edge,
			const std::vector<const TPG::TPGTeam*>& preExistingTeams,
			const std::vector<const TPG::TPGAction*>& preExistingActions,
			const Mutator::MutationParameters& params);

		/**
		* \brief Mutate the Program and the Destination of the given TPGEdge.
		*
		* This function mutates the behavior of the given TPGEdge Program, 
		* using the ProgramMutator functions, until the Program behavior is 
		* unique according to recordings held in the given Archive.
		* The Program mutation is applid systematically, and a call to 
		* MutateEdgeDestination is also made with a probability from the 
		* given MutationParameters.
		* 
		* \param[in,out] graph the TPGGraph within which the team and edge are
		*                stored.
		* \param[in] archive Archive used to assess the uniqueness of the 
		*            mutated Program behavior.
		* \param[in] team the source TPGTeam of the edge.
		* \param[in] edge the TPGEdge whose destination will be altered.
		* \param[in] preExistingTeams the TPGTeam candidates for destination.
		* \param[in] preExistingActions the TPGAction candidates for
		*            destination.
		* \param[in] params Probability parameters for the mutation.
		*/
		void mutateOutgoingEdge(TPG::TPGGraph& graph,
			const Archive& archive,
			const TPG::TPGTeam& team,
			const TPG::TPGEdge* edge,
			const std::vector<const TPG::TPGTeam*>& preExistingTeams,
			const std::vector<const TPG::TPGAction*>& preExistingActions,
			const Mutator::MutationParameters& params);

		/**
		* \brief Mutates a TPGTeam by stochastically adding, deleting, and 
		* mutating the Program and destination of outgoing TPGEdge.
		*
		* This function successively:
		* - removes outgoing TPGEdge from the TPGTeam,
		* - adds outgoing TPGEdge to the TPGTeam
		* - mutates the Program and destination of outgoing TPGEdge.
		* Probabilities in given MutationParameters are used to control
		* the application of previous mutations.
		* 
		* \param[in,out] graph the TPGGraph within which the team and edge are
		*                stored.
		* \param[in] archive Archive used to assess the uniqueness of the
		*            mutated Program behavior.
		* \param[in] team the source TPGTeam of the edge.
		* \param[in] edge the TPGEdge whose destination will be altered.
		* \param[in] preExistingTeams the TPGTeam candidates for destination.
		* \param[in] preExistingActions the TPGAction candidates for
		*            destination.
		* \param[in] params Probability parameters for the mutation.
		*/
		void mutateTPGTeam(TPG::TPGGraph& graph,
			const Archive& archive,
			const TPG::TPGTeam& team,
			const std::vector<const TPG::TPGTeam*>& preExistingTeams,
			const std::vector<const TPG::TPGAction*>& preExistingActions,
			const std::list<const TPG::TPGEdge*>& preExistingEdges,
			const Mutator::MutationParameters& params);

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
		*/
		void populateTPG(TPG::TPGGraph& graph, const Archive& archive, const Mutator::MutationParameters& params);
	};
};

#endif