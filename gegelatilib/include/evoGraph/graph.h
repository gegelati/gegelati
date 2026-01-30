/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2024 - 2025)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

#ifndef TPG_GRAPH_H
#define TPG_GRAPH_H

#include <list>
#include <set>

#include "evoGraph/action.h"
#include "evoGraph/edge.h"
#include "evoGraph/factory.h"
#include "evoGraph/team.h"
#include "evoGraph/vertex.h"
#include "evoGraph/element.h"
#include "util/genericComparator.h"

namespace EvoGraph {

    /**
     * \brief Class for storing a Evolutinary Graph.
     */
    class Graph
    {
      public:
        /**
         * \brief Main Graph constructor.
         *
         * \param[in] f the GraphFactory used to create the graph elements.
         */
        Graph(std::unique_ptr<GraphFactory> f = std::make_unique<GraphFactory>())
            : factory{std::move(f)}
        {
        }

        /**
         * \brief delete copy constructor
         */
        Graph(const Graph& model) = delete;

        /**
         * \brief Graph move assignment operator
         *
         * \param[in] model the Graph to copy
         */
        Graph(Graph&& model) noexcept
        {
            swap(*this, model);
        }

        /**
         *	\brief Helper function for move constructor.
         *
         *	Swaps the Graphs objects.
         */
        friend inline void swap(Graph& a, Graph& b)
        {
            using std::swap;
            swap(a.vertices, b.vertices);
            swap(a.edges, b.edges);
        }

        /**
         *	\brief assignement operator for class Graph
         */
        Graph& operator=(Graph model);

        /**
         * \brief Destructor for the Graph.
         *
         * Free the memory allocated for TPGVertices.
         */
        virtual ~Graph();

        /**
         * \brief Empty the Graph of all its content.
         */
        void clear();

        /**
         * \brief Get a reference to the GraphFactory of the Graph.
         *
         * \return a reference to the GraphFactory.
         */
        const GraphFactory& getFactory() const;

        /**
         * \brief Create a new Team and add it to the vertices of the
         * Graph.
         *
         * The new Team is added to the back of the vertices list.
         * The Team is created using the GraphFactory of the Graph.
         * 
         * \param[in] programAgent shared pointer to the Agent Program associated to the newly
         *                 created Team.
         *
         * \return a const reference to the newly created Team.
         */
        std::shared_ptr<const EvoGraph::Team> addNewTeam(std::shared_ptr<const Algorithm::Agent> programAgent = nullptr);

        /**
         * \brief Create a new Action and add it to the vertices of the
         * Graph.
         *
         * The new Action is added to the back of the vertices list.
         * The Action is created using the GraphFactory of the Graph.
         *
         * \param[in] actionID the identifier to associate to the Action.
         * \param[in] programAgent shared pointer to the Agent Program associated to the newly
         *                 created Action.
         * 
         * \return a const reference to the newly created Action.
         */
        std::shared_ptr<const EvoGraph::Action> addNewAction(uint64_t actionID, std::shared_ptr<const Algorithm::Agent> programAgent = nullptr);


        /**
         * \brief Get the number of Vertex contained in the Graph.
         *
         * \return the size of the vertices attribute.
         */
        size_t getNbVertices() const;

        /**
         * \brief Get vector of const pointer to the vertices of the Graph.
         *
         * Content of the retrieved vector is valid only as long as no non-const
         * method is called on the TPG. The returned vector is a copy of the
         * current set of vertices.
         *
         * \return a vector containing pointers to the vertices of the graph.
         */
        const std::vector<std::shared_ptr<const Vertex>> getVertices() const;

        /**
         * \brief Get the number of rootVertices of the Graph.
         *
         * \return the number of Vertex in the graph with no incomingEdge.
         */
        uint64_t getNbRootVertices() const;

        /**
         * \brief Get vector of const pointer to the root actions of the
         * Graph.
         *
         * Content of the retrieved vector is valid only as long as no non-const
         * method is called on the TPG. The returned vector is the
         * current set of actions.
         *
         * \return a vector containing pointers to the root actions of the
         * graph.
         */
        const std::vector<std::shared_ptr<const Action>> getRootActions() const;

        /**
         * \brief Get vector of const pointer to the actions of the
         * Graph.
         */
        const std::vector<std::shared_ptr<const Action>> getActions() const;

        /**
         * \brief Get vector of const pointer to the root teams of the
         * Graph.
         *
         * Content of the retrieved vector is valid only as long as no non-const
         * method is called on the TPG. The returned vector is the
         * current set of teams.
         *
         * \return a vector containing pointers to the root teams of the
         * graph.
         */
        const std::vector<std::shared_ptr<const Team>> getRootTeams() const;

        /**
         * \brief Get vector of const pointer to the root vertices of the
         * Graph.
         *
         * Content of the retrieved vector is valid only as long as no non-const
         * method is called on the TPG. The returned vector is a copy of the
         * current set of vertices.
         *
         * \return a vector containing pointers to the root vertices of the
         * graph.
         */
        const std::vector<std::shared_ptr<const Vertex>> getRootVertices() const;

        /**
         * \brief Check whether a given vertex exists in the Graph.
         *
         * \param[in] vertex the EvoGraph::Vertex whose presence in the Graph
         * is checked.
         * \return true if the vertex exists in the Graph, false otherwise.
         */
        bool hasVertex(const EvoGraph::Vertex& vertex) const;

        /**
         * \brief Check whether a given edge exists in the Graph.
         *
         * \param[in] edge the EvoGraph::Edge whose presence in the Graph
         * is checked.
         * \return true if the edge exists in the Graph, false otherwise.
         */
        bool hasEdge(const EvoGraph::Edge& edge) const;

        /**
         * \brief Check whether a given element exists in the Graph.
         *
         * \param[in] element the EvoGraph::Element whose presence in the Graph
         * is checked.
         * \return true if the element exists in the Graph, false otherwise.
         */
        bool hasElement(const EvoGraph::Element& element) const;

        /**
         * \brief Remove a Vertex from the Graph and destroy it.
         *
         * If the edge is connected to Edges within the graph, they are also
         * removed and destroyed.
         *
         * \param[in] vertex a const reference to the Vertex to remove.
         */
        void removeVertex(const Vertex& vertex);


        /**
         * \brief Clone an element of the graph, either its a vertex or an edge.
         *
         * \param[in] element the const reference to the Element to clone.
         * \return a const reference to the new Element.
         * \throw std::runtime_error if the given element does not belong to the
         * Graph.
         */
        std::shared_ptr<const EvoGraph::Element> cloneElement(const Element& element);

        /**
         * \brief Clone a Vertex of the graph and all its outgoing Edge.
         *
         * \param[in] vertex the const reference to the Vertex to clone.
         * \return a const reference to the new Vertex.
         * \throw std::runtime_error if the given vertex does not belong to the
         * Graph.
         */
        std::shared_ptr<const EvoGraph::Vertex> cloneVertex(const Vertex& vertex);

        /**
         * \brief Add a new Edge to the Graph.
         *
         * Add a new Edge to the Graph, between the two given Vertex
         * and associated with the given Agent Program. The newly created Edge is
         * inserted in the incoming and outgoing edges lists of the connected
         * Vertex.
         * The Edge is created using the GraphFactory of the Graph.
         *
         * \param[in] src the source Vertex of the newly created Edge.
         * \param[in] dest the destination Vertex of the newly created
         *                Edge.
         * \param[in] programAgent shared pointer to the Agent Program associated to the newly
         *                 created Edge.
         * \return a const reference to the created Edge.
         * \throw std::runtime_error In case one of the Vertex does not
         *                           exist in the Graph, or if the
         *							destination is a Action.
         */
        std::shared_ptr<const EvoGraph::Edge> addNewEdge(const Vertex& src, const Vertex& dest,
                                  const std::shared_ptr<const Algorithm::Agent> programAgent);


        /**
         * \brief Get a const reference to the edges of the Graph.
         *
         * \return a const reference to the edges attribute.
         */
        const std::vector<std::shared_ptr<const EvoGraph::Edge>>
        getEdges() const;

        /**
         * \brief Remove a Edge from the Graph.
         *
         * If the edge is connected to Vertex within the graph, they are
         * updated.
         *
         * \param[in] edge a const reference to the Edge to remove.
         *
         * \throw std::runtime_error In case one of the Edges does not
         *                           exist in the Graph.
         */
        void removeEdge(const Edge& edge);

        /**
         * \brief Change the program agent of a Vertex
         * 
         * \param[in] vertex a const reference to the modified Vertex.
         * \param[in] programAgent shared pointer to the Agent Program to set to the vertex
         */
        void setVertexProgram(const Vertex& vertex, std::shared_ptr<const Algorithm::Agent> programAgent);

        /**
         * Duplicate a Edge from the Graph.
         *
         * This method creates a perfect copy of the given Edge, that is
         * a Edge with the same source, destination and agent program shared
         * pointer.
         *
         * \param[in] edge a const reference to the TPGedge to duplicate.
         * \return a const reference to the newly created Edge.
         * \throw std::runtime_error if the given Edge does not belong to
         * the Graph.
         */
        std::shared_ptr<const EvoGraph::Edge> cloneEdge(const Edge& edge);

        /**
         * \brief Change the destination of the Edge to the given target.
         *
         * Change the destination Vertex of a Edge to a given Vertex.
         * This function updates the Edge attributes as well as those of all
         * impacted Vertex.
         * \param[in] edge a const reference to the modified Edge.
         * \param[in] newDest a const reference to the destination Vertex.
         * \return true if the given edge and vertex are part of the graph, and
         * the operation was successful, false otherwise.
         */
        bool setEdgeDestination(const Edge& edge, const Vertex& newDest);

        /**
         * \brief Change the source of the Edge to the given vertex.
         *
         * Change the source Vertex of a Edge to a given Vertex.
         * This function updates the Edge attributes as well as those of all
         * impacted Vertex.
         * \param[in] edge a const reference to the modified Edge.
         * \param[in] newSrc a const reference to the new source Vertex.
         * \return true if the given edge and vertex are part of the graph, and
         * the operation was successful, false otherwise.
         */
        bool setEdgeSource(const Edge& edge, const Vertex& newSrc);

        /**
         * \brief set a new program agent to a Edge
         * 
         * \param[in] edge a const reference to the modified Edge.
         * \param[in] programAgent shared pointer to the Agent Program to set to the edge.
         * \return true if the given edge is part of the graph, and
         */
        bool setEdgeProgram(const Edge& edge, std::shared_ptr<const Algorithm::Agent> programAgent);


        /**
         * Update the assessed actions of the current vertex, and all the
         * incomming edges
         *
         * Used only for continuous cases with multi-action per
         * Action.
         *
         * Assessed actions represent the set of action class used by the
         * vertex.
         *
         * \param[in] vertex Vertex to order
         */
        void updateAssessedActions(std::shared_ptr<const Vertex> vertex);

        /**
         * Update the assessed actions of the all graph.
         *
         * This is to be use only at the initialisation, the complexity could be
         * really high after
         *
         * This method basically execute "updateAssessedActions" for all the
         * Actions in the graph
         */
        void updateAllAssessedActions();

        /**
         * Order the Edge pointing to an action of the given team
         *
         * \param[in] team Team to order
         */
        void orderActionEdges(std::shared_ptr<const Team> team);

        /**
         * \brief Set a new ID to a vertex
         *
         * An error is thrown if the vertex does not belong to the graph
         * An error is thrown if the newID is already used
         *
         * \param[in] vertex the vertex to change ID
         * \param[in] newID the new ID to set
         */
        void setNewVertexID(const EvoGraph::Vertex& vertex, uint64_t newID);

        /**
         * \brief Set a new ID to an edge
         *
         * An error is thrown if the edge does not belong to the graph
         * An error is thrown if the newID is already used
         *
         * \param[in] edge the edge to change ID
         * \param[in] newID the new ID to set
         */
        void setNewEdgeID(const EvoGraph::Edge& edge, uint64_t newID);

      protected:

        /// GraphFactory of the Graph
        const std::unique_ptr<GraphFactory> factory;

        /**
         * \brief Set of all edges currently used in the graph.
         */
        std::set<std::shared_ptr<Edge>, SharedLess<Edge>> edges;

        /**
         * \brief Set of all vertices currently used in the graph.
         */
        std::set<std::shared_ptr<Vertex>, SharedLess<Vertex>> vertices;
    };
} // namespace EvoGraph

#endif
