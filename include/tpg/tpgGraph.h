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
		* \brief Empty the TPGGraph of all its content.
		*/
		void clear();

		/**
		* \brief Accessor to the Environment of the TPGGraph.
		*
		* \return the const reference to the env attribute.
		*/
		const Environment& getEnvironment() const;

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
		* method is called on the TPG. The returned vector is a copy of the
		* current set of vertices.
		*
		* \return a vector containing pointers to the vertices of the graph.
		*/
		const std::vector<const TPGVertex*> getVertices() const;

		/**
		* \brief Get the number of rootVertices of the TPGGraph.
		*
		* \return the number of TPGVertex in the graph with no incomingEdge.
		*/
		uint64_t getNbRootVertices() const;

		/**
		* \brief Get vector of const pointer to the root vertices of the TPGGraph.
		*
		* Content of the retrieved vector is valid only as long as no non-const
		* method is called on the TPG. The returned vector is a copy of the
		* current set of vertices.
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

		/**
		* Duplicate a TPGEdge from the TPGGraph.
		*
		* This method creates a perfecte copy of the given TPGEdge, that is
		* a TPGEdge with the same source, destination and program shared
		* pointer.
		*
		* \param[in] edge a const reference to the TPGedge to duplicate.
		* \return a const reference to the newly created TPGEdge.
		* \throw std::runtime_error if the given TPGEdge does not belong to
		* the TPGGraph.
		*/
		const TPGEdge& cloneEdge(const TPGEdge& edge);

		/**
		* \brief Change the destination of the Edge to the given target.
		*
		* Change the destination TPGVertex of a TPGEdge to a given TPGVertex.
		* This function updates the TPGEdge attributes as well as those of all
		* impacted TPGVertex.
		* \param[in] edge a const reference to the modified TPGEdge.
		* \param[in] newDest a const reference to the destination TPGVertex.
		* \return true if the given edge and vertex are part of the graph, and
		* the operation was successful, false otherwise.
		*/
		bool setEdgeDestination(const TPGEdge& edge, const TPGVertex& newDest);

		/**
		* \brief Change the source of the TPGEdge to the given vertex.
		*
		* Change the source TPGVertex of a TPGEdge to a given TPGVertex.
		* This function updates the TPGEdge attributes as well as those of all
		* impacted TPGVertex.
		* \param[in] edge a const reference to the modified TPGEdge.
		* \param[in] newDest a const reference to the new source TPGVertex.
		* \return true if the given edge and vertex are part of the graph, and
		* the operation was successful, false otherwise.
		*/
		bool setEdgeSource(const TPGEdge& edge, const TPGVertex& newSrc);

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

		/**
		* \brief Find the non-const iterator to an edge of the graph from
		* its const pointer.
		*
		* \param[in] edge the const pointer to the TPGEdge.
		* \return the iterator on the edges attribute, at the position of
		*         the searched edge pointer. If the given vertex pointer is
		*         not in the vertices, then vertices.end() is returned.
		*/
		std::list<TPGEdge>::iterator findEdge(const TPGEdge* vertex);
	};
};

#endif