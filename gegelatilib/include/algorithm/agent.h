
#ifndef AGENT_H
#define AGENT_H

#include <cstdint>
#include <string>


struct CounterReset;
namespace Algorithm {
    /**
     * \brief Abstract class representing an Agent used by an Algorithm.
     * 
     * Available algorithms are TPG, MAPLE, and LGP
     */
    class Agent
    {
    protected:
        

        /// Unique ID of the agent.
        uint64_t agentID;

        /// Name of the algorithm.
        std::string algorithmName;

        /**
         * \brief Incremente the agent ID counter and return the new value.
         */
        static uint64_t incrementeCounter();

        /**
         * \brief Reset the agent ID counter.
         *
         * This method set the ID counter to a new value.
         * It can quickly lead to segmentation fault if not used carefully.
         */
        static void resetAgentIDCounter();
        friend struct ::CounterReset;

    public:


        /**
         * \brief Constructor for the Agent.
         * 
         * \param[in] algorithmName name of the algorithm used by the Agent.
         */
        Agent(std::string algorithmName) : algorithmName(algorithmName), agentID(incrementeCounter()) {};

        /**
         * \brief Return the name of the algorithm.
         */
        std::string getAlgorithmName() const { return this->algorithmName; }

        /**
         * \brief return the ID of the agent.
         */
        virtual uint64_t getAgentIDCounter();

        /**
         * \brief Method that return if the agent is valid for execution.
         */
        virtual bool isValid() const {return true;};


    
        /**
         * \brief Get the unique identifier of the Agent.
         *
         * \return the integer ID of the Agent.
         */
        virtual uint64_t getAgentID() const;

        /**
         * \brief Set a new unique identifier to the Agent.
         *
         * \param[in] newID the new integer ID to set to the Agent.
         */
        virtual void setAgentID(uint64_t newID);
    };
    
    /**
     * \brief Comparison function to enable sorting of Vertex with
     * STL.
     */
    bool operator<(const Agent& a, const Agent& b);
}; // namespace Algorithm

#endif
