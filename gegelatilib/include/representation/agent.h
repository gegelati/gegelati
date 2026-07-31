#ifndef AGENT_H
#define AGENT_H

#include <cstdint>
#include <string>

#include "evoGraph/element.h"

struct CounterReset;
namespace Representation {
    /**
     * \brief Abstract class representing an Agent used by an Representation.
     * 
     * Available representations are TPG, MAPLE, and LGP
     */
    class Agent
    {
    protected:
        
        /// ID of the representation.
        uint64_t representationID;

        /// Unique ID of the agent.
        uint64_t agentID;

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

        /// Default polymorphic destructor
        virtual ~Agent() = default;


        /**
         * \brief Constructor for the Agent.
         * 
         * \param[in] representationID id of the representation used by the Agent.
         */
        Agent(uint64_t representationID) : representationID(representationID), agentID(incrementeCounter()) {};

        /**
         * \brief Return the id of the representation.
         */
        uint64_t getRepresentationID() const { return this->representationID; }

        /**
         * \brief return the ID of the agent.
         */
        static uint64_t getAgentIDCounter();

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

        // Disable copying to avoid accidental copies (use references or pointers instead).
        Agent(const Agent&) = delete;
        Agent& operator=(const Agent&) = delete;
    };
    
    /**
     * \brief Comparison function to enable sorting of Agent with
     * STL.
     */
    bool operator<(const Agent& a, const Agent& b);

    /**
     * \brief Comparison function to enable sorting of Agent with
     * STL.
     */
    bool operator==(const Agent& a, const Agent& b);

    /**
     * \brief Comparison function to enable sorting of Agent with
     * STL.
     */
    bool operator!=(const Agent& a, const Agent& b);
}; // namespace Representation

#endif
