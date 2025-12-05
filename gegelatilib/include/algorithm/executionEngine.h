
#ifndef EXECUTION_ENGINE_H
#define EXECUTION_ENGINE_H

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>

#include "algorithm/agent.h"

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
        std::map<std::string, std::shared_ptr<ExecutionEngine>> subExecutionEngines;


    public:

        /**
         * \brief Main ExecutionEngine constructor.
         * 
         * \param[in] algorithmName name of the algorithm used.
         */
        ExecutionEngine(std::string algorithmName): algorithmName{algorithmName} {}

        /**
         * \brief Main ExecutionEngine constructor.
         * 
         * \param[in] executedAgent the agent to execute.
         * \param[in] algorithmName name of the algorithm used.
         */
        ExecutionEngine(std::shared_ptr<const Agent> executedAgent, std::string algorithmName): executedAgent{executedAgent}, algorithmName{algorithmName} {}

        /**
         * \brief Return the name of the algorithm.
         */
        std::string getAlgorithmName() const { return this->algorithmName; }

        /**
         * \brief Return the current agent executed.
         */
        virtual std::shared_ptr<const Agent> getExecutedAgent() const;

    
        /**
         * \brief Add a sub-executionEngine to the current executionEngine.
         * 
         * \param[in] subExecutionEngine the sub-executionEngine to add.
         */
        virtual void addSubExecutionEngine(std::shared_ptr<ExecutionEngine> subExecutionEngine);


        /**
         * \brief return the sub-executionEngine corresponding to the name of the algorithm given.
         * 
         * \param[in] nameAlgorithm name of the algorithm given.
         */
        virtual std::shared_ptr<ExecutionEngine> getSubExecutionEngine(std::string nameAlgorithm);


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

    }; // namespace ExecutionEngine
}; // namespace Algorithm

#endif // EXECUTION_ENGINE_H