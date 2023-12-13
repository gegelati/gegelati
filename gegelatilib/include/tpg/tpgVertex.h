/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2022) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
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

#include <list>

namespace TPG {
    // Declare class to make it usable as an attribute.
    class TPGEdge;

    /**
     * \brief Abstract class representing the vertices of a TPGGraph
     */
    class TPGVertex
    {
      public:
        /// Default polymorphic destructor
        virtual ~TPGVertex() = default;

        /**
         * \brief Get a const reference to incoming edges of this TPGVertex.
         */
        const std::list<TPGEdge*>& getIncomingEdges() const;

        /**
         * \brief Get a const reference to outgoing edges of this TPGVertex.
         */
        const std::list<TPGEdge*>& getOutgoingEdges() const;

        /**
         * \brief Method to add an incoming TPGEdge to the TPGVertex.
         *
         * Since the incomingEdges set is a std::set, the same TPGEdge pointer
         * can not be added twice to the set.
         * This method does not register the TPGVertex as the destination of the
         * TPGEdge.
         *
         * \param[in] edge the TPGEdge pointer to be added to the incomingEdges
         *                 Set.
         */
        virtual void addIncomingEdge(TPG::TPGEdge* edge);

        /**
         * \brief Removes the given incoming edge from the TPGVertex.
         *
         * If the given pointer is NULL or if the given edge is not in the
         * set of the TPGVertex, nothing happens. Otherwise, the edge is
         * removed.
         *
         * \param[in] edge the TPGEdge to remove.
         */
        virtual void removeIncomingEdge(TPG::TPGEdge* edge);

        /**
         * \brief Method to add an outgoing TPGEdge to the TPGVertex.
         *
         * Since the outgoingEdges set is a std::set, the same TPGEdge pointer
         * can not be added twice to the set.
         * This method does not register the TPGVertex as the spource of the
         * TPGEdge.
         *
         * \param[in] edge the TPGEdge pointer to be added to the outgoingEdges
         *                 Set.
         */
        virtual void addOutgoingEdge(TPG::TPGEdge* edge);

        /**
         * \brief Removes the given outgoing edge from the TPGVertex.
         *
         * If the given pointer is NULL or if the given edge is not in the
         * set of the TPGVertex, nothing happens. Otherwise, the edge is
         * removed.
         *
         * \param[in] edge the TPGEdge to remove.
         */
        virtual void removeOutgoingEdge(TPG::TPGEdge* edge);

      protected:
        /**
         * \brief Protected default constructor to forbid the instanciation of
         * object of this abstract class.
         */
        TPGVertex(){};

        /**
         * \brief Set of incoming TPGEdge of the TPGVertex.
         */
        std::list<TPG::TPGEdge*> incomingEdges;

        /**
         * \brief Set of outgoing TPGEdge of the TPGVertex.
         */
        std::list<TPG::TPGEdge*> outgoingEdges;
    };
}; // namespace TPG

#endif
