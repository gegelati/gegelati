#ifndef TPG_EXECUTION_ENGINE_H
#define TPG_EXECUTION_ENGINE_H

#include <set>
#include <vector>

#include "archive.h"

#include "tpgGraph.h"

namespace TPG {
	/**
	* Class in charge of executing a TPGGraph.
	*
	* This first implementation is purely sequential and does not parallelize
	* Program execution, nor executions of the TPG starting from several roots.
	*/
	class TPGExecutionEngine {
	protected:
		/**
		*  \brief Archive for recording Program results.
		*/
		Archive* archive;

	public:
		/**
		* \brief Main constructor of the class.
		*
		* \param[in] arch pointer to the Archive for storing recordings of
		*                 the Program Execution. By default, a NULL pointer is
		*                 given, meaning that no recording of the execution
		*                 will be made.
		*/
		TPGExecutionEngine(Archive* arch = NULL) : archive{ arch } {};

		/**
		* \brief Execute the Program associated to an Edge and returns the
		* obtained double.
		*
		* If an Archive is associated to the TPGExecutionEngine, the Program result
		* is recorded in it.
		*
		* \param[in] edge the const ref to the TPGEdge whose Program will be
		* evaluated.
		* \return the double value returned by the Program of the TPGEdge.
		*/
		double evaluateEdge(const TPGEdge& edge);

		/**
		* \brief Evaluate all the Program of the outgoing TPGEdge of the
		*        TPGTeam.
		*
		* This method evaluates the Programs of all outgoing TPGEdge of the
		* TPGTeam, and returns the reference to the TPGEdge providing the
		* largest evaluation.
		* TPGEdge leading to a TPGTeam in the excluded set will not be
		* evaluated.
		*
		* \param[in] team the TPGTeam whose outgoing TPGEdge are evaluated.
		* \param[in] excluded the TPGTeam pointers that must be avoided when
		*            TPGEdge lead to them.
		* \return the reference to the TPGEdge evaluated with the the highest
		*         double value (and not excluded).
		*
		* \throw std::runtime_error in case the TPGTeam has no outgoing edge
		*        after excluding all edges leading to TPGVertex from the
		*        excluded set. This should not happen in a correctly
		*        constructed TPGGraph where each TPGTeam must be connected to
		*        at least one TPGAction, to ensure that all cycles have an
		*        exit.
		*/
		const TPG::TPGEdge& evaluateTeam(const TPGTeam& team, const std::vector<const TPGVertex*>& excluded);

		/**
		* \brief Execute the TPGGraph starting from the given TPGVertex.
		*
		* This method browse the graph by successively evaluating Teams and
		* following the TPGEdge proposing the best bids.
		*
		* \param[in] root the TPGVertex from which the execution will start.
		* \return a vector containing all the TPGVertex traversed during the
		*         evaluation of the TPGGraph. The TPGAction resulting from the
		*         TPGGraph execution is at the end of the returned vector.
		*/
		const std::vector<const TPGVertex*> executeFromRoot(const TPGVertex& root);
	};
};

#endif