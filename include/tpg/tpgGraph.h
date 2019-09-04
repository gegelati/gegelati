#ifndef TPG_GRAPH_H
#define TPG_GRAPH_H

#include <list>

#include "environment.h"
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
		* \brief Main TPGGraph constructor.
		*
		* \param[in] e the Environment for the TPGGraph.
		*/
		TPGGraph(const Environment& e) : env{ e } {};

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
		* The new TPGTeam is added to the back of the vertices list.
		*
		* \return a const reference to the newly created TPGTeam.
		*/
		const TPGTeam& addNewTeam();

		/**
		* \brief Create a new TPGAction and add it to the vertices of the
		* TPGGraph.
		*
		* The new TPGAction is added to the back of the vertices list.
		*
		* \param[in] actioID the identifier to associate to the TPGAction.
		* \return a const reference to the newly created TPGAction.
		*/
		const TPGAction& addNewAction(uint64_t actionID);

		/**
		* \brief Get vector of const pointer to the vertices of the TPGGraph.
		*
		* Content of the retrieved vector is valid only as long as no non-const
		* method is called on the TPG.
		*
		* \return a vector containing pointers to the vertices of the graph.
		*/
		const std::vector<const TPGVertex*> getVertices() const;

		/**
		* \brief Get vector of const pointer to the root vertices of the TPGGraph.
		*
		* Content of the retrieved vector is valid only as long as no non-const
		* method is called on the TPG.
		*
		* \return a vector containing pointers to the root vertices of the
		* graph.
		*/
		const std::vector<const TPGVertex*> getRootVertices() const;

		/**
		* \brief Remove a TPGVertex from the TPGGraph and destroy it.
		*
		* If the edge is connected to TPGEdges within the graph, they are also
		* removed and destroyed.
		*
		* \param[in] vertex a const reference to the TPGVertex to remove.
		*/
		void removeVertex(const TPGVertex& vertex);

		/**
		* \brief Clone a TPGVertex of the graph and all its outgoing TPGEdge.
		*
		* \param[in] vertex the const reference to the TPGVertex to clone.
		* \return a const reference to the new TPGVertex.
		* \throw std::runtime_error if the given vertex does not belong to the
		* TPGGraph.
		*/
		const TPGVertex& cloneVertex(const TPGVertex&);

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
		* \brief Get a const reference to the edges of the TPGGraph.
		*
		* \return a const reference to the edges attribute.
		*/
		const std::list<TPGEdge>& getEdges() const;

		/**
		* \brief Remove a TPGEdge from the TPGGraph.
		*
		* If the edge is connected to TPGVertex within the graph, they are
		* updated.
		*
		* \param[in] edge a const reference to the TPGEdge to remove.
		*/
		void removeEdge(const TPGEdge& edge);

	protected:

		/// Environment of the TPGGraph
		const Environment& env;

		/**
		* \brief Set of TPGVertex composing the TPGGraph.
		*/
		std::list<TPGVertex*> vertices;

		/**
		* \brief Set of TPGEdge composing the TPGGraph.
		*/
		std::list<TPGEdge> edges;

		/**
		* \brief Find the non-const iterator to a vertex of the graph from
		* its const pointer.
		*
		* \param[in] vertex the const pointer to the TPGVertex.
		* \return the iterator on the vertices attribute, at the position of
		*         the searched vertex pointer. If the given vertex pointer is
		*         not in the vertices, then vertices.end() is returned.
		*/
		std::list<TPGVertex*>::iterator findVertex(const TPGVertex* vertex);
	};
};

#endif