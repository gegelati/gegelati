
#ifndef ATPG_AGENT_H
#define ATPG_AGENT_H

#include <cstdint>
#include "algorithm/tpg/tpgAgent.h"

namespace Algorithm::ATPG {
      
        /**
         * \brief Abstract class representing a ActionTPGAgent
         */
        class ATPGAgent : public TPG::TPGAgent
        {

        public:

            /**
             * \brief Main constructor of the ATPGAgent.
             *
             * \param[in] vertex the Vertex that the TPGAgent will represent.
             * \param[in] algorithmName name of the algorithm used.
             */
            ATPGAgent(std::shared_ptr<const EvoGraph::Vertex> vertex, std::string algorithmName) : TPGAgent(vertex, algorithmName) {
            };
        };
  
}; // namespace Algorithm::ATPG

#endif // ATPG_AGENT
