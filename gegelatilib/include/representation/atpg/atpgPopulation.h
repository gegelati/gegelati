
#ifndef ATPG_AGENT_POPULATION_H
#define ATPG_AGENT_POPULATION_H

#include "representation/tpg/tpgPopulation.h"
#include "representation/atpg/atpgExecutionEngine.h"

namespace Representation::ATPG {


    /**
     * \brief Class representing a TpgPopulation used by the ActionTPGRepresentation.
     * 
     * The Population is in charge of storing, creating, copying or removing Agents.
     * Basically, the population is the interface between the Representation and the Graph.
     */
    class ATpgPopulation : public TPG::TpgPopulation
    {

    protected:
    

        /// Name of the action program representation associated with the ATPG agents.
        uint64_t actionProgramRepresentationID;

    public:

        /**
         * \brief Main ATpgPopulation constructor.
         * 
         * \param[in] outputs outputs of the agents.
         * \param[in] representationID id of the representation used.
         */
        ATpgPopulation(const Output::OutputHandler& outputs, uint64_t representationID) : TpgPopulation(outputs, representationID) {};

        /**
         * \brief Set the id of the action program representation associated with the TPG agents.
         * 
         * \param[in] id the id of the action program representation.
         */
        void setActionProgramRepresentationID(uint64_t id) { this->actionProgramRepresentationID = id; }


        /**
         * \brief Delete the TpgIndividual.
         * 
         * \param[in] agent the Individual to delete.
         * \param[in] graph the Graph associated with the Individual.
         * 
         * \return a shared pointer to the created Individual.
         */
        virtual void emptyAgent(const Individual& agent, EvoGraph::Graph& graph) override;

        /**
         * \brief create and return a TPG execution engine.
         */
        virtual std::unique_ptr<ExecutionEngine> createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources = {}, bool isTraining = false) const override;
    };
}; // namespace Representation::ATPG

#endif // ATPG_AGENT_POPULATION_H
