
#ifndef ATPG_EXECUTION_ENGINE_H
#define ATPG_EXECUTION_ENGINE_H

#include "algorithm/tpg/tpgExecutionEngine.h"

namespace Algorithm::ATPG {
    /**
     * \brief class used to execute the agent of an algorithm
     */
    class ATPGExecutionEngine : public TPG::TPGExecutionEngine {

    protected:

        /// Execution engine used to execute the action program of the ActionTPG agents
        ExecutionEngine* actionProgramExecutionEngine;

    public:

        /**
         * \brief TPGExecutionEngine constructor.
         * 
         * \param[in] outputs outputs that will be usable for
         * interacting with this LearningEnviromnent.
         * \param[in] algorithmName name of the algorithm used.
         * \param[in] isTraining Boolean indicating if this executionEngine will be executed for training or testing purpose.
         */
        ATPGExecutionEngine(const Output::OutputHandler& outputs, std::string algorithmName, bool isTraining = false): TPGExecutionEngine(outputs, algorithmName, isTraining) {}

        /**
         * \brief TPGExecutionEngine constructor.
         * 
         * \param[in] executedAgent the agent to execute.
         * \param[in] outputs outputs that will be usable for
         * interacting with this LearningEnviromnent.
         * \param[in] isTraining Boolean indicating if this executionEngine will be executed for training or testing purpose.
         */
        ATPGExecutionEngine(std::shared_ptr<const Agent> executedAgent, const Output::OutputHandler& outputs, bool isTraining = false): TPGExecutionEngine(executedAgent, outputs, isTraining) {}


        /**
         * \brief Set the action program execution engine associated with the ATPG agents.
         * 
         * \param[in] actionProgramExecutionEngine the program execution engine.
         */
        void setActionProgramExecutionEngine(std::unique_ptr<ExecutionEngine> actionProgramExecutionEngine);

        /**
         * \brief Get the ation program execution engine associated with the ATPG agents.
         * 
         * \return the action program execution engine.
         */
        ExecutionEngine& getActionProgramExecutionEngine() const { return *this->actionProgramExecutionEngine; }


        /**
         * \brief set the continuous action values based on last values outputted.
         */
        virtual void setContinuousActionValues() override {};

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

        
        /**
         * \brief Inherrited from ExecutionEngine
         */
        //virtual const std::vector<std::reference_wrapper<const Data::DataHandler>>&
        //getDataSources() const override;
        
        
    }; 
}; // namespace Algorithm::ATPG

#endif // ATPG_EXECUTION_ENGINE_H