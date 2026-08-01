
#ifndef ATPG_INDIVIDUAL_POPULATION_H
#define ATPG_INDIVIDUAL_POPULATION_H

#include "representation/tpg/tpgPopulation.h"
#include "representation/atpg/atpgExecutionEngine.h"

namespace Representation::ATPG {


    /**
     * \brief Class representing a TpgPopulation used by the ActionTPGRepresentation.
     * 
     * The Population is in charge of storing, creating, copying or removing Individuals.
     * Basically, the population is the interface between the Representation and the Graph.
     */
    class ATpgPopulation : public TPG::TpgPopulation
    {

    protected:
    

        /// Name of the action program representation associated with the ATPG individuals.
        uint64_t actionProgramRepresentationID;

    public:

        /**
         * \brief Main ATpgPopulation constructor.
         * 
         * \param[in] outputs outputs of the individuals.
         * \param[in] representationID id of the representation used.
         */
        ATpgPopulation(const Output::OutputHandler& outputs, uint64_t representationID) : TpgPopulation(outputs, representationID) {};

        /**
         * \brief Set the id of the action program representation associated with the TPG individuals.
         * 
         * \param[in] id the id of the action program representation.
         */
        void setActionProgramRepresentationID(uint64_t id) { this->actionProgramRepresentationID = id; }


        /**
         * \brief Delete the TpgIndividual.
         * 
         * \param[in] individual the Individual to delete.
         * \param[in] graph the Graph associated with the Individual.
         * 
         * \return a shared pointer to the created Individual.
         */
        virtual void emptyIndividual(const Individual& individual, EvoGraph::Graph& graph) override;

        /**
         * \brief create and return a TPG execution engine.
         */
        virtual std::unique_ptr<ExecutionEngine> createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources = {}, bool isTraining = false) const override;
    };
}; // namespace Representation::ATPG

#endif // ATPG_INDIVIDUAL_POPULATION_H
