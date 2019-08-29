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
		* \brief Destructor for the TPGGraph.
		*
		* Free the memory allocated for TPGVertices.
		*/
		~TPGGraph();

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
		* Get vector of const pointer to the vertices of the TPGGraph.
		*
		* Content of the retrieved vector is valid only as long as no non-const
		* method is called on the TPG. 
		*
		* \return a vector containing pointers to the vertices of the graph.
		*/
		const std::vector<const TPGVertex*> getVertices() const;

		/**
		* Remove a TPGVertex from the TPGGraph and destroy it.
		*
		* \param[in] vertex a const reference to the TPGVertex to remove.
		*/
		void removeVertex(const TPGVertex& vertex);

		/**
		* \brief Add a new TPGEdge to the TPGGraph.
		*
		* Add a new TPGEdge to the TPGGraph, between the two given TPGVertex
		* and associated with the given Program. The newly created TPGEdge is 
		* inserted in the incoming and outgoing edges lists of the connected
		* TPGVertex.
		*
		* \return a const reference to the created TPGEdge.
		* \throw std::runtime_error In case one of the TPGVertex does not 
		*                           exist in the TPGGraph, or if the 
		*							destination is a TPGAction. 
		*/
		const TPGEdge& addNewEdge(const TPGVertex& src, const TPGVertex& dest,
			const std::shared_ptr<Program::Program> prog);

		/**
		* Get a const reference to the edges of the TPGGraph.
		*
		* \return a const reference to the edges attribute.
		*/
		const std::list<TPGEdge>& getEdges() const;

	protected:
		/**
		* \brief Set of TPGVertex composing the TPGGraph.
		*/
		std::list<TPGVertex*> vertices;

		/**
		* \brief Set of TPGEdge composing the TPGGraph.
		*/
		std::list<TPGEdge> edges;
	};
};

#endif