
#ifndef TPG_EXECUTION_ENGINE_H
#define TPG_EXECUTION_ENGINE_H

#include "algorithm/executionEngine.h"
#include "algorithm/tpg/tpgAgent.h"
#include "evoGraph/action.h"
#include "evoGraph/team.h"

namespace Algorithm::TPG {
    /**
     * \brief class used to execute the agent of an algorithm
     */
    class TPGExecutionEngine : public ExecutionEngine {

    protected:

        /// Execution engine used to execute the program of the TPG agents
        std::unique_ptr<ExecutionEngine> programExecutionEngine;

    public:

        /**
         * \brief TPGExecutionEngine constructor.
         * 
         * \param[in] algorithmName name of the algorithm used.
         */
        TPGExecutionEngine(std::string algorithmName): ExecutionEngine(algorithmName) {}

        /**
         * \brief TPGExecutionEngine constructor.
         * 
         * \param[in] executedAgent the agent to execute.
         */
        TPGExecutionEngine(std::shared_ptr<const Agent> executedAgent): ExecutionEngine(executedAgent) {}

        /**
         * \brief Set the program execution engine associated with the TPG agents.
         * 
         * \param[in] programExecutionEngine the program execution engine.
         */
        void setProgramExecutionEngine(std::unique_ptr<ExecutionEngine> programExecutionEngine) { this->programExecutionEngine = std::move(programExecutionEngine); }

        /**
         * \brief Get the program execution engine associated with the TPG agents.
         * 
         * \return the program execution engine.
         */
        ExecutionEngine& getProgramExecutionEngine() const { return *this->programExecutionEngine; }

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
        
        
    }; 
}; // namespace Algorithm::TPG

#endif // TPG_EXECUTION_ENGINE_H