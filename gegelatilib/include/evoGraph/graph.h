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

#include "environment.h"
#include "evoGraph/action.h"
#include "evoGraph/edge.h"
#include "evoGraph/factory.h"
#include "evoGraph/team.h"
#include "evoGraph/vertex.h"
#include "util/genericComparator.h"

namespace EvoGraph {

    /**
     * \brief Class for storing a Tangled-Program-Graph.
     */
    class Graph
    {
      public:
        /**
         * \brief Main Graph constructor.
         *
         * \param[in] e the Environment for the Graph.
         * \param[in] f the GraphFactory used to create the graph elements.
         */
        Graph(const Environment& e,
                 std::unique_ptr<GraphFactory> f = std::make_unique<GraphFactory>())
            : env{e}, factory{std::move(f)}
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
        Graph(Graph&& model) noexcept : env{model.getEnvironment()}
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
         * \brief Accessor to the Environment of the Graph.
         *
         * \return the const reference to the env attribute.
         */
        const Environment& getEnvironment() const;

        /**
         * \brief Get a reference to the GraphFactory of the Graph.
         *
         * \return a reference to the GraphFactory.
         */
        const GraphFactory& getFactory() const;

        /**
         * \brief Create a new TPGTeam and add it to the vertices of the
         * Graph.
         *
         * The new TPGTeam is added to the back of the vertices list.
         * The TPGTeam is created using the GraphFactory of the Graph.
         *
         * \return a const reference to the newly created TPGTeam.
         */
        const TPGTeam& addNewTeam();

        /**
         * \brief Create a new Action and add it to the vertices of the
         * Graph.
         *
         * The new Action is added to the back of the vertices list.
         * The Action is created using the GraphFactory of the Graph.
         *
         * \param[in] actionID the identifier to associate to the Action.
         * \return a const reference to the newly created Action.
         */
        const Action& addNewAction(uint64_t actionID);

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
        const std::vector<const Vertex*> getVertices() const;

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
        const std::vector<const Action*> getRootActions() const;
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
        const std::vector<const TPGTeam*> getRootTeams() const;

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
        const std::vector<const Vertex*> getRootVertices() const;

        /**
         * \brief Check whether a given vertex exists in the Graph.
         *
         * \param[in] vertex the EvoGraph::Vertex whose presence in the Graph
         * is checked.
         * \return true if the vertex exists in the Graph, false otherwise.
         */
        bool hasVertex(const EvoGraph::Vertex& vertex) const;

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
         * \brief Clone a Vertex of the graph and all its outgoing Edge.
         *
         * \param[in] vertex the const reference to the Vertex to clone.
         * \return a const reference to the new Vertex.
         * \throw std::runtime_error if the given vertex does not belong to the
         * Graph.
         */
        const Vertex& cloneVertex(const Vertex& vertex);

        /**
         * \brief Add a new Edge to the Graph.
         *
         * Add a new Edge to the Graph, between the two given Vertex
         * and associated with the given Program. The newly created Edge is
         * inserted in the incoming and outgoing edges lists of the connected
         * Vertex.
         * The Edge is created using the GraphFactory of the Graph.
         *
         * \param[in] src the source Vertex of the newly created Edge.
         * \param[in] dest the destination Vertex of the newly created
         *                Edge.
         * \param[in] prog shared pointer to the Program associated to the newly
         *                 created Edge.
         * \return a const reference to the created Edge.
         * \throw std::runtime_error In case one of the Vertex does not
         *                           exist in the Graph, or if the
         *							destination is a Action.
         */
        const Edge& addNewEdge(const Vertex& src, const Vertex& dest,
                                  const std::shared_ptr<Program::Program> prog);
        /**
         * \brief Add a new ActionEdge to the Graph.
         *
         * Add a new ActionEdge to the Graph, between the give vertex is
         * associated the given Program. The newly created Edge is
         * inserted in the outgoing edges list of the connected
         * Vertex.
         * The ActionEdge is created using the GraphFactory of the Graph.
         *
         * \param[in] src the source Vertex of the newly created Edge.
         * \param[in] prog shared pointer to the Program associated to the newly
         *                 created Edge.
         * \param[in] actionClass of the actionEdge
         * \return a const reference to the created Edge.
         * \throw std::runtime_error In case the Vertex does not
         * exist in the Graph, or if the source is a TPGTeam.
         */
        const Edge& addNewActionEdge(
            const Vertex& src, const std::shared_ptr<Program::Program> prog,
            uint64_t actionClass);

        /**
         * \brief Get a const reference to the edges of the Graph.
         *
         * \return a const reference to the edges attribute.
         */
        const std::set<std::unique_ptr<EvoGraph::Edge>, UniqueLess<EvoGraph::Edge>>&
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
         * \brief Remove a ActionEdge from the Graph.
         *
         * If the edge is connected to Vertex within the graph, they are
         * updated.
         *
         * \param[in] edge a const reference to the ActionEdge to remove.
         *
         * \throw std::runtime_error In case one of the Edges does not
         *                           exist in the Graph.
         */
        void removeActionEdge(const Edge& edge);

        /**
         * Duplicate a Edge from the Graph.
         *
         * This method creates a perfect copy of the given Edge, that is
         * a Edge with the same source, destination and program shared
         * pointer.
         *
         * \param[in] edge a const reference to the TPGedge to duplicate.
         * \return a const reference to the newly created Edge.
         * \throw std::runtime_error if the given Edge does not belong to
         * the Graph.
         */
        const Edge& cloneEdge(const Edge& edge);

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
         * \brief Clear all intron instructions in the Program of the Graph.
         *
         * This method scans all the Programs associated to the Edge of the
         * Graph and removes all intron instructions from them.
         */
        void clearProgramIntrons();

        /**
         * \brief set a new action class to a ActionEdge
         *
         * \param[in] edge ActionEdge changed
         * \param[in] newActionClass new action class
         */
        void setActionClassEdge(const Edge* edge, uint64_t newActionClass);

        /**
         * Update the assessed actions of the current vertex, and all the
         * incomming edges
         *
         * Used only for continuous cases with multi-action program per
         * Action.
         *
         * Assessed actions represent the set of action class used by the
         * vertex.
         *
         * \param[in] vertex Vertex to order
         */
        void updateAssessedActions(const EvoGraph::Vertex* vertex);

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
         * Order the ActionEdge of the given action
         *
         * \param[in] action Action to order
         */
        void orderActionEdges(const EvoGraph::Action* action);

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
        /// Environment of the Graph
        const Environment& env;

        /// GraphFactory of the Graph
        const std::unique_ptr<GraphFactory> factory;

        /**
         * \brief Set of all edges currently used in the graph.
         */
        std::set<std::unique_ptr<Edge>, UniqueLess<Edge>> edges;

        /**
         * \brief Set of all vertices currently used in the graph.
         */
        std::set<std::unique_ptr<Vertex>, UniqueLess<Vertex>> vertices;
    };
}; // namespace EvoGraph

#endif
