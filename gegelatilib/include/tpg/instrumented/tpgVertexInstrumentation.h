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

#ifndef TPG_VERTEX_INSTRUMENTATION_H
#define TPG_VERTEX_INSTRUMENTATION_H

#include <atomic>
#include <cstddef>

namespace TPG {
    /**
     * \brief Instrumentation code for TPGVertex class for instrumented
     * execution.
     */
    class TPGVertexInstrumentation
    {
      public:
        /**
         * \brief Get the number of time a TPGVertexInstrumentation was visited.
         */
        uint64_t getNbVisits() const;

        /**
         * \brief Add one to the number of visits for this
         * TPGVertexInstrumented.
         */
        void incrementNbVisits() const;

        /**
         * \brief Get the number of time a TPGVertexInstrumentation was used.
         */
        uint64_t getNbUses() const;

        /**
         * \brief Add one to the number of uses for this
         * TPGVertexInstrumented.
         */
        void incrementNbUses() const;

        /**
         *  \brief Reset the instrumentation attributes.
         */
        void reset() const;

      protected:
        /**
         * \brief Protected default constructor to forbid instanciation.
         *
         * This constructor initializes the instrumentation attributes.
         */
        TPGVertexInstrumentation() : nbVisits{0}
        {
        }

        /// Number of a time a TPGVertex has been visited
        /// Attribute is mutable because all TPGVertex are seen as const outside
        /// from their TPGGraph.
        mutable std::atomic_uint64_t nbVisits;


        /// Number of a time a TPGVertex has been used
        /// Attribute is mutable because all TPGVertex are seen as const outside
        /// from their TPGGraph.
        mutable std::atomic_uint64_t nbUses;
    };
} // namespace TPG

#endif // !TPG_VERTEX_INSTRUMENTATION_H
