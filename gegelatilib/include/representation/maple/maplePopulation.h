
#ifndef MAPLE_INDIVIDUAL_POPULATION_H
#define MAPLE_INDIVIDUAL_POPULATION_H

#include "representation/maple/mapleIndividual.h"
#include "representation/maple/mapleExecutionEngine.h"

namespace Representation::Maple {


    /**
     * \brief Class representing a MaplePopulation used by the MapleRepresentation.
     * 
     * The Population is in charge of storing, creating, copying or removing Individuals.
     * Basically, the population is the interface between the Representation and the Graph.
     */
    class MaplePopulation : public TPG::TpgPopulation
    {

    protected:

        /**
         * \brief Get the MapleIndividual from a const Individual pointer.
         * 
         * \param[in] individual the Individual to cast.
         */
        virtual MapleIndividual& getMapleIndividualFromCst(const Individual& individual);
    public:

        /**
         * \brief Main MaplePopulation constructor.
         * 
         * \param[in] outputs outputs of the individuals.
         * \param[in] representationID id of the representation used.
         */
        MaplePopulation(const Output::OutputHandler& outputs, uint64_t representationID) : TpgPopulation(outputs, representationID) {};


        /**
         * \brief Get the current individuals used by the representation.
         */
        virtual const std::vector<std::reference_wrapper<const Individual>> getIndividuals() const override;

        /**
         * \brief Create a new MapleIndividual on a specific vertex.
         * 
         * \param[in] vertex the vertex associated with the Individual.
         * 
         * \return a shared pointer to the created Individual.
         */
        virtual const Individual& createIndividual(std::optional<std::reference_wrapper<const EvoGraph::Vertex>> vertex) override;

        /**
         * \brief create and return a Maple execution engine.
         */
        virtual std::unique_ptr<ExecutionEngine> createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources = {}, bool isTraining = false) const override;
    };
}; // namespace Representation::MAPLE

#endif // MAPLE_INDIVIDUAL_POPULATION_H
