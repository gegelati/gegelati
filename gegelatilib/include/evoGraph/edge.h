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

#ifndef TPG_EDGE_H
#define TPG_EDGE_H

#include <memory>
#include <stdexcept>

struct CounterReset;

namespace Algorithm{
  class Agent;
}

namespace EvoGraph {
    // Declare class to make it usable as an attribute.
    class Vertex;

    /**
     * \brief Class representing edges of the Tangled Program Graphs.
     */
    class Edge
    {
      public:
        /// Default virtual destructor (for polymorphism)
        virtual ~Edge() = default;

        /**
         * \brief Main constructor of the Edge class.
         *
         * This constructor does not register the created Edge in the
         * list of incoming or outgoing edges of the given Vertex.
         *
         * \param[in] src pointer to the source Vertex of the edge.
         * \param[in] dest pointer to the destination Vertex of the edge.
         * \param[in] agentProgram the shared pointer to the Agent Program associated to the
         *            edge.
         */
        Edge(std::shared_ptr<const Vertex> src, std::shared_ptr<const Vertex> dest,
                const std::shared_ptr<const Algorithm::Agent> agentProgram = nullptr)
            : edgeID(incrementeCounter()), source{src}, destination{dest},
              program{agentProgram} {};

        /**
         * \brief Get a const shared pointer of the Agent Program of the Edge.
         *
         * \return a const shared pointer of the Agent Program of the Edge.
         */
        std::shared_ptr<const Algorithm::Agent> getProgram() const;

        /**
         * \brief Set a new Agent Program for the Edge.
         *
         * This method is const to enable use outside of the Graph which is
         * the only class accessing the non-const Edge. Since the agent
         * pointer attribute is mutable, this method can successfully be used to
         * alter the agent.
         *
         * \param[in] agentProgram the new shared pointer to a Agent Program.
         */
        void setProgram(std::shared_ptr<const Algorithm::Agent> agentProgram);

        /**
         * \brief Get the source Vertex of the Edge.
         *
         * \return a const pointer to the source Vertex.
         */
        std::shared_ptr<const Vertex> getSource() const;

        /**
         * \brief Set a new source Vertex to the Edge.
         *
         * \param[in] newSource the new Vertex  to register as the source.
         */
        void setSource(std::shared_ptr<const Vertex> newSource);

        /**
         * \brief Get the destination Vertex of the Edge.
         *
         * \return a const pointer to the destination Vertex.
         */
        virtual std::shared_ptr<const Vertex> getDestination() const;

        /**
         * \brief Set a new destination Vertex to the Edge.
         *
         * \param[in] newDestination the new Vertex to register as the
         * destination.
         */
        virtual void setDestination(std::shared_ptr<const Vertex> newDestination);

        /**
         * \brief Get the unique identifier of the Edge.
         *
         * \return the integer ID of the Edge..
         */
        virtual uint64_t getEdgeID() const;

        /**
         * \brief Set a new unique identifier to the Edge.
         *
         * \param[in] newID the new integer ID to set to the Edge.
         */
        virtual void setEdgeID(uint64_t newID);

        /**
         * \brief Get the current value of the edge ID counter.
         *
         * This method is mainly used for testing purpose to ensure that
         * edge IDs are predictable.
         *
         * \return the current value of the edge ID counter.
         */
        static uint64_t getEdgeIDCounter();

      protected:
        /// Pointer to the source Vertex of this Edge
        std::shared_ptr<const Vertex> source;

        /// Pointer to the destination Vertex of this Edge
        std::shared_ptr<const Vertex> destination;

        /// Shared pointer to the Agent to execute when evaluating the bid
        /// of this Edge.
        /// This attribute is mutable to enable its modification during
        /// mutations.
        std::shared_ptr<const Algorithm::Agent> program;

        /**
         * \brief Unique identifier of the Edge.
         */
        uint64_t edgeID;

        /**
         * \brief Incremente the edge ID counter and return the new value.
         */
        static uint64_t incrementeCounter();

        /**
         * \brief Reset the edge ID counter.
         *
         * This method set the ID counter to a new value.
         * It can quickly lead to segmentation fault if not used carefully.
         */
        static void resetEdgeIDCounter();
        friend struct ::CounterReset;

        /// Delete the default constructor.
        Edge() = delete;
    };

    /**
     * \brief Comparison function to enable sorting of Vertex with
     * STL.
     */
    bool operator<(const Edge& a, const Edge& b);

}; // namespace EvoGraph

#endif
