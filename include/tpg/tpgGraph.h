#ifndef TPG_GRAPH_H
#define TPG_GRAPH_H

#include <vector>

#include "tpg/tpgVertex.h"
#include "tpg/tpgEdge.h"

namespace TPG {
	/**
	* \brief Class for storing a Tangled-Program-Graph.
	*/
	class TPGGraph {
	protected:
		/**
		* \brief Set of TPGVertex composing the TPGGraph.
		*/
		std::vector<TPGVertex> vertices;

		/**
		* \brief Set of TPGEdge composing the TPGGraph.
		*/
		std::vector<TPGEdge> edges;
	};
};

#endif