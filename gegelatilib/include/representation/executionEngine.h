
#ifndef EXECUTION_ENGINE_H
#define EXECUTION_ENGINE_H

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>
#include <optional>

#include "representation/individual.h"
#include "representation/job.h"
#include "data/dataHandler.h"
#include "outputInfo.h"

namespace Representation {
    /**
     * \brief class used to execute the individual of an representation
     */
    class ExecutionEngine {

    protected:
        /// Individual executed
        std::optional<std::reference_wrapper<const Individual>> executedIndividual;

        /// Values to outputs 
        const Output::OutputHandler& outputs;

        /// Id of the representation.
        uint64_t representationID;

        /// Sub-executionEngine for sub-representations
        std::map<uint64_t, std::unique_ptr<ExecutionEngine>> subExecutionEngines;

        /// Boolean indicating if this executionEngine will be executed for training or testing purpose.
        bool isTraining = false;

        /// Data sources from the environment used for archiving a program.
        std::vector<std::reference_wrapper<const Data::DataHandler>>
            dataSources;



    public:
    
        /// Default polymorphic destructor
        virtual ~ExecutionEngine() = default;

        // Disable copying to avoid accidental copies (use references or pointers instead).
        ExecutionEngine(const ExecutionEngine&) = delete;
        ExecutionEngine& operator=(const ExecutionEngine&) = delete;

        /**
         * \brief Main ExecutionEngine constructor.
         * 
         * \param[in] outputs outputs that will be usable for
         * interacting with this LearningEnviromnent.
         * \param[in] representationID id of the representation used.
         * \param[in] isTraining Boolean indicating if this executionEngine will be executed for training or testing purpose.
         */
        ExecutionEngine(const Output::OutputHandler& outputs, uint64_t representationID, bool isTraining = false): representationID{representationID}, outputs{outputs}, isTraining{isTraining} {}

        /**
         * \brief Main ExecutionEngine constructor.
         * 
         * \param[in] executedIndividual the individual to execute.
         * \param[in] outputs outputs that will be usable for
         * interacting with this LearningEnviromnent.
         * \param[in] isTraining Boolean indicating if this executionEngine will be executed for training or testing purpose.
         */
        ExecutionEngine(const Individual& executedIndividual, const Output::OutputHandler& outputs, bool isTraining = false): executedIndividual{executedIndividual}, outputs{outputs}, representationID{executedIndividual.getRepresentationID()}, isTraining{isTraining} {}

        /**
         * \brief setter for the isTraining attribute. 
         * 
         * \param[in] isTraining new value of boolean isTraining
         */
        void setExecutionMode(bool isTraining);

        /**
         * \brief Return the id of the representation.
         */
        uint64_t getRepresentationID() const { return this->representationID; }

        /**
         * \brief Return the current individual executed.
         */
        virtual const Individual& getExecutedIndividual() const;

        /**
         * \brief Set a new individual executed by the execution engine.
         * 
         * \param[in] newExecutedIndividual new executed individual. 
         */
        virtual void setExecutedIndividual(const Individual& newExecutedIndividual);

        /**
         * \brief Setup the execution engine with the given job.
         * 
         * \param[in] job the job to setup the execution engine with.
         */
        virtual void setupJob(const Job& job);

        /**
         * \brief execute the current individual saved
         */
        virtual std::vector<double> execute() = 0; 

        /**
         * \brief Method for changing the dataSources on which the individual will
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

}; // namespace Representation

#endif // EXECUTION_ENGINE_H