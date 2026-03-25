
#ifndef CGP_AGENT_H
#define CGP_AGENT_H

#include <cstdint>
#include "algorithm/lgp/lgpAgent.h"
#include "data/constantHandler.h"
#include "outputInfo.h"

namespace Algorithm::CGP {
      
        /**
         * \brief Class representing a CGPAgent, which is basically a LGP in two dimensions, evolving differently and with index of outputs evolving
         */
        class CGPAgent : public LGP::LGPAgent
        {
        protected:

            /// number of layer of nodes
            size_t nbLayers;
            /// number of nodes per layer
            size_t nbNodesLayer;
            /// total number of nodes
            size_t nbNodes;

            /// Delete the default constructor.
            CGPAgent() = delete;

        public:
            /**
             * \brief Main constructor of the CGPAgent.
             *
             * \param[in] e the reference to the Environment that will be referenced
             * by the CGPAgent.
             * \param[in] nbLayers number of layer of nodes
             * \param[in] nbNodesLayer number of nodes per layer
             * \param[in] outputs outputs of the CGP.
             * \param[in] algorithmID id of the algorithm used.
             */
            CGPAgent(const Environment& e, size_t nbLayers, size_t nbNodesLayer, const Output::OutputHandler& outputs, uint64_t algorithmID)
                : LGPAgent(e, outputs, algorithmID), nbLayers{nbLayers}, nbNodesLayer{nbNodesLayer}, nbNodes{nbLayers * nbNodesLayer} {
                };

            // Disable copy semantics: CGPAgent should never be copied.
            CGPAgent(const CGPAgent&) = delete;
            CGPAgent& operator=(const CGPAgent&) = delete;


            /**
             * \brief getter for nbLayers
             */
            size_t getNbLayers() const;

            /**
             * \brief getter for nbNodesLayer
             */
            size_t getNbNodesLayer() const;

            /**
             * \brief getter for nbNodes
             */
            size_t getNbNodes() const;

        };
    
}; // namespace Algorithm::CGP

#endif // CGP_AGENT
