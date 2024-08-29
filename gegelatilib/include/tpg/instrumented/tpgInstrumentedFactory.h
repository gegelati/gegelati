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

#include "tpg/tpgFactory.h"
#include "tpg/tpgGraph.h"

namespace TPG {

    /// Specialization of the TPGFactory class producing TPGTeamInstrumented,
    /// TPGActionInstrumented, TPGEdgeInstrumented, and
    /// TPGExecutionEngineInstrumented
    ///
    /// Also contains a helper function for resetting the instrumentation of a
    /// TPGGraph.
    class TPGInstrumentedFactory : public TPGFactory
    {
      public:
        /// Specialization of the method returing the TPGGraph with a
        /// TPGInstrumentedFactory as an attribute.
        virtual std::shared_ptr<TPGGraph> createTPGGraph(
            const Environment& env) const override;

        ///  Specialization of the method returning a TPGTeamInstrumented
        virtual TPGTeam* createTPGTeam() const override;

        ///  Specialization of the method returning a TPGActionInstrumented
        virtual TPGAction* createTPGAction(
            const uint64_t actID, const uint64_t actClass) const override;

        ///  Specialization of the method returning a TPGEdgeInstrumented
        virtual std::unique_ptr<TPGEdge> createTPGEdge(
            const TPGVertex* src, const TPGVertex* dest,
            const std::shared_ptr<Program::Program> prog) const override;

        ///  Specialization of the method returning a
        ///  TPGExecutionEngineInstrumented
        virtual std::unique_ptr<TPGExecutionEngine> createTPGExecutionEngine(
            const Environment& env, Archive* arch = NULL) const override;
        /**
         * \brief Reset all visit and traversal counters of a TPGGraph.
         *
         * \param[in] tpg the TPGGraph whose TPGTeamInstrumented,
         * TPGActionInstrumented, and TPGEdgeInstrumented will be reseted.The
         * TPGGraph is const since all instrumentation counters are mutable.
         */
        void resetTPGGraphCounters(const TPG::TPGGraph& tpg) const;

        /**
         * \brief Removes from the TPGGraph the vertices and edges that were
         * never visited (since the last reset).
         *
         * This method removes from the TPGGraph:
         * - The TPGVertex with a number of visit equal to zero.
         * - The TPGEdge with a number of traversal equal to zero.
         *
         * The method will do nothing on a TPGGraph whose TPGVertex and TPGEdge
         * are not TPGVertexInstrumentation and TPGEdgeInstrumented
         * specializations.
         *
         * Beware, this function may remove TPGAction from the TPGGraph, thus
         * making it improper for future training.
         *
         * \param[in] tpg Reference to the TPGGraph whose
         * TPGVertexInstrumentation and TPGEdge will be removed.
         */
        void clearUnusedTPGGraphElements(TPG::TPGGraph& tpg) const;
    };
} // namespace TPG

#endif // !TPG_INSTRUMENTED_FACTORY
