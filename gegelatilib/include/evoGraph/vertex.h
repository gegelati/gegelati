/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
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

#ifndef TPG_VERTEX_H
#define TPG_VERTEX_H

#include "evoGraph/element.h"

struct CounterReset;

namespace EvoGraph {
    // Declare class to make it usable as an attribute.
    class Edge;

    /**
     * \brief Abstract class representing the vertices of a Graph
     */
    class Vertex : public Element
    {
      public:
        /// Default polymorphic destructor
        virtual ~Vertex() = default;

        /**
         * \brief Get a const reference to incoming edges of this Vertex.
         */
        const std::list<std::shared_ptr<const Edge>>& getIncomingEdges() const;

        /**
         * \brief Get a const reference to outgoing edges of this Vertex.
         */
        const std::list<std::shared_ptr<const Edge>>& getOutgoingEdges() const;

        /**
         * \brief Method to add an incoming Edge to the Vertex.
         *
         * Since the incomingEdges set is a std::set, the same Edge pointer
         * can not be added twice to the set.
         * This method does not register the Vertex as the destination of the
         * Edge.
         *
         * \param[in] edge the Edge pointer to be added to the incomingEdges
         *                 Set.
         */
        virtual void addIncomingEdge(std::shared_ptr<const Edge> edge);

        /**
         * \brief Removes the given incoming edge from the Vertex.
         *
         * If the given pointer is NULL or if the given edge is not in the
         * set of the Vertex, nothing happens. Otherwise, the edge is
         * removed.
         *
         * \param[in] edge the Edge to remove.
         */
        virtual void removeIncomingEdge(std::shared_ptr<const Edge> edge);

        /**
         * \brief Method to add an outgoing Edge to the Vertex.
         *
         * Since the outgoingEdges set is a std::set, the same Edge pointer
         * can not be added twice to the set.
         * This method does not register the Vertex as the spource of the
         * Edge.
         *
         * \param[in] edge the Edge pointer to be added to the outgoingEdges
         *                 Set.
         */
        virtual void addOutgoingEdge(std::shared_ptr<const Edge> edge);

        /**
         * \brief Removes the given outgoing edge from the Vertex.
         *
         * If the given pointer is NULL or if the given edge is not in the
         * set of the Vertex, nothing happens. Otherwise, the edge is
         * removed.
         *
         * \param[in] edge the Edge to remove.
         */
        virtual void removeOutgoingEdge(std::shared_ptr<const Edge> edge);

        /**
         * \brief return assessed actions
         */
        virtual const std::set<uint64_t>& getAssessedActions() const;

        /**
         * \brief Update the assessed actions
         */
        virtual void updateAssessedActions() = 0;

        /**
         * \brief compare the set given and the assessed actions of the vertex
         *
         * If the intersection is empty, return false, else true
         */
        virtual bool hasSameAssessedActions(std::set<uint64_t> actions) const;

        /**
         * \brief Get the unique identifier of the Vertex.
         *
         * \return the integer ID of the Vertex.
         */
        virtual uint64_t getVertexID() const;

        /**
         * \brief Set a new unique identifier to the Vertex.
         *
         * \param[in] newID the new integer ID to set to the Vertex.
         */
        virtual void setVertexID(uint64_t newID);

        /**
         * \brief Get the current value of the vertex ID counter.
         *
         * This method is mainly used for testing purpose to ensure that
         * vertex IDs are predictable.
         *
         * \return the current value of the vertex ID counter.
         */
        static uint64_t getVertexIDCounter();

      protected:
        /**
         * \brief Protected default constructor to forbid the instanciation of
         * object of this abstract class.
         * 
         * \param[in] agentProgram the weak pointer to the Agent Program associated to the
         *            Vertex.
         */
        Vertex(const std::weak_ptr<const Algorithm::Agent> agentProgram = std::weak_ptr<const Algorithm::Agent>()) : Element{agentProgram}, vertexID(incrementeCounter()){};

        /**
         * \brief Set of incoming Edge of the Vertex.
         */
        std::list<std::shared_ptr<const EvoGraph::Edge>> incomingEdges;

        /**
         * \brief Set of outgoing Edge of the Vertex.
         */
        std::list<std::shared_ptr<const EvoGraph::Edge>> outgoingEdges;

        /**
         * \brief Set of assessed actions by the team
         */
        std::set<uint64_t> assessedActions;

        /**
         * \brief Unique identifier of the Vertex.
         */
        uint64_t vertexID;

        /**
         * \brief Incremente the vertex ID counter and return the new value.
         */
        static uint64_t incrementeCounter();

        /**
         * \brief Reset the vertex ID counter.
         *
         * This method set the ID counter to a new value.
         * It can quickly lead to segmentation fault if not used carefully.
         */
        static void resetVertexIDCounter();
        friend struct ::CounterReset;
    };

    /**
     * \brief Comparison function to enable sorting of Vertex with
     * STL.
     */
    bool operator<(const Vertex& a, const Vertex& b);

}; // namespace EvoGraph

#endif
