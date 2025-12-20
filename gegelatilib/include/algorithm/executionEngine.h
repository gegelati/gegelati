
#ifndef EXECUTION_ENGINE_H
#define EXECUTION_ENGINE_H

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>

#include "algorithm/agent.h"
#include "data/dataHandler.h"

namespace Algorithm {
    /**
     * \brief class used to execute the agent of an algorithm
     */
    class ExecutionEngine {

    protected:
        /// Agent executed
        std::shared_ptr<const Agent> executedAgent;

        /// Name of the algorithm.
        std::string algorithmName;

        /// Sub-executionEngine for sub-algorithms
        std::map<std::string, std::unique_ptr<ExecutionEngine>> subExecutionEngines;

        /// Boolean indicating if this executionEngine will be executed for training or testing purpose.
        bool isTraining = false;


    public:

        /**
         * \brief Main ExecutionEngine constructor.
         * 
         * \param[in] algorithmName name of the algorithm used.
         * \param[in] isTraining Boolean indicating if this executionEngine will be executed for training or testing purpose.
         */
        ExecutionEngine(std::string algorithmName, bool isTraining = false): algorithmName{algorithmName}, isTraining{isTraining} {}

        /**
         * \brief Main ExecutionEngine constructor.
         * 
         * \param[in] executedAgent the agent to execute.
         * \param[in] isTraining Boolean indicating if this executionEngine will be executed for training or testing purpose.
         */
        ExecutionEngine(std::shared_ptr<const Agent> executedAgent, bool isTraining = false): executedAgent{executedAgent}, algorithmName{executedAgent->getAlgorithmName()}, isTraining{isTraining} {
            if(executedAgent->getAlgorithmName() != algorithmName){
                throw std::runtime_error("Algorithm::ExecutionEngine::ExecutionEngine trying to set an agent from a different algorithm");
            }
        }

        /**
         * \brief setter for the isTraining attribute. 
         * 
         * \param[in] isTraining new value of boolean isTraining
         */
        void setExecutionMode(bool isTraining);

        /**
         * \brief Return the name of the algorithm.
         */
        std::string getAlgorithmName() const { return this->algorithmName; }

        /**
         * \brief Return the current agent executed.
         */
        virtual std::shared_ptr<const Agent> getExecutedAgent() const;


        /**
         * \brief Set a new agent executed by the execution engine.
         * 
         * \param[in] newExecutedAgent new executed agent. 
         */
        virtual void setExecutedAgent(std::shared_ptr<const Agent> newExecutedAgent);

        /**
         * \brief execute the current agent saved
         */
        virtual std::vector<double> execute() = 0; 

        /**
         * \brief Method for changing the dataSources on which the agent will
         * be executed.
         *
         * \param[in] dataSrc The vector of DataHandler references with which
         * the Program will be executed.
         */
        
        virtual void setDataSources(
            const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSrc);

        /**
         * \brief Get the DataHandler of the ExecutionEngine.
         *
         * \return a vector containing references to the dataHandlers of the
         * dataSourses attribute (i.e. without the registers)
         */
        virtual const std::vector<std::reference_wrapper<const Data::DataHandler>>&
        getDataSources() const = 0;

    }; // namespace ExecutionEngine

}; // namespace Algorithm

#endif // EXECUTION_ENGINE_H