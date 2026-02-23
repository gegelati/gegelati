
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
            
            /// Program representing the agent. Eech program is associated to a specific edge
            std::map<std::reference_wrapper<const EvoGraph::Edge>, std::optional<std::reference_wrapper<const Agent>>> programs;

            /**
             * \brief return the itertaor in the map of edge and program at the position of the specified edge.
             */
            std::map<std::reference_wrapper<const EvoGraph::Edge>, std::optional<std::reference_wrapper<const Agent>>>::iterator getIteratorEdge(const EvoGraph::Edge& edge);

            // TODO -> DIVIDE CONTEXT AND EDGE PROGRAMS
            // TODO -> CHANGE ACTION VALUE OF THE VERTICES

        public:

            /**
             * \brief Main constructor of the SpeciesAgent.
             *
             * \param[in] algorithmID id of the algorithm used.
             * \param[in] edges the edges on which programs will be set
             */
            SpeciesAgent(uint64_t algorithmID, const std::set<std::reference_wrapper<const EvoGraph::Edge>>& edges) : Agent(algorithmID) {
                for(const EvoGraph::Edge& edge: edges){
                    programs.insert({edge, std::nullopt});
                }
            };
            
            /**
             * @brief Control if agent has the searched edge
             * 
             * \param[in] edge the edge searched
             */
            bool hasEdge(const EvoGraph::Edge& edge) const;

            /**
             * @brief Control if agent has a program on the specified edge
             * 
             * \param[in] edge the edge specified
             */
            bool hasProgram(const EvoGraph::Edge& edge) const;

            /**
             * @brief Remove the program on the specified edge
             * 
             * \param[in] edge the edge specified
             */
            virtual void removeEdgeProgram(const EvoGraph::Edge& edge);

            /**
             * @brief Set the program on the specified edge
             * 
             * \param[in] edge the edge specified
             * \param[in] program the set program
             */
            virtual void setEdgeProgram(const EvoGraph::Edge& edge, const Agent& program);

            /**
             * @brief get the program on the specified edge
             * 
             * \param[in] edge the edge specified
             */
            virtual const Agent& getProgram(const EvoGraph::Edge& edge) const;

            /**
             * @brief get the map of edge and programs
             */
            virtual const std::map<std::reference_wrapper<const EvoGraph::Edge>, std::optional<std::reference_wrapper<const Agent>>>& getPrograms() const;

            /**
             * @brief indicate if the agent is valid, by checking if each edge has a program set
             */
            virtual bool isValid() const override;
        };
  
}; // namespace Algorithm::Species

#endif // SPECIES_AGENT
