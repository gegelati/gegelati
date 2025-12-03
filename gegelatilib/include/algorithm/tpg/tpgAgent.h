
#ifndef TPG_AGENT_H
#define TPG_AGENT_H

#include <cstdint>
#include "algorithm/agent.h"
#include "evoGraph/vertex.h"

namespace Algorithm {

    namespace TPG {
      
        /**
         * \brief Abstract class representing a TPGAhent
         */
        class TPGAgent : public Agent
        {
        protected:

            /// Attribute that specify if the agent can be mutated.
            bool isMutable = true;

            /// Vertex of the evolution graph that the agent represent.
            const EvoGraph::Vertex* vertex;

        public:

            TPGAgent(const EvoGraph::Vertex& vertex) : Agent(), vertex{&vertex} {};

            /**
             * \brief Getter for the vertex that the agent represent
             */
            virtual const EvoGraph::Vertex& getVertex() const;

            /**
             * \brief Method that return if the agent is valid for execution.
             */
            virtual bool isValid() const;

            /**
             * \brief Setter for the vertex that the agent represent
             * 
             * \param[in] newVertex the new vertex to set.
             */
            virtual void setVertex(const EvoGraph::Vertex& newVertex);


            /**
             * \brief Method that return if the agent's vertex is currently root.
             */
            virtual bool isRoot() const;
        };
  
    };
}; // namespace Algorithm

#endif // TPG_AGENT
