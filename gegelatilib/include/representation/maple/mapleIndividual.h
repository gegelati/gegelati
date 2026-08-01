
#ifndef MAPLE_INDIVIDUAL_H
#define MAPLE_INDIVIDUAL_H

#include <cstdint>
#include "representation/tpg/tpgIndividual.h"
#include "evoGraph/action.h"


namespace Representation::Maple {
      
        /**
         * \brief Abstract class representing a TpgIndividual
         */
        class MapleIndividual : public TPG::TpgIndividual
        {
        protected:

        public:

            /**
             * \brief Main constructor of the TpgIndividual.
             *
             * \param[in] vertex the Vertex that the TpgIndividual will represent. This vertex must be an action vertex.
             * \param[in] representationID id of the representation used.
             */
            MapleIndividual(std::optional<std::reference_wrapper<const EvoGraph::Vertex>> vertex, uint64_t representationID) 
            : TpgIndividual(vertex, representationID) {};

            /**
             * \brief Method that return if the individual is valid for execution.
             */
            virtual bool isValid() const override;
        };
  
}; // namespace Representation::MAPLE

#endif // TPG_INDIVIDUAL
