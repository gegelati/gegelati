#ifndef TPG_EXECUTION_ENGINE_H
#define TPG_EXECUTION_ENGINE_H

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
		* \brief TPGGraph executed by the Execution Engine.
		*/
		const TPGGraph& graph;

	public:
		/**
		* \brief Main constructor of the class.
		*
		* \param[in] tpgGraph a const reference to the TPGGraph to execute.
		*/
		TPGExecutionEngine(const TPGGraph& tpgGraph) : graph{ tpgGraph } {};

		/**
		* \brief Execute the Program associated to an Edge and returns the 
		* obtained double.
		* 
		* \param[in] edge the const ref to the TPGEdge whose Program will be 
		* evaluated.
		*/
		double evaluateEdge(const TPGEdge& edge);
	};
};

#endif