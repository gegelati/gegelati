#ifndef TPG_EXECUTION_ENGINE_KEYED_H
#define TPG_EXECUTION_ENGINE_KEYED_H

#include <set>
#include <vector>

#include "archive.h"
#include "program/programExecutionEngine.h"
#include "tpg/tpgExecutionEngine.h"

#include "tpg/tpgGraph.h"

namespace TPG {
    /**
     * Specialization of the TPGExecutionEngine class.
     */
    class TPGExecutionEngineKeyed : public TPGExecutionEngine
    {
      protected:
        ///  Integer Keys collected during the execution of the TPGGraph. Each
        ///  key is a prime number which unlocks the execution of TPGEdgeKeyed
        ///  associated to a lock that is a multiple of this prime number.
        std::set<uint64_t> collectedKeys;

      public:
        /**
         * \brief Main constructor of the class.
         *
         * \param[in] env Environment in which the Program of the TPGGraph will
         *                be executed.
         * \param[in] arch pointer to the Archive for storing recordings of
         *                 the Program Execution. By default, a NULL pointer is
         *                 given, meaning that no recording of the execution
         *                 will be made.
         */
        TPGExecutionEngineKeyed(const Environment& env, Archive* arch = NULL)
            : TPGExecutionEngine(env, arch) {};

        /**
         * \brief Specialization of the evaluateTeam function.
         *
         * This specialization evaluates the Programs of all outgoing
         * TPGEdgeKeyed of the TPGTeamKeyed whose lock is a multiple of the key
         * of the TPGTeamKeyed, and returns the reference to the TPGEdgeKeyed
         * providing the largest bid.
         *
         * Key of the TPGTeamKeyed is added to the collectedKeys attribute of
         * the TPGExecutionEngineKeyed for subsequent TPGTeamKeyed in the graph.
         */
        const TPG::TPGEdge& evaluateTeam(const TPGTeam& team) override;

        /**
         * \brief Specialization of the evaluateTeam function.
         *
         * In addition to calling the executeFromRoot method from
         * TPGExecutionEngine, this specialization resets the collectedKeys
         * attribute of the TPGExecutionEngineKeyed to an empty set.
         */
        const std::pair<std::vector<const TPG::TPGVertex*>, std::vector<double>>
        executeFromRoot(const TPG::TPGVertex& root,
                        const std::vector<uint64_t>& initActions = {
                            0}) override;

        /**
         * \brief Get the set of keys collected during the execution of the
         * TPGGraph.
         *
         * \return The set of keys collected during the execution of the
         * TPGGraph.
         */
        const std::set<uint64_t> getCollectedKeys() const;

      private:
        /**
         * \brief Clear the set of keys collected during the execution of the
         * TPGGraph.
         */
        void clearCollectedKeys();
    };
}; // namespace TPG

#endif
