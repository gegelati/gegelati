
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
        protected:
            

            /// Element of the evolution graph that the agent represent.
            /// This element can stay a nullptr if it is not required by the agent.
            std::optional<std::reference_wrapper<const EvoGraph::Vertex>> vertex;

        public:

            /**
             * \brief Main constructor of the TPGAgent.
             *
             * \param[in] vertex the Vertex that the TPGAgent will represent.
             * \param[in] algorithmID id of the algorithm used.
             */
            TPGAgent(std::optional<std::reference_wrapper<const EvoGraph::Vertex>> vertex, uint64_t algorithmID) : vertex{vertex}, Agent(algorithmID) {};


            /**
             * \brief Method that return if the agent is valid for execution.
             */
            virtual bool isValid() const;

            /**
             * \brief return if the tpgAgent has a vertex set or not.
             */
            virtual bool hasVertex() const;

            /**
             * \brief remove the vertex set to the agent.
             */
            virtual void removeVertex();

            /**
             * \brief Setter for the vertex that the agent represent
             * This vertex should be a Team
             * 
             * \param[in] newVertex the new vertex to set.
             */
            virtual void setVertex(const EvoGraph::Vertex& newVertex);

            
            /**
             * \brief Getter for the vertex that the agent represent
             */
            virtual const EvoGraph::Vertex& getVertex() const;

            /**
             * \brief Method that return if the agent's vertex is currently root.
             */
            virtual bool isRoot() const;
        };
  
}; // namespace Algorithm::TPG

#endif // TPG_AGENT
