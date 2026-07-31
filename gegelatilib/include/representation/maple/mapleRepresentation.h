

#ifndef MAPLE_REPRESENTATION_H
#define MAPLE_REPRESENTATION_H

#include "representation/tpg/tpgRepresentation.h"
#include "representation/maple/maplePopulation.h"
#include "representation/maple/mapleMutator.h"

namespace Representation::Maple {

    /**
     * \brief Class representing a MapleRepresentation, inherit from TPGRepresentation
     * 
     * Maple agents can only contain action vertex. Each action contains a 
     */
    class MapleRepresentation : public TPG::TPGRepresentation
    {
        public:

            /**
             * \brief Main Representation constructor.
             * 
             * \param[in] programRepresentation the sub-representation used to manipulate programs.
             * \param[in] parameters the LearningParameters used by the Representation.
             * \param[in] representationName name of the representation used.
             * \param[in] representationColor name of the representation used.
             */
            MapleRepresentation(const Representation& programRepresentation, std::unique_ptr<RepresentationParameters> parameters = std::make_unique<RepresentationParameters>(), std::string representationName = "MAPLE", std::string representationColor = "#ffd900")
                : TPG::TPGRepresentation(programRepresentation, std::move(parameters), representationName, representationColor){
            };

            
            /**
             * \brief Initialize the populationof the representation
             * 
             * \param[in] outputs outputs needed for the representation.
             */
            virtual void initPopulation() override;

            /**
             * \brief Initialize the mutator of the representation
             */
            virtual void initMutator() override;

            
            /**
             * \brief Initialize the representation.
             */
            virtual void initRepresentation(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph) override;

            /**
             * \brief Initialize the sub-representations of the representation
             * 
             * \param[in] rng deterministic random generator
             * \param[in] outputs outputs needed for the representation.
             * \param[in] dataSource input sources of the representation.
             * \param[in] graph the EvoGraph::Graph used by the representation.
             */
            virtual void initSubRepresentations(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph) override;
            
            /**
             * Copy and return a uniqure pointer of the representation
             */
            virtual std::unique_ptr<Representation> copy() const override;

            
            /**
             * \brief inherrit from representation class
             */
            virtual void printCodeGenAgents(std::ofstream& fileMain, std::ofstream& fileMainH, const std::set<std::reference_wrapper<const Individual>>& agents, std::map<uint64_t, std::set<std::reference_wrapper<const Individual>>>& subAgents) const;
            
    };

} // namespace Representation::Maple


namespace Representation{
    /**
     * To make the representation accessible from representation namespace
     */
    using MapleRepresentation = Maple::MapleRepresentation;
}

#endif // MAPLE_REPRESENTATION_H