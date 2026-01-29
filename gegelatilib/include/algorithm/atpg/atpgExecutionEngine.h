
#ifndef ATPG_EXECUTION_ENGINE_H
#define ATPG_EXECUTION_ENGINE_H

#include "algorithm/tpg/tpgExecutionEngine.h"
#include "algorithm/atpg/atpgAgent.h"

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
         * \param[in] archive Archive used by the program agents.
         * \param[in] isTraining Boolean indicating if this executionEngine will be executed for training or testing purpose.
         */
        ATPGExecutionEngine(const Output::OutputHandler& outputs, std::string algorithmName, Archive& archive, bool isTraining = false): TPGExecutionEngine(outputs, algorithmName, archive, isTraining) {}

        /**
         * \brief TPGExecutionEngine constructor.
         * 
         * \param[in] executedAgent the agent to execute.
         * \param[in] archive Archive used by the program agents.
         * \param[in] outputs outputs that will be usable for
         * interacting with this LearningEnviromnent.
         * \param[in] isTraining Boolean indicating if this executionEngine will be executed for training or testing purpose.
         */
        ATPGExecutionEngine(std::shared_ptr<const Agent> executedAgent, const Output::OutputHandler& outputs, Archive& archive, bool isTraining = false): TPGExecutionEngine(executedAgent, outputs, archive, isTraining) {}


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
         * \brief Execute the Program associated to an Action and returns the
         * obtained double.
         *
         *
         * \param[in] action the const ref to the Action whose Program will be
         * evaluated.
         * \return the double value returned by the Program of the Action.
         */
        virtual void evaluateAction(const EvoGraph::Action& action);

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