/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2022) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
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

#include "environment.h"
#include "tpg/tpgAction.h"
#include "tpg/tpgEdge.h"
#include "tpg/tpgFactory.h"
#include "tpg/tpgTeam.h"
#include "tpg/tpgVertex.h"

namespace TPG {
    /**
     * \brief Class for storing a Tangled-Program-Graph.
     */
    class TPGGraph
    {
      public:
        /**
         * \brief Main TPGGraph constructor.
         *
         * \param[in] e the Environment for the TPGGraph.
         * \param[in] f the TPGFactory used to create the graph elements.
         */
        TPGGraph(const Environment& e,
                 std::unique_ptr<TPGFactory> f = std::make_unique<TPGFactory>())
            : env{e}, factory{std::move(f)} {};

        /**
         * \brief delete copy constructor
         */
        TPGGraph(const TPGGraph& model) = delete;

        /**
         * \brief TPGGraph move assignment operator
         *
         * \param[in] model the TPGGraph to copy
         */
        TPGGraph(TPGGraph&& model) noexcept : env{model.getEnvironment()}
        {
            swap(*this, model);
        }

        /**
         *	\brief Helper function for move constructor.
         *
         *	Swaps the TPGGraphs objects.
         */
        friend inline void swap(TPGGraph& a, TPGGraph& b)
        {
            using std::swap;
            swap(a.vertices, b.vertices);
            swap(a.edges, b.edges);
        }

        /**
         *	\brief assignement operator for class TPGGraph
         */
        TPGGraph& operator=(TPGGraph model);

        /**
         * \brief Destructor for the TPGGraph.
         *
         * Free the memory allocated for TPGVertices.
         */
        virtual ~TPGGraph();

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
         * \brief Get a reference to the TPGFactory of the TPGGraph.
         *
         * \return a reference to the TPGFactory.
         */
        const TPGFactory& getFactory() const;

        /**
         * \brief Create a new TPGTeam and add it to the vertices of the
         * TPGGraph.
         *
         * The new TPGTeam is added to the back of the vertices list.
         * The TPGTeam is created using the TPGFactory of the TPGGraph.
         *
         * \return a const reference to the newly created TPGTeam.
         */
        const TPGTeam& addNewTeam();

        /**
         * \brief Create a new TPGAction and add it to the vertices of the
         * TPGGraph.
         *
         * The new TPGAction is added to the back of the vertices list.
         * The TPGAction is created using the TPGFactory of the TPGGraph.
         *
         * \param[in] actionID the ID identifier to associate to the TPGAction.
         * \param[in] actionClass the Class identifier to associate to the TPGAction. 
         * Default value set to 0 for single action cases;
         * \return a const reference to the newly created TPGAction.
         */
        const TPGAction& addNewAction(uint64_t actionID, uint64_t actionClass=0);

        /**
         * \brief Get the number of TPGVertex contained in the TPGGraph.
         *
         * \return the size of the vertices attribute.
         */
        size_t getNbVertices() const;

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
         * \brief Get vector of const pointer to the root vertices of the
         * TPGGraph.
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
         * \brief Check whether a given vertex exists in the TPGGraph.
         *
         * \param[in] vertex the TPG::TPGVertex whose presence in the TPGGraph
         * is checked.
         * \return true if the vertex exists in the TPGGraph, false otherwise.
         */
        bool hasVertex(const TPG::TPGVertex& vertex) const;

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
        const TPGVertex& cloneVertex(const TPGVertex& vertex);

        /**
         * \brief Add a new TPGEdge to the TPGGraph.
         *
         * Add a new TPGEdge to the TPGGraph, between the two given TPGVertex
         * and associated with the given Program. The newly created TPGEdge is
         * inserted in the incoming and outgoing edges lists of the connected
         * TPGVertex.
         * The TPGEdge is created using the TPGFactory of the TPGGraph.
         *
         * \param[in] src the source TPGVertex of the newly created TPGEdge.
         * \param[in] dest the destination TPGVertex of the newly created
         *                TPGEdge.
         * \param[in] prog shared pointer to the Program associated to the newly
         *                 created TPGEdge.
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
        const std::list<std::unique_ptr<TPGEdge>>& getEdges() const;

        /**
         * \brief Remove a TPGEdge from the TPGGraph.
         *
         * If the edge is connected to TPGVertex within the graph, they are
         * updated.
         *
         * \param[in] edge a const reference to the TPGEdge to remove.
         *
         * \throw std::runtime_error In case one of the TPGEdges does not
         *                           exist in the TPGGraph.
         */
        void removeEdge(const TPGEdge& edge);

        /**
         * Duplicate a TPGEdge from the TPGGraph.
         *
         * This method creates a perfect copy of the given TPGEdge, that is
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
         * \param[in] newSrc a const reference to the new source TPGVertex.
         * \return true if the given edge and vertex are part of the graph, and
         * the operation was successful, false otherwise.
         */
        bool setEdgeSource(const TPGEdge& edge, const TPGVertex& newSrc);

        /**
         * \brief Clear all intron instructions in the Program of the TPGGraph.
         *
         * This method scans all the Programs associated to the TPGEdge of the
         * TPGGraph and removes all intron instructions from them.
         */
        void clearProgramIntrons();

      protected:
        /// Environment of the TPGGraph
        const Environment& env;

        /// TPGFactory of the TPGGraph
        const std::unique_ptr<TPGFactory> factory;

        /**
         * \brief Set of TPGVertex composing the TPGGraph.
         */
        std::list<TPGVertex*> vertices;

        /**
         * \brief Set of TPGEdge composing the TPGGraph.
         */
        std::list<std::unique_ptr<TPGEdge>> edges;

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
        std::list<std::unique_ptr<TPGEdge>>::iterator findEdge(
            const TPGEdge* edge);
    };
}; // namespace TPG

#endif
