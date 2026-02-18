
#ifndef MAPLE_EXECUTION_ENGINE_H
#define MAPLE_EXECUTION_ENGINE_H

#include "algorithm/tpg/tpgExecutionEngine.h"
#include "algorithm/maple/mapleAgent.h"


namespace Algorithm::Maple {
    /**
     * \brief class used to execute the agent of an algorithm
     */
    class MapleExecutionEngine : public TPG::TPGExecutionEngine {

    public:

        /**
         * \brief MapleExecutionEngine constructor.
         * 
         * \param[in] outputs outputs that will be usable for
         * interacting with this LearningEnviromnent.
         * \param[in] algorithmID id of the algorithm used.
         * \param[in] isTraining Boolean indicating if this executionEngine will be executed for training or testing purpose.
         */
        MapleExecutionEngine(const Output::OutputHandler& outputs, uint64_t algorithmID, bool isTraining = false): TPGExecutionEngine(outputs, algorithmID, isTraining) {}

        /**
         * \brief TPGExecutionEngine constructor.
         * 
         * \param[in] executedAgent the agent to execute.
         * \param[in] outputs outputs that will be usable for
         * interacting with this LearningEnviromnent.
         * \param[in] isTraining Boolean indicating if this executionEngine will be executed for training or testing purpose.
         */
        MapleExecutionEngine(const Agent& executedAgent, const Output::OutputHandler& outputs, bool isTraining = false): TPGExecutionEngine(executedAgent, outputs, isTraining) {}


        /**
         * \brief Execute the Graph starting from the vertex pointed by the given agent.
         *
         * This method browse the graph by successively evaluating Teams and
         * following the Edge proposing the best bids.
         * 
         * \return a vector containing all the Vertex traversed during the
         *         evaluation of the Graph. The Action resulting from the
         *         Graph execution is at the end of the returned vector.
         */
        virtual std::vector<double> execute() override; 
        
    }; 
}; // namespace Algorithm::Maple

#endif // MAPLE_EXECUTION_ENGINE_H