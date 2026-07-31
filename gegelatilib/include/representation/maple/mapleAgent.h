
#ifndef MAPLE_AGENT_H
#define MAPLE_AGENT_H

#include <cstdint>
#include "representation/tpg/tpgAgent.h"
#include "evoGraph/action.h"


namespace Representation::Maple {
      
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
             * \param[in] representationID id of the representation used.
             */
            MapleAgent(std::optional<std::reference_wrapper<const EvoGraph::Vertex>> vertex, uint64_t representationID) 
            : TPGAgent(vertex, representationID) {};

            /**
             * \brief Method that return if the agent is valid for execution.
             */
            virtual bool isValid() const override;
        };
  
}; // namespace Representation::MAPLE

#endif // TPG_AGENT
