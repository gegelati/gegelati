
#ifndef TPG_EXECUTION_ENGINE_H
#define TPG_EXECUTION_ENGINE_H

#include "algorithm/executionEngine.h"
#include "algorithm/tpg/tpgAgent.h"
#include "algorithm/tpg/tpgJob.h"
#include "evoGraph/action.h"
#include "evoGraph/team.h"
#include "archive.h"

#include "util/activationFunctions.h"

namespace Algorithm::TPG {
    /**
     * \brief class used to execute the agent of an algorithm
     */
    class TPGExecutionEngine : public ExecutionEngine {

    protected:

        /// Execution engine used to execute the program of the TPG agents
        ExecutionEngine* programExecutionEngine;

        /// Archive used by the program agents.
        std::reference_wrapper<Archive> archive;

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
         * \param[in] algorithmName name of the algorithm used.
         * \param[in] archive Archive used by the program agents.
         * \param[in] isTraining Boolean indicating if this executionEngine will be executed for training or testing purpose.
         */
        TPGExecutionEngine(const Output::OutputHandler& outputs, std::string algorithmName, Archive& archive, bool isTraining = false): ExecutionEngine(outputs, algorithmName, isTraining), archive{archive} {}

        /**
         * \brief TPGExecutionEngine constructor.
         * 
         * \param[in] executedAgent the agent to execute.
         * \param[in] archive Archive used by the program agents.
         * \param[in] outputs outputs that will be usable for
         * interacting with this LearningEnviromnent.
         * \param[in] isTraining Boolean indicating if this executionEngine will be executed for training or testing purpose.
         */
        TPGExecutionEngine(std::shared_ptr<const Agent> executedAgent, const Output::OutputHandler& outputs, Archive& archive, bool isTraining = false): ExecutionEngine(executedAgent, outputs, isTraining), archive{archive} {}

        /**
         * Setter for the archive
         * 
         * \param[in] archive Archive used by the program agents.
         */
        void setArchive(Archive& archive);

        /**
         * \brief getter for the archive.
         */
        Archive& getArchive();

        /**
         * \brief Setup the execution engine with the given job.
         * 
         * \param[in] job the job to setup the execution engine with.
         */
        void setupJob(const Job& job) override;
        
        /**
         * \brief Set the program execution engine associated with the TPG agents.
         * 
         * \param[in] programExecutionEngine the program execution engine.
         */
        void setProgramExecutionEngine(std::unique_ptr<ExecutionEngine> programExecutionEngine);

        /**
         * \brief Get the program execution engine associated with the TPG agents.
         * 
         * \return the program execution engine.
         */
        ExecutionEngine& getProgramExecutionEngine() const { return *this->programExecutionEngine; }

        /**
         * \brief set the continuous action values based on last values outputted.
         */
        virtual void setContinuousActionValues();

        /**
         * \brief Execute the Program associated to an Edge and returns the
         * obtained double.
         *
         * If an Archive is associated to the ExecutionEngine, the Program
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
        virtual std::shared_ptr<const EvoGraph::Edge> evaluateTeam(const EvoGraph::Team& team);

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
        virtual const std::vector<std::reference_wrapper<const Data::DataHandler>>&
        getDataSources() const override;
        
        
    }; 
}; // namespace Algorithm::TPG

#endif // TPG_EXECUTION_ENGINE_H