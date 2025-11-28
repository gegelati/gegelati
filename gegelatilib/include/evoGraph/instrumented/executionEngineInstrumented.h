/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2022 - 2025) :
 *
 * Elinor Montmasson <elinor.montmasson@gmail.com> (2022)
 * Karol Desnos <kdesnos@insa-rennes.fr> (2022)
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

#ifndef TPG_EXECUTION_ENGINE_INSTRUMENTED_H
#define TPG_EXECUTION_ENGINE_INSTRUMENTED_H

#include <set>
#include <vector>

#include "archive.h"
#include "program/programExecutionEngine.h"
#include "evoGraph/executionEngine.h"

#include "evoGraph/graph.h"

namespace EvoGraph {
    /**
     * Specialization of the ExecutionEngine class.
     */
    class ExecutionEngineInstrumented : public ExecutionEngine
    {
      protected:
        /// History of all previous execution traces. New traces are pushed
        /// back.
        std::vector<std::vector<const Vertex*>> traceHistory;

      public:
        /**
         * \brief Main constructor of the class.
         *
         * \param[in] env Environment in which the Program of the Graph will
         *                be executed.
         * \param[in] arch pointer to the Archive for storing recordings of
         *                 the Program Execution. By default, a NULL pointer is
         *                 given, meaning that no recording of the execution
         *                 will be made.
         */
        ExecutionEngineInstrumented(const Environment& env,
                                       Archive* arch = NULL)
            : ExecutionEngine(env, arch){};

        /**
         * \brief Specialization of the evaluateEdge function.
         *
         * In addition to calling the evaluateEdge method from
         * ExecutionEngine, this specialization increments the number of
         * visits of the evaluated Edge.
         */
        double evaluateEdge(const Edge& edge) override;

        /**
         * \brief Specialization of the evaluateTeam function.
         *
         * In addition to calling the evaluateTeam method from
         * ExecutionEngine, this specialization increments the number of
         * visits of the evaluated TPGTeam and the number of traversal of the
         * Edge with the winning bid.
         */
        const EvoGraph::Edge& evaluateTeam(const TPGTeam& team) override;
        /**
         * \brief Specialization of the evaluateTeam function.
         *
         * In addition to calling the executeFromRoot method from
         * ExecutionEngine, this specialization increments the number of
         * visits of the reached Action.
         */
        const std::pair<std::vector<const EvoGraph::Vertex*>, std::vector<double>>
        executeFromRoot(const EvoGraph::Vertex& root,
                        const std::vector<uint64_t>& initActions = {
                            0}) override;

        /// Get all previous execution traces.
        const std::vector<std::vector<const Vertex*>>& getTraceHistory()
            const;

        /// Clear the trace history from all previous execution trace.
        void clearTraceHistory();
    };
}; // namespace EvoGraph

#endif
