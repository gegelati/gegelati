/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2022) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2022)
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

#ifndef TPG_EDGE_INSTRUMENTED_H
#define TPG_EDGE_INSTRUMENTED_H

#include <atomic>
#include <cstddef>

#include "evoGraph/edge.h"

namespace EvoGraph {

    /**
     * \brief Instrumented Edge class to keep track of a TPG execution
     * statistics.
     */
    class EdgeInstrumented : public Edge
    {
      public:
        /// Default constructor
        EdgeInstrumented(std::shared_ptr<const Vertex> src, std::shared_ptr<const Vertex> dest,
                            std::optional<std::reference_wrapper<const Algorithm::Agent>> agentProgram = std::nullopt)
            : Edge(src, dest, agentProgram), nbVisits{0}, nbTraversal{0}
        {
        }

        /**
         * \brief Get the number of time a Edge was visited.
         *
         * That is the number of time it caused an execution of its program.
         */
        uint64_t getNbVisits() const;

        /**
         * \brief Add one to the number of visits for this
         * Edge
         */
        void incrementNbVisits() const;

        /**
         * \brief Get the number of time a Edge was traversed.
         *
         * That is the number of time its program produced the winning bid.
         */
        uint64_t getNbTraversal() const;

        /**
         * \brief Add one to the number of traversal for this
         * Edge
         */
        void incrementNbTraversal() const;

        /**
         *  \brief Reset the instrumentation attributes.
         */
        void reset() const;

      protected:
        /// Number of a time a Edge has been visited
        /// That is the number of time it caused an execution of its program.
        /// Attribute is mutable because all Edge are seen as const outside
        /// from their Graph.
        mutable std::atomic_uint64_t nbVisits;

        /// Number of a time a Edge has been traversed
        /// That is the number of time its program produced the winning bid.
        /// Attribute is mutable because all Edge are seen as const outside
        /// from their Graph.
        mutable std::atomic_uint64_t nbTraversal;
    };
} // namespace EvoGraph

#endif // !TPG_EDGE_INSTRUMENTED_H
