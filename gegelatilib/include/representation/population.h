
#ifndef POPULATION_H
#define POPULATION_H

#include <vector>
#include <memory>
#include <ranges>
#include <string>

#include "evoGraph/graph.h"
#include "representation/individual.h"
#include "representation/executionEngine.h"
#include "util/genericComparator.h"
#include "outputInfo.h"

namespace Representation {
    /**
     * \brief Abstract class representing an Population used by an Representation.
     * 
     * The Population is in charge of storing, creating, copying or removing Individuals.
     * Basically, the population is the interface between the Representation and the Graph.
     * 
     * Available representations are TPG, MAPLE, and LGP
     */
    class Population
    {
    protected:

        /// Current individuals used by the representation
        std::set<std::unique_ptr<Individual>, UniqueLess<Individual>> individuals;

        /// Sub-populations for sub-representations
        std::map<uint64_t, std::reference_wrapper<Population>> subPopulations;

        /// Id of the representation.
        uint64_t representationID;

        /// Number of outputs of the individuals
        const Output::OutputHandler& outputs;

        /// Populations of aggregated representations
        std::vector<std::reference_wrapper<const Population>> aggregatedPopulations;

        /**
         * \brief Get the Individual from a const Individual pointer.
         * 
         * \param[in] individual the Individual to cast.
         */
        virtual std::set<std::unique_ptr<Individual>>::iterator getIndividualFromCst(const Individual& individual);

    public:

    
        /// Default polymorphic destructor
        virtual ~Population() = default;

        // Disable copying to avoid accidental copies (use references or pointers instead).
        Population(const Population&) = delete;
        Population& operator=(const Population&) = delete;

        /**
         * Constructor for individual population
         * 
         * \param[in] outputs outputs of the individuals
         * \param[in] representationID id of the representation used.
         */
        Population(const Output::OutputHandler& outputs, uint64_t representationID) : outputs{outputs}, representationID{representationID} {}

        /**
         * \brief Get the current individuals used by the representation.
         */
        virtual const std::vector<std::reference_wrapper<const Individual>> getIndividuals() const;

        /**
         * \brief Return the outputs of the individuals.
         */
        virtual const Output::OutputHandler& getOutputs() const {return outputs; };

        /**
         * \brief Add a sub-population to the current population.
         * 
         * \param[in] subPopulation the sub-population to add.
         */
        virtual void addSubPopulation(Population& subPopulation);

        /**
         * \brief Method that add an access from this population to another population.
         * 
         * The population need to be the same type.
         * This access allows for the population to dupplicate an individual from the accessed population to its own individuals.
         * 
         * \param[in] populationAggregated the population to access.
         */
        virtual void addAggregatedPopulation(const Population& populationAggregated);

        /**
         * \brief return the subPopulation corresponding to the name of the representation given.
         * 
         * \param[in] representationID name of the representation given.
         */
        virtual Population& getSubPopulation(uint64_t representationID);

        /**
         * \brief return the subPopulation corresponding to the name of the representation given.
         * 
         * \param[in] representationID name of the representation given.
         */
        virtual const Population& cGetSubPopulation(uint64_t representationID) const;

        /**
         * \brief return the aggregated population corresponding to the name of the representation given.
         * 
         * \param[in] representationID name of the representation given.
         */
        virtual const Population& getAggregatedPopulation(uint64_t representationID) const;

        /**
         * \brief return the aggregated populations of this population
         */
        virtual const std::vector<std::reference_wrapper<const Population>>& getAggregatedPopulations() const;

        /**
         * \brief Return the id of the representation.
         */
        uint64_t getRepresentationID() const { return this->representationID; }

        /**
         * \brief method that indicate if the population contains a specific individual.
         * 
         * \param[in] individual searched individual.
         */
        virtual bool containsIndividual(const Individual& individual) const;

        /**
         * \brief method that indicate if the individual is accessible by the population.
         * 
         * An individual is accessible by the population if it is created by the population or by one of its aggregated populations.
         * 
         * \param[in] individual searched individual.
         */
        virtual bool isIndividualAccessible(const Individual& individual) const;

        /**
         * \brief Create a new Individual of the type used by the current representation.
         * 
         * \param[in] graph the Graph associated with the Individual.
         * 
         * \return a shared pointer to the created Individual.
         */
        virtual const Individual& createIndividual(EvoGraph::Graph& graph) = 0;

        /**
         * \brief Copy a new Individual of the type used by the current representation.
         * 
         * \param[in] individual the Individual to copy.
         * \param[in] graph the Graph associated with the Individual.
         * 
         * \return a shared pointer to the created Individual.
         */
        virtual const Individual& copyIndividual(const Individual& individual, EvoGraph::Graph& graph) = 0;

        /**
         * \brief Create a new Individual of the type used by the current representation.
         * 
         * \param[in] individual the Individual to delete.
         * \param[in] graph the Graph associated with the Individual.
         * 
         * \return a shared pointer to the created Individual.
         */
        virtual void deleteIndividual(const Individual& individual, EvoGraph::Graph& graph);

        /**
         * \brief Empty an Individual of the type used by the current representation.
         * 
         * \param[in] individual the Individual to empty.
         * \param[in] graph the Graph associated with the Individual.
         */
        virtual void emptyIndividual(const Individual& individual, EvoGraph::Graph& graph) = 0;

        /**
         * \brief Clear all individuals from the population.
         * 
         * \param[in] graph the Graph associated with the Individuals.
         */
        virtual void clearIndividuals(EvoGraph::Graph& graph);

        /**
         * \brief Create the execution engine associated with the representation.
         * 
         * \param[in] dataSources the data sources to use for the execution engine.
         * \param[in] isTraining Boolean indicating if this executionEngine will be executed for training or testing purpose.
         * 
         * \return a shared pointer to the created execution engine.
         */
        virtual std::unique_ptr<ExecutionEngine> createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources = {}, bool isTraining = false) const = 0;

        /**
         * \brief Set a new ID to an individual
         *
         * An error is thrown if the individual does not belong to the population
         * An error is thrown if the newID is already used
         *
         * \param[in] individual the individual to change ID
         * \param[in] newID the new ID to set
         */
        virtual void setNewIndividualID(const Individual& individual, uint64_t newID);
    };
}; // namespace Representation

#endif // INDIVIDUAL_POPULATION_H
