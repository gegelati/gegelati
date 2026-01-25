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

#ifndef TPG_INSTRUMENTED_FACTORY
#define TPG_INSTRUMENTED_FACTORY

#include "evoGraph/factory.h"
#include "evoGraph/graph.h"

namespace EvoGraph {

    /// Specialization of the GraphFactory class producing TeamInstrumented,
    /// ActionInstrumented, EdgeInstrumented, and
    /// ExecutionEngineInstrumented
    ///
    /// Also contains a helper function for resetting the instrumentation of a
    /// Graph.
    class TPGInstrumentedFactory : public GraphFactory
    {
      public:
        /// Specialization of the method returing the Graph with a
        /// TPGInstrumentedFactory as an attribute.
        virtual std::shared_ptr<Graph> createGraph() const override;

        ///  Specialization of the method returning a TeamInstrumented
        virtual std::shared_ptr<Team> createTeam() const override;

        ///  Specialization of the method returning a ActionInstrumented
        virtual std::shared_ptr<Action> createAction(
            const uint64_t id) const override;

        ///  Specialization of the method returning a EdgeInstrumented
        virtual std::shared_ptr<Edge> createEdge(
            std::shared_ptr<const Vertex> src, std::shared_ptr<const Vertex> dest,
            const std::shared_ptr<const Algorithm::Agent> prog) const override;

        /**
         * \brief Reset all visit and traversal counters of a Graph.
         *
         * \param[in] graph the Graph whose TeamInstrumented,
         * ActionInstrumented, and EdgeInstrumented will be reseted.The
         * Graph is const since all instrumentation counters are mutable.
         */
        void resetGraphCounters(const EvoGraph::Graph& graph) const;

        /**
         * \brief Removes from the Graph the vertices and edges that were
         * never visited (since the last reset).
         *
         * This method removes from the Graph:
         * - The Vertex with a number of visit equal to zero.
         * - The Edge with a number of traversal equal to zero.
         *
         * The method will do nothing on a Graph whose Vertex and Edge
         * are not VertexInstrumentation and EdgeInstrumented
         * specializations.
         *
         * Beware, this function may remove Action from the Graph, thus
         * making it improper for future training.
         *
         * \param[in] graph Reference to the Graph whose
         * VertexInstrumentation and Edge will be removed.
         */
        void clearUnusedGraphElements(EvoGraph::Graph& graph) const;
    };
} // namespace EvoGraph

#endif // !TPG_INSTRUMENTED_FACTORY
