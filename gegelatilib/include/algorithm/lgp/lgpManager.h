
#ifndef LGP_AGENT_MANAGER_H
#define LGP_AGENT_MANAGER_H

#include "algorithm/agentManager.h"
#include "algorithm/lgp/lgpAgent.h"
#include "algorithm/lgp/lgpExecutionEngine.h"


namespace Algorithm {

    namespace LGP{

        /**
         * \brief Class representing a LGPManager used by the LGPAlgorithm.
         * 
         * The AgentManager is in charge of storing, creating, copying or removing Agents.
         * Basically, the agentManager is the interface between the Algorithm and the Graph.
         */
        class LGPManager : public AgentManager
        {
        protected:
            
            /// Environment for executing LGP 
            std::shared_ptr<const Environment> env;


            /**
             * \brief Get the LGPAgent from a const Agent pointer.
             * 
             * \param[in] agent the Agent to cast.
             */
            virtual std::shared_ptr<LGPAgent> getLGPAgentFromCst(std::shared_ptr<const Agent> agent);

            /**
             * \brief Get the LGPAgent from a const Agent pointer.
             * 
             * \param[in] agent the Agent to cast.
             */
            virtual std::shared_ptr<const LGPAgent> cGetLGPAgentFromCst(std::shared_ptr<const Agent> agent) const;

        public:

            /**
             * \brief LGPManager constructor.
             * 
             * \param[in] env the Environment for executing LGP
             * \param[in] outputs outputs of the agents.
             */
            LGPManager(std::shared_ptr<const Environment> env, const Output::OutputHandler& outputs): AgentManager(outputs), env(env){};

            /**
             * \brief Create a new LGPAgent.
             * 
             * \param[in] graph the Graph associated with the Agent.
             * 
             * \return a shared pointer to the created Agent.
             */
            virtual std::shared_ptr<const Agent> createAgent(std::shared_ptr<EvoGraph::Graph> graph) override;

            /**
             * \brief Copy a LGPAgent.
             * 
             * \param[in] agent the Agent to copy.
             * \param[in] graph the Graph associated with the Agent.
             * 
             * \return a shared pointer to the created Agent.
             */
            virtual std::shared_ptr<const Agent> copyAgent(std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph) override;

            /**
             * \brief Delete the LGPAgent.
             * 
             * \param[in] agent the Agent to delete.
             * \param[in] graph the Graph associated with the Agent.
             * 
             * \return a shared pointer to the created Agent.
             */
            virtual void deleteAgent(std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph) override;

            /**
             * \brief Get the number of outputs of the agents.
             * 
             * \return the number of outputs.
             */
            virtual const Output::OutputHandler& getOutputs() const;

            /**
             * \brief Set the constant at the given index for the given agent.
             * 
             * \param[in] agent the Agent to modify.
             * \param[in] index the index of the constant to set.
             * \param[in] value the value to set.
             */
            virtual void setConstantAt(std::shared_ptr<const Agent> agent, size_t index, const Data::Constant& value);

            /**
             * \brief Delete a line to the given agent at the given index.
             * 
             * \param[in] agent the Agent to modify.
             * \param[in] index the index at which the new line should be deleted.
             */
            virtual void removeLine(std::shared_ptr<const LGPAgent> agent, size_t index);

            /**
             * \brief Add a new line to the given agent at the given index.
             * 
             * \param[in] agent the Agent to modify.
             * \param[in] index the index at which the new line should be added.
             * 
             * \return a reference to the newly added LGPLine.
             */
            virtual LGPLine& addNewLine(std::shared_ptr<const LGPAgent> agent, size_t index);
            
            /**
             * \brief Add a new line to the given agent at the end of the agent.
             * 
             * \param[in] agent the Agent to modify.
             * 
             * \return a reference to the newly added LGPLine.
             */
            virtual LGPLine& addNewLine(std::shared_ptr<const LGPAgent> agent);

            /**
             * \brief Swap two lines of the given agent.
             * 
             * \param[in] agent the Agent to modify.
             * \param[in] index1 the index of the first line to swap.
             * \param[in] index2 the index of the second line to swap.
             */
            virtual void swapLines(std::shared_ptr<const LGPAgent> agent, size_t index1, size_t index2);

            /**
             * \brief Get a line of the given agent at the given index.
             * 
             * \param[in] agent the Agent to access.
             * \param[in] index the index of the line to get.
             * 
             * \return a reference to the LGPLine at the given index.
             */
            virtual LGPLine& getLine(std::shared_ptr<const LGPAgent> agent, size_t index);

            /**
             * \brief Identify the introns of the given agent.
             * 
             * \param[in] agent the Agent to analyze.
             */
            virtual uint64_t identifyIntrons(std::shared_ptr<const Agent> agent);
            
            /**
             * \brief Check if two LGP have the same behavior.
             *
             * Two LGP have the same behaviour if their sequence of non-intron
             * Lines are strictly identical (i.e. same instructions and operands, in
             * the same order, and used Constant with identical values).
             *
             * \param[in] agent1 first lgp.
             * \param[in] agent2 second lgp.
             */
            bool hasIdenticalBehavior(std::shared_ptr<const Agent> agent1, std::shared_ptr<const Agent> agent2) const;

            /**
             * \brief Create a new ExecutionEngine for this Algorithm.
             */
            virtual std::unique_ptr<ExecutionEngine> createExecutionEngine(bool isTraining = false) const override;

        };
    } // namespace LGP
}; // namespace Algorithm

#endif // LGP_AGENT_MANAGER_H
