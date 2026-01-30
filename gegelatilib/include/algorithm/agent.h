
#ifndef AGENT_H
#define AGENT_H

#include <cstdint>
#include <string>

#include "evoGraph/element.h"

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
        
        /// Name of the algorithm.
        std::string algorithmName;

        /// Element of the evolution graph that the agent represent.
        /// This element can stay a nullptr if it is not required by the agent.
        std::shared_ptr<const EvoGraph::Element> element;

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


        /**
         * \brief Constructor for the Agent.
         * 
         * \param[in] algorithmName name of the algorithm used by the Agent.
         * \param[in] element Element of the evolution graph that the agent represent.
         */
        Agent(std::string algorithmName, std::shared_ptr<const EvoGraph::Element> element = nullptr) : algorithmName(algorithmName), element{element}, agentID(incrementeCounter()) {};

        /**
         * \brief Return the name of the algorithm.
         */
        std::string getAlgorithmName() const { return this->algorithmName; }

        /**
         * \brief return the ID of the agent.
         */
        static uint64_t getAgentIDCounter();

        /**
         * \brief Method that return if the agent is valid for execution.
         */
        virtual bool isValid() const {return true;};


        /**
         * \brief Getter for the element that the agent represent
         */
        virtual std::shared_ptr<const EvoGraph::Element> getElement() const;

        /**
         * \brief Setter for the element that the agent represent
         * 
         * \param[in] newElement the new element to set.
         */
        virtual void setElement(std::shared_ptr<const EvoGraph::Element> newElement);
    
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
