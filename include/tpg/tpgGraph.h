#ifndef TPG_GRAPH_H
#define TPG_GRAPH_H

#include <list>

#include "tpg/tpgVertex.h"
#include "tpg/tpgTeam.h"
#include "tpg/tpgAction.h"
#include "tpg/tpgEdge.h"

namespace TPG {
	/**
	* \brief Class for storing a Tangled-Program-Graph.
	*/
	class TPGGraph {
	public:

		/**
		* \brief Create a new TPGTeam and add it to the vertices of the
		* TPGGraph.
		*
		* \return a const reference to the newly created TPGTeam.
		*/
		const TPGTeam& addNewTeam();

		/**
		* \brief Create a new TPGAction and add it to the vertices of the
		* TPGGraph.
		*
		* \return a const reference to the newly created TPGAction.
		*/
		const TPGAction& addNewAction();

		/**
		* Get a const reference to the vertices of the TPGGraph.
		*
		* \return a const reference to the vertices attribute.
		*/
		const std::list<TPGVertex>& getVertices() const;

		/**
		* Remove a TPGVertex from the TPGGraph and destroy it.
		*
		* \param[in] vertex a const reference to the TPGVertex to remove.
		*/
		void removeVertex(const TPGVertex& vertex);

	protected:
		/**
		* \brief Set of TPGVertex composing the TPGGraph.
		*/
		std::list<TPGVertex> vertices;

		/**
		* \brief Set of TPGEdge composing the TPGGraph.
		*/
		std::list<TPGEdge> edges;
	};
};

#endif