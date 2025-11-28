
#ifndef AGENT_H
#define AGENT_H

#include <cstdint>

namespace Algorithm {
    /**
     * \brief Abstract class representing an Agent used by an Algorithm.
     * 
     * Available algorithms are TPG, MAPLE, and LGP
     */
    class Agent
    {
    protected:

        /// Attribute that specify if the agent can be mutated.
        bool isMutable = true;
        
        /// Unique ID of the agent.
        uint64_t agentID;

    public:

        Agent()
            {};

        /**
         * \brief return the ID of the agent.
         */
        virtual uint64_t getID() const;

        /**
         * \brief Method that return if the agent is valid for execution.
         */
        virtual bool isValid() const = 0;
    };
}; // namespace Algorithm

#endif
