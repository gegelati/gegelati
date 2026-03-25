
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
            
            /**
             * \brief string used to spot the end of a line in the agent
             * description.
             */
            static const std::string lineSeparator;

            /// Environment for executing LGP 
            const Environment& env;


            /**
             * \brief Get the LGPAgent from a const Agent pointer.
             * 
             * \param[in] agent the Agent to cast.
             */
            virtual LGPAgent& getLGPAgentFromCst(const Agent& agent);
        public:

            /**
             * \brief LGPManager constructor.
             * 
             * \param[in] env the Environment for executing LGP
             * \param[in] outputs outputs of the agents.
             * \param[in] algorithmID id of the algorithm used.
             */
            LGPManager(const Environment& env, const Output::OutputHandler& outputs, uint64_t algorithmID): AgentManager(outputs, algorithmID), env(env){};

            /**
             * \brief Create a new LGPAgent.
             * 
             * \param[in] graph the Graph associated with the Agent.
             * 
             * \return a shared pointer to the created Agent.
             */
            virtual const Agent& createAgent(EvoGraph::Graph& graph) override;

            /**
             * \brief Copy a LGPAgent.
             * 
             * \param[in] agent the Agent to copy.
             * \param[in] graph the Graph associated with the Agent.
             * 
             * \return a shared pointer to the created Agent.
             */
            virtual const Agent& copyAgent(const Agent& agent, EvoGraph::Graph& graph) override;

            /**
             * \brief Empty a LGPAgent of its program.
             * 
             * \param[in] agent the Agent to empty.
             * \param[in] graph the Graph associated with the Agent.
             * 
             * \return a shared pointer to the created Agent.
             */
            virtual void emptyAgent(const Agent& agent, EvoGraph::Graph& graph) override;

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
            virtual void setConstantAt(const Agent& agent, size_t index, const Data::Constant& value);

            /**
             * \brief Delete a line to the given agent at the given index.
             * 
             * \param[in] agent the Agent to modify.
             * \param[in] index the index at which the new line should be deleted.
             */
            virtual void removeLine(const Agent& agent, size_t index);

            /**
             * \brief Add a new line to the given agent at the given index.
             * 
             * \param[in] agent the Agent to modify.
             * \param[in] index the index at which the new line should be added.
             * 
             * \return a reference to the newly added LGPLine.
             */
            virtual const LGPLine& addNewLine(const Agent& agent, size_t index);
            
            /**
             * \brief Add a new line to the given agent at the end of the agent.
             * 
             * \param[in] agent the Agent to modify.
             * 
             * \return a reference to the newly added LGPLine.
             */
            virtual const LGPLine& addNewLine(const Agent& agent);

            /**
             * \brief Add a copy of the given line to the given agent at the end of the agent.
             * 
             * \param[in] agent the Agent to modify.
             * \param[in] newLine the line to copy.
             */
            virtual void addNewLine(const Agent& agent, const LGPLine& newLine);

            /**
             * \brief Swap two lines of the given agent.
             * 
             * \param[in] agent the Agent to modify.
             * \param[in] index1 the index of the first line to swap.
             * \param[in] index2 the index of the second line to swap.
             */
            virtual void swapLines(const Agent& agent, size_t index1, size_t index2);

            /**
             * \brief Get a line of the given agent at the given index.
             * 
             * \param[in] agent the Agent to access.
             * \param[in] index the index of the line to get.
             * 
             * \return a reference to the LGPLine at the given index.
             */
            virtual const LGPLine& getLine(const Agent& agent, size_t index) const;

            /**
             * \brief Get a line of the given agent at the given index for mutation.
             * 
             * \param[in] agent the Agent to access.
             * \param[in] index the index of the line to get.
             * 
             * \return a reference to the LGPLine at the given index.
             */
            virtual LGPLine& getLineForMutation(const Agent& agent, size_t index);

            /**
             * \brief Identify the introns of the given agent.
             * 
             * \param[in] agent the Agent to analyze.
             */
            virtual uint64_t identifyIntrons(const Agent& agent);
            
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
            bool hasIdenticalBehavior(const Agent& agent1, const Agent& agent2) const;

            /**
             * \brief Create a new ExecutionEngine for this Algorithm.
             */
            virtual std::unique_ptr<ExecutionEngine> createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources = {}, bool isTraining = false) const override;

            


            /**
             * \brief Set a specific output index to an agent.
             * 
             * 
             * \param[in] agent the Agent to empty.
             * \param[in] newOutputIndex new output index set
             * \param[in] location location of input at which the new index is set
             */
            void setOutputIndex(const LGPAgent& agent, size_t newOutputIndex, size_t location);

            /**
             * \brief Reads the content of the operands and puts it in the line
             * passed in parameter
             *
             * \param[in] str the string to parse
             * \param[in] line the line to fill with the parsed informations
             */
            virtual void readOperands(std::string& str, LGPLine& line);

            /**
             * \brief Reads the content of a line
             */
            virtual void readLines(std::string instructionsStr, const Agent& agent);

        };
    } // namespace LGP
}; // namespace Algorithm

#endif // LGP_AGENT_MANAGER_H
