
#ifndef MAPLE_AGENT_H
#define MAPLE_AGENT_H

#include <cstdint>
#include "algorithm/tpg/tpgAgent.h"
#include "evoGraph/action.h"


namespace Algorithm::Maple {
      
        /**
         * \brief Abstract class representing a TPGAgent
         */
        class MapleAgent : public TPG::TPGAgent
        {
        protected:

        public:

            /**
             * \brief Main constructor of the TPGAgent.
             *
             * \param[in] vertex the Vertex that the TPGAgent will represent. This vertex must be an action vertex.
             * \param[in] algorithmName name of the algorithm used.
             */
            MapleAgent(std::shared_ptr<const EvoGraph::Vertex> vertex, std::string algorithmName) 
            : TPGAgent(vertex, algorithmName) {};

            /**
             * \brief Method that return if the agent is valid for execution.
             */
            virtual bool isValid() const override;
        };
  
}; // namespace Algorithm::MAPLE

#endif // TPG_AGENT
