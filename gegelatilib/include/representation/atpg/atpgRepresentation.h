
#ifndef ATPG_REPRESENTATION_H
#define ATPG_REPRESENTATION_H

#include <memory>
#include <vector>

#include "representation/atpg/atpgPolicyStats.h"
#include "representation/tpg/tpgRepresentation.h"
#include "representation/atpg/atpgPopulation.h"
#include "representation/atpg/atpgMutator.h"
#include "representation/tpg/tpgIndividual.h"

#include "learn/learningEnvironment.h"

namespace Representation::ATPG {

    /**
     * \brief Abstract class representing an ActionTPGRepresentation
     * 
     * ActionTPG works as classic TPG, with an addition program set on action vertices to take continuous actions
     */
    class ATPGRepresentation : public TPG::TPGRepresentation
    {
        protected:

            /// ID of the program representation associated with the TPG individuals.
            uint64_t actionProgramRepresentationID;

        public:


            /**
             * \brief Main Representation constructor.
             * 
             * \param[in] contextProgramRepresentation the sub-representation used to manipulate context programs.
             * \param[in] actionProgramRepresentation the sub-representation used to manipulate action programs.
             * \param[in] parameters the LearningParameters used by the Representation.
             * \param[in] representationName name of the representation used.
             * \param[in] representationColor name of the representation used.
             */
            ATPGRepresentation(const Representation& contextProgramRepresentation, const Representation& actionProgramRepresentation, std::unique_ptr<RepresentationParameters> parameters = std::make_unique<RepresentationParameters>(), std::string representationName = "ATPG", std::string representationColor = "#335ce2")
                : TPGRepresentation(contextProgramRepresentation, std::move(parameters), representationName, representationColor){
                this->setActionProgramRepresentation(actionProgramRepresentation);
            };

            /**
             * \brief Add the action program sub-representation to the TPGRepresentation.
             * 
             */
            void setActionProgramRepresentation(const Representation& programRepresentation);

            /**
             * \brief Add an aggregated action program representation to the TPGRepresentation.
             * 
             * \param[in] programRepresentation the aggregated action program representation.
             */
            void addAggregatedActionProgramRepresentation(const Representation& programRepresentation);

            /**
             * \brief Initialize the populationof the representation
             */
            virtual void initPopulation() override;

            /**
             * \brief Initialize the mutator of the representation
             */
            virtual void initMutator() override;

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
             * \brief Inherited method to clear all the unused sub individuals
             * 
             * The sub individuals are the individuals used by the program sub-representation and the action sub-representation.
             */
            virtual std::map<uint64_t, std::set<std::reference_wrapper<const Individual>>> getUsedSubIndividuals() const override;

            /**
             * \brief Inherited method to create the policy stats of the representation
             * 
             * This policy stats contains a program sub policy stats
             */
            virtual std::shared_ptr<PolicyStats> createPolicyStats() const override;


            /**
             * \brief inherrit from representation class
             */
            virtual void printCodeGenIndividuals(std::ofstream& fileMain, std::ofstream& fileMainH, const std::set<std::reference_wrapper<const Individual>>& individuals, std::map<uint64_t, std::set<std::reference_wrapper<const Individual>>>& subIndividuals) const;

            /**
             * Copy and return a uniqure pointer of the representation
             */
            virtual std::unique_ptr<Representation> copy() const override;
    };
}; // namespace ATPG_Representation

namespace Representation{
    /**
     * To make the aglrotihm accessible from representation namespace
     */
    using ATPGRepresentation = ATPG::ATPGRepresentation;
}

#endif
