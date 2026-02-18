
#ifndef EXECUTION_ENGINE_H
#define EXECUTION_ENGINE_H

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>
#include <optional>

#include "algorithm/agent.h"
#include "algorithm/job.h"
#include "data/dataHandler.h"
#include "outputInfo.h"

namespace Algorithm {
    /**
     * \brief class used to execute the agent of an algorithm
     */
    class ExecutionEngine {

    protected:
        /// Agent executed
        std::optional<std::reference_wrapper<const Agent>> executedAgent;

        /// Values to outputs 
        const Output::OutputHandler& outputs;

        /// Id of the algorithm.
        uint64_t algorithmID;

        /// Sub-executionEngine for sub-algorithms
        std::map<uint64_t, std::unique_ptr<ExecutionEngine>> subExecutionEngines;

        /// Boolean indicating if this executionEngine will be executed for training or testing purpose.
        bool isTraining = false;

        /// Data sources from the environment used for archiving a program.
        std::vector<std::reference_wrapper<const Data::DataHandler>>
            dataSources;



    public:
    
        // Disable copying to avoid accidental copies (use references or pointers instead).
        ExecutionEngine(const ExecutionEngine&) = delete;
        ExecutionEngine& operator=(const ExecutionEngine&) = delete;

        /**
         * \brief Main ExecutionEngine constructor.
         * 
         * \param[in] outputs outputs that will be usable for
         * interacting with this LearningEnviromnent.
         * \param[in] algorithmID id of the algorithm used.
         * \param[in] isTraining Boolean indicating if this executionEngine will be executed for training or testing purpose.
         */
        ExecutionEngine(const Output::OutputHandler& outputs, uint64_t algorithmID, bool isTraining = false): algorithmID{algorithmID}, outputs{outputs}, isTraining{isTraining} {}

        /**
         * \brief Main ExecutionEngine constructor.
         * 
         * \param[in] executedAgent the agent to execute.
         * \param[in] outputs outputs that will be usable for
         * interacting with this LearningEnviromnent.
         * \param[in] isTraining Boolean indicating if this executionEngine will be executed for training or testing purpose.
         */
        ExecutionEngine(const Agent& executedAgent, const Output::OutputHandler& outputs, bool isTraining = false): executedAgent{executedAgent}, outputs{outputs}, algorithmID{executedAgent.getAlgorithmID()}, isTraining{isTraining} {}

        /**
         * \brief setter for the isTraining attribute. 
         * 
         * \param[in] isTraining new value of boolean isTraining
         */
        void setExecutionMode(bool isTraining);

        /**
         * \brief Return the id of the algorithm.
         */
        uint64_t getAlgorithmID() const { return this->algorithmID; }

        /**
         * \brief Return the current agent executed.
         */
        virtual const Agent& getExecutedAgent() const;

        /**
         * \brief Set a new agent executed by the execution engine.
         * 
         * \param[in] newExecutedAgent new executed agent. 
         */
        virtual void setExecutedAgent(const Agent& newExecutedAgent);

        /**
         * \brief Setup the execution engine with the given job.
         * 
         * \param[in] job the job to setup the execution engine with.
         */
        virtual void setupJob(const Job& job);

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