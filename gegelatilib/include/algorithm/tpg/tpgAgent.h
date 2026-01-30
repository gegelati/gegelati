
#ifndef TPG_AGENT_H
#define TPG_AGENT_H

#include <cstdint>
#include "algorithm/agent.h"
#include "evoGraph/team.h"
#include "evoGraph/vertex.h"

namespace Algorithm::TPG {
      
        /**
         * \brief Abstract class representing a TPGAgent
         */
        class TPGAgent : public Agent
        {

        public:

            /**
             * \brief Main constructor of the TPGAgent.
             *
             * \param[in] vertex the Vertex that the TPGAgent will represent.
             * \param[in] algorithmName name of the algorithm used.
             */
            TPGAgent(std::shared_ptr<const EvoGraph::Vertex> vertex, std::string algorithmName) : Agent(algorithmName, vertex) {};


            /**
             * \brief Method that return if the agent is valid for execution.
             */
            virtual bool isValid() const;

            /**
             * \brief Setter for the element that the agent represent
             * This element should be a Team
             * 
             * \param[in] newElement the new element to set.
             */
            virtual void setElement(std::shared_ptr<const EvoGraph::Element> newElement) override;

            
            /**
             * \brief Getter for the vertex that the agent represent
             */
            virtual std::shared_ptr<const EvoGraph::Vertex> getVertex() const;

            /**
             * \brief Method that return if the agent's vertex is currently root.
             */
            virtual bool isRoot() const;
        };
  
}; // namespace Algorithm::TPG

#endif // TPG_AGENT
