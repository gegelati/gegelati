#ifndef TPG_MUTATOR_H
#define TPG_MUTATOR_H

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
		* \throw std::runtime_error if maxInitOutgoingEdges exceeds nbAction.
		*        Or if nbAction is smaller than 1.
		*/
		void initRandomTPG(TPG::TPGGraph& graph, const MutationParameters& params);
	};
};

#endif