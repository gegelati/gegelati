#ifndef TPG_MUTATOR_H
#define TPG_MUTATOR_H

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
		* Hence, the maxInitOutgoingEdges value can not be greater than nbAction.
		*
		* \throw std::runtime_error if maxInitOutgoingEdges exceeds nbAction.
		*        Or if nbAction is smaller than 1.
		*/
		void initRandomTPG(TPG::TPGGraph& graph, size_t nbAction, size_t maxInitOutgoingEdges, size_t maxProgramSize);
	};
};

#endif