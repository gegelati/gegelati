
#ifndef TPG_AGENT_POPULATION_H
#define TPG_AGENT_POPULATION_H

#include "representation/population.h"
#include "representation/tpg/tpgIndividual.h"
#include "representation/tpg/tpgExecutionEngine.h"

namespace Representation::TPG {


    /**
     * \brief Class representing a TpgPopulation used by the TPGRepresentation.
     * 
     * The Population is in charge of storing, creating, copying or removing Agents.
     * Basically, the population is the interface between the Representation and the Graph.
     */
    class TpgPopulation : public Population
    {

    protected:
    

        /// id of the program representation associated with the TPG agents.
        uint64_t programRepresentationID;


        /**
         * \brief Get the TpgIndividual from a const Individual pointer.
         * 
         * \param[in] agent the Individual to cast.
         */
        virtual TpgIndividual& getTpgIndividualFromCst(const Individual& agent);

    public:

        /**
         * \brief Main TpgPopulation constructor.
         * 
         * \param[in] outputs outputs of the agents.
         * \param[in] representationID id of the representation used.
         */
        TpgPopulation(const Output::OutputHandler& outputs, uint64_t representationID) : Population(outputs, representationID) {};

        /**
         * \brief Set the id of the program representation associated with the TPG agents.
         * 
         * \param[in] id the id of the program representation.
         */
        void setProgramRepresentationID(uint64_t id) { this->programRepresentationID = id; }

        /**
         * \brief Get the current agents used by the representation.
         */
        virtual const std::vector<std::reference_wrapper<const Individual>> getAgents() const override;

        /**
         * \brief Create a new TpgIndividual.
         * 
         * \param[in] graph the Graph associated with the Individual.
         * 
         * \return the created Individual.
         */
        virtual const Individual& createAgent(EvoGraph::Graph& graph) override;

        /**
         * \brief Create a new TpgIndividual on a specific vertex.
         * The vertex used can only be a EvoGraph::Team
         * 
         * \param[in] vertex the vertex associated with the Individual.
         * 
         * \return the created Individual.
         */
        virtual const Individual& createAgent(std::optional<std::reference_wrapper<const EvoGraph::Vertex>> vertex);

        /**
         * \brief Create a new TpgIndividual with no vertex. 
         * This method is made only for readAgent during loading a .dot file.
         * 
         * \return the created Individual.
         */
        virtual const Individual& createEmptyAgent();

        /**
         * \brief Copy a TpgIndividual.
         * 
         * if element is not a nullptr, a copy of the team of the agent is done. 
         * 
         * \param[in] agent the Individual to copy.
         * \param[in] graph the Graph associated with the Individual.
         * 
         * \return the created Individual.
         */
        virtual const Individual& copyAgent(const Individual& agent, EvoGraph::Graph& graph) override;

        /**
         * \brief Delete the TpgIndividual.
         * 
         * \param[in] agent the Individual to delete.
         * \param[in] graph the Graph associated with the Individual.
         */
        virtual void deleteAgent(const Individual& agent, EvoGraph::Graph& graph) override;

        /**
         * \brief Empty a TpgIndividual of its program.
         * 
         * \param[in] agent the Individual to empty.
         * \param[in] graph the Graph associated with the Individual.
         */
        virtual void emptyAgent(const Individual& agent, EvoGraph::Graph& graph) override;

        /**
         * \brief Set the vertex of an agent.
         * 
         * \param[in] agent the Individual to delete.
         * \param[in] vertex the vertex set to the agent.
         */
        virtual void setVertex(const Individual& agent, const EvoGraph::Vertex& vertex);

        /**
         * \brief create and return a TPG execution engine.
         */
        virtual std::unique_ptr<ExecutionEngine> createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources = {}, bool isTraining = false) const override;
    };
}; // namespace Representation

#endif // TPG_AGENT_POPULATION_H
