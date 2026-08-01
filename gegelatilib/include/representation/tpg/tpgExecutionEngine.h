
#ifndef TPG_EXECUTION_ENGINE_H
#define TPG_EXECUTION_ENGINE_H

#include "representation/executionEngine.h"
#include "representation/tpg/tpgIndividual.h"
#include "representation/tpg/tpgJob.h"
#include "evoGraph/action.h"
#include "evoGraph/team.h"
#include "representation/tpg/archive.h"

#include "util/activationFunctions.h"

namespace Representation::TPG {
    /**
     * \brief class used to execute the individual of an representation
     */
    class TPGExecutionEngine : public ExecutionEngine {

    protected:

        /// Execution engine used to execute the program of the TPG individuals
        uint64_t programExecutionEngineID;

        /// TPGArchive used by the program individuals.
        std::optional<std::reference_wrapper<TPGArchive>> archive = std::nullopt;

        /// Action values selected
        std::vector<double> actionValues;

        /// Last values outputted by the program
        std::vector<double> lastValues;

    public:


        /**
         * \brief TPGExecutionEngine constructor.
         * 
         * \param[in] outputs outputs that will be usable for
         * interacting with this LearningEnviromnent.
         * \param[in] representationID id of the representation used.
         * \param[in] isTraining Boolean indicating if this executionEngine will be executed for training or testing purpose.
         */
        TPGExecutionEngine(const Output::OutputHandler& outputs, uint64_t representationID, bool isTraining = false): ExecutionEngine(outputs, representationID, isTraining) {}

        /**
         * \brief TPGExecutionEngine constructor. 
         * 
         * \param[in] executedIndividual the individual to execute.
         * \param[in] outputs outputs that will be usable for
         * interacting with this LearningEnviromnent.
         * \param[in] isTraining Boolean indicating if this executionEngine will be executed for training or testing purpose.
         */
        TPGExecutionEngine(const Individual& executedIndividual, const Output::OutputHandler& outputs, bool isTraining = false): ExecutionEngine(executedIndividual, outputs, isTraining) {}

        /**
         * Setter for the archive
         * 
         * \param[in] archive TPGArchive used by the program individuals.
         */
        void setArchive(TPGArchive& archive);

        /**
         * \brief getter for the archive.
         */
        std::optional<std::reference_wrapper<TPGArchive>> getArchive();

        /**
         * \brief Setup the execution engine with the given job.
         * 
         * \param[in] job the job to setup the execution engine with.
         */
        void setupJob(const Job& job) override;
        
        /**
         * \brief Set the program execution engine associated with the TPG individuals.
         * 
         * \param[in] programExecutionEngine the program execution engine.
         */
        void setProgramExecutionEngine(std::unique_ptr<ExecutionEngine> programExecutionEngine);

        /**
         * \brief Get the program execution engine associated with the TPG individuals.
         * 
         * \return the program execution engine.
         */
        ExecutionEngine& getProgramExecutionEngine() const { return *this->subExecutionEngines.at(this->programExecutionEngineID); }

        /**
         * \brief set the continuous action values based on last values outputted.
         */
        virtual void setContinuousActionValues();

        /**
         * \brief Execute the Program associated to an Edge and returns the
         * obtained double.
         *
         * If an TPGArchive is associated to the ExecutionEngine, the Program
         * result is recorded in it.
         *
         * If the value returned by the Program is NaN, then it is replaced with
         * a -inf value.
         *
         * \param[in] edge the const ref to the Edge whose Program will be
         * evaluated.
         * \return the double value returned by the Program of the Edge.
         */
        virtual double evaluateEdge(const EvoGraph::Edge& edge);

        /**
         * \brief Evaluate all the Program of the outgoing Edge of the
         *        Team.
         *
         * This method evaluates the Programs of all outgoing Edge of the
         * Team, and returns the reference to the Edge providing the
         * largest evaluation.
         *
         * \param[in] team the Team whose outgoing Edge are evaluated.
         * \return the reference to the Edge evaluated with the the highest
         *         double value (and not excluded).
         *
         * \throw std::runtime_error in case the Team has no outgoing edge.
         * This should not happen in a correctly constructed Graph.
         */
        virtual const EvoGraph::Edge& evaluateTeam(const EvoGraph::Team& team);

        /**
         * \brief Execute the Graph starting from the vertex pointed by the given individual.
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
        virtual const std::vector<std::reference_wrapper<const Data::DataHandler>>&
        getDataSources() const override;
        
        
    }; 
}; // namespace Representation::TPG

#endif // TPG_EXECUTION_ENGINE_H