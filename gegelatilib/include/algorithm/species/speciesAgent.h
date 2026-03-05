
#ifndef SPECIES_AGENT_H
#define SPECIES_AGENT_H

#include <map>

#include <cstdint>
#include "algorithm/agent.h"
#include "evoGraph/edge.h"

namespace Algorithm::Species {
      
        /**
         * \brief Abstract class representing a SpeciesAgent
         */
        class SpeciesAgent : public Agent
        {
        protected:
            
            /// action Programs representing the agent. Each action program is associated to a specific edge
            std::map<std::reference_wrapper<const EvoGraph::Edge>, std::optional<std::reference_wrapper<const Agent>>> actionPrograms;

            /// context Programs representing the agent. Each context program is associated to a specific edge
            std::map<std::reference_wrapper<const EvoGraph::Edge>, std::optional<std::reference_wrapper<const Agent>>> contextPrograms;
            
            /// Map specific to the agent, linking the action vertices to the action used by the agent.
            /// All action in both keys and values should appear once.
            std::map<size_t, size_t> actionLinks;

            /**
             * \brief return the itertaor in the map of edge and program at the position of the specified edge.
             */
            std::map<std::reference_wrapper<const EvoGraph::Edge>, std::optional<std::reference_wrapper<const Agent>>>::iterator getIteratorActionEdge(const EvoGraph::Edge& edge);
            /**
             * \brief return the itertaor in the map of edge and program at the position of the specified edge.
             */
            std::map<std::reference_wrapper<const EvoGraph::Edge>, std::optional<std::reference_wrapper<const Agent>>>::iterator getIteratorContextEdge(const EvoGraph::Edge& edge);

        public:

            /**
             * \brief Main constructor of the SpeciesAgent.
             *
             * \param[in] algorithmID id of the algorithm used.
             * \param[in] nbActions number of actions used by the agent
             * \param[in] actionEdges the action edges on which programs will be set
             * \param[in] contextEdges the context edges on which programs will be set
             */
            SpeciesAgent(uint64_t algorithmID, size_t nbActions, const std::set<std::reference_wrapper<const EvoGraph::Edge>>& actionEdges, const std::set<std::reference_wrapper<const EvoGraph::Edge>>& contextEdges) : Agent(algorithmID) {
                // Initialize with : action <=> action.
                for(size_t idx = 0; idx < nbActions; idx++) {
                    actionLinks.insert({idx, idx});
                }
                for(const EvoGraph::Edge& edge: actionEdges){
                    actionPrograms.insert({edge, std::nullopt});
                }                
                for(const EvoGraph::Edge& edge: contextEdges){
                    contextPrograms.insert({edge, std::nullopt});
                }
            };

            /**
             * \brief set an action link between an action vertex ID an action
             * 
             * \param[in] actionID action vertex ID
             * \param[in] actionValue action value searched
             */
            void setActionLink(size_t actionID, size_t actionValue);

            /**
             * \brief getter for an action vertex ID
             * 
             * \param[in] actionID action vertex ID
             */
            size_t getActionLink(size_t actionID) const;

            /**
             * \brief Getter for the map of action links
             */
            const std::map<size_t, size_t>& getActionLinks() const;
            
            /**
             * @brief Control if agent has the searched edge
             * 
             * \param[in] edge the edge searched
             */
            bool hasActionEdge(const EvoGraph::Edge& edge) const;

            /**
             * @brief Control if agent has a program on the specified edge
             * 
             * \param[in] edge the edge specified
             */
            bool hasActionProgram(const EvoGraph::Edge& edge) const;

            /**
             * @brief Remove the program on the specified edge
             * 
             * \param[in] edge the edge specified
             */
            virtual void removeActionEdgeProgram(const EvoGraph::Edge& edge);

            /**
             * @brief Set the program on the specified edge
             * 
             * \param[in] edge the edge specified
             * \param[in] program the set program
             */
            virtual void setActionEdgeProgram(const EvoGraph::Edge& edge, const Agent& program);

            /**
             * @brief get the program on the specified edge
             * 
             * \param[in] edge the edge specified
             */
            virtual const Agent& getActionProgram(const EvoGraph::Edge& edge) const;

            /**
             * @brief get the map of edge and programs
             */
            virtual const std::map<std::reference_wrapper<const EvoGraph::Edge>, std::optional<std::reference_wrapper<const Agent>>>& getActionPrograms() const;
            
            /**
             * @brief Control if agent has the searched edge
             * 
             * \param[in] edge the edge searched
             */
            bool hasContextEdge(const EvoGraph::Edge& edge) const;

            /**
             * @brief Control if agent has a program on the specified edge
             * 
             * \param[in] edge the edge specified
             */
            bool hasContextProgram(const EvoGraph::Edge& edge) const;

            /**
             * @brief Remove the program on the specified edge
             * 
             * \param[in] edge the edge specified
             */
            virtual void removeContextEdgeProgram(const EvoGraph::Edge& edge);

            /**
             * @brief Set the program on the specified edge
             * 
             * \param[in] edge the edge specified
             * \param[in] program the set program
             */
            virtual void setContextEdgeProgram(const EvoGraph::Edge& edge, const Agent& program);

            /**
             * @brief get the program on the specified edge
             * 
             * \param[in] edge the edge specified
             */
            virtual const Agent& getContextProgram(const EvoGraph::Edge& edge) const;

            /**
             * @brief get the map of edge and programs
             */
            virtual const std::map<std::reference_wrapper<const EvoGraph::Edge>, std::optional<std::reference_wrapper<const Agent>>>& getContextPrograms() const;

            /**
             * @brief get the program on the specified edge, context or action independant
             * 
             * \param[in] edge the edge specified
             */
            virtual const Agent& getProgram(const EvoGraph::Edge& edge) const;

            /**
             * @brief indicate if the agent is valid, by checking if each edge has a program set
             */
            virtual bool isValid() const override;
        };
  
}; // namespace Algorithm::Species

#endif // SPECIES_AGENT
