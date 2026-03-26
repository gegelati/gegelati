
#ifndef CGP_AGENT_MANAGER_H
#define CGP_AGENT_MANAGER_H

#include "algorithm/lgp/lgpManager.h"
#include "algorithm/cgp/cgpAgent.h"


namespace Algorithm {

    namespace CGP{

        /**
         * \brief Class representing a CGPManager used by the CGPAlgorithm.
         * 
         * The AgentManager is in charge of storing, creating, copying or removing Agents.
         * Basically, the agentManager is the interface between the Algorithm and the Graph.
         */
        class CGPManager : public LGP::LGPManager
        {
        protected:

            /**
             * \brief Get the CGPAgent from a const Agent pointer.
             * 
             * \param[in] agent the Agent to cast.
             */
            virtual CGPAgent& getCGPAgentFromCst(const Agent& agent);
        public:

            /**
             * \brief CGPManager constructor.
             * 
             * \param[in] env the Environment for executing CGP
             * \param[in] outputs outputs of the agents.
             * \param[in] algorithmID id of the algorithm used.
             */
            CGPManager(const Environment& env, const Output::OutputHandler& outputs, uint64_t algorithmID): LGPManager(env, outputs, algorithmID) {};

            /**
             * \brief Create a new CGPAgent.
             * 
             * \param[in] graph the Graph associated with the Agent.
             * 
             * \return a shared pointer to the created Agent.
             */
            virtual const Agent& createAgent(EvoGraph::Graph& graph) override;

            /**
             * \brief Copy a CGPAgent.
             * 
             * \param[in] agent the Agent to copy.
             * \param[in] graph the Graph associated with the Agent.
             * 
             * \return a shared pointer to the created Agent.
             */
            virtual const Agent& copyAgent(const Agent& agent, EvoGraph::Graph& graph) override;

            /**
             * \brief Empty a CGPAgent of its program.
             * 
             * \param[in] agent the Agent to empty.
             * \param[in] graph the Graph associated with the Agent.
             * 
             * \return a shared pointer to the created Agent.
             */
            virtual void emptyAgent(const Agent& agent, EvoGraph::Graph& graph) override;

        };
    } // namespace CGP
}; // namespace Algorithm

#endif // CGP_AGENT_MANAGER_H
