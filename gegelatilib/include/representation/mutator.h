
#ifndef R_MUTATOR_H
#define R_MUTATOR_H

#include <vector>
#include <string>
#include <array>

#include "representation/individual.h"
#include "representation/population.h"
#include "mutator/rng.h"
#include "representation/repParameters.h"
#include "selector/selector.h"
#include "evoGraph/graph.h"

namespace Representation {
    /**
     * \brief Abstract class representing a Mutator used by an Representation.
     * 
     * Available representations are TPG, MAPLE, and LGP
     */
    class Mutator
    {
    protected:


        /// Name of the representation.
        uint64_t representationID;

        /// Sub-mutators for sub-representations
        std::map<uint64_t, std::reference_wrapper<Mutator>> subMutators;

        /// Pointer to the current context used by the mutator to populate the Graph.
        std::unique_ptr<Selector::SelectionContext> currentContext;

        /// Reference to the current context used by the mutator to populate the Graph.
        std::reference_wrapper<const Selector::Selector> selector;

    public:

        /// Default polymorphic destructor
        virtual ~Mutator() = default;

        // Disable copying to avoid accidental copies (use references or pointers instead).
        Mutator(const Mutator&) = delete;
        Mutator& operator=(const Mutator&) = delete;
    
        /**
         * \brief Main Mutator constructor.
         * 
         * \param[in] selector Reference to the current selector used by the representation.
         * \param[in] representationID id of the representation used.
         */
        Mutator(const Selector::Selector& selector, uint64_t representationID) : selector(selector), representationID{representationID} {};

        /**
         * \brief Update the context used by the TPGMutator to populate the Graph.
         * 
         * \param[in] graph the Graph.
         * \param[in] population the population to change the individuals.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void updateSpecificContext(
            EvoGraph::Graph& graph, Population& population,
            const RepresentationParameters& params,
            RNG::RNG& rng);

        /**
         * \brief Return a reference to the current context
         */
        virtual const Selector::SelectionContext& getContext();

        /**
         * \brief Return the id of the representation.
         */
        uint64_t getRepresentationID() const { return this->representationID; }

        
        /**
         * \brief Add a sub-mutator to the current mutator.
         * 
         * \param[in] subMutator the sub-mutator to add.
         */
        virtual void addSubMutator(Mutator& subMutator);


        /**
         * \brief return the subMutator corresponding to the name of the representation given.
         * 
         * \param[in] representationID id of the representation given.
         */
        virtual Mutator& getSubMutator(uint64_t representationID);


        /**
         * \brief Check if the configuration of the mutator is valid according to the given parameters.
         * 
         * This method should be call before initializing the population.
         * 
         * \param[in] params the Parameters for the mutation.
         * \param[in] outputs the OutputHandler of the population.
         */
        virtual bool isConfigurationValid(const RepresentationParameters& params, const Output::OutputHandler& outputs) const = 0;

        /**
         * \brief Initialize a the action vertices.
         * 
         * This methods created only the action vertices that does not already exist in the graph
         * 
         * The action vertices are based on the outputs attribute of the population. 
         * If the outputs contain a single discrete output, an action vertex for each takeable output value is created.
         * If the outputs contain multiple discrete or continuous outputs, an action vertex is created for each output.
         * 
         * However, Gegelati currently does not handle environment with discrete and continuous outputs. 
         * 
         * \param[in,out] graph the initialized Graph.
         * \param[in] nbActionVertices number of individuals to create.
         */
        virtual std::vector<std::reference_wrapper<const EvoGraph::Action>> initActionVertices(EvoGraph::Graph& graph, size_t nbActionVertices);

        /**
         * \brief Initialize a random population.
         *
         * \param[in,out] graph the initialized Graph.
         * \param[in] population the population to change the individuals.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void initRandomPopulation(EvoGraph::Graph& graph, Population& population, const RepresentationParameters& params, RNG::RNG& rng) = 0;

        /**
         * \brief Initialize a random Individual.
         *
         * \param[in,out] graph the Graph.
         * \param[in] population the population to change the individuals.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual const Individual& initRandomIndividual(EvoGraph::Graph& graph, Population& population, const RepresentationParameters& params, RNG::RNG& rng);

        
        /**
         * \brief Initialize a random Individual.
         *
         * \param[in] individual initialized.
         * \param[in,out] graph the Graph.
         * \param[in] population the population to change the individuals.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void initRandomSpecificIndividual(
            const Individual& individual,
            EvoGraph::Graph& graph,
            Population& population,
            const RepresentationParameters& params, RNG::RNG& rng) = 0;

        /**
         * \brief mutate the whole population, by dupplicating and adding new individuals from the current representation.
         * 
         * \param[in] graph the graph to mutate.
         * \param[in] population the population to change the individuals.
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         * \param[in] maxNbThreads Integer parameter controlling the number of
         * threads used for parallel execution. Possible values are:
         *   - default:  Let the runtime decide using
         *               std::thread::hardware_concurrency().
         *   - `0` and `1`: Do not use parallelism.
         *   - `n > 1`: Set the number of threads explicitly.
         */
        virtual void mutatePopulation(
            EvoGraph::Graph& graph, Population& population,
            const RepresentationParameters& params,
            RNG::RNG& rng, uint64_t maxNbThreads = std::thread::hardware_concurrency());
        /**
         * \brief mutate a specific individual of an representation within a population
         * 
         * \param[in,out] individuals the Individual to crossover.
         * \param[in,out] graph the graph to mutate.
         * \param[in] population the population to change the individuals.
         * \param[in] newSubIndividuals vector of new individuals of sub representation created while crossing over the individuals
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void crossoverIndividuals(
            std::array<std::reference_wrapper<const Individual>, 2> individuals, EvoGraph::Graph& graph, Population& population, std::vector<std::reference_wrapper<const Individual>>& newSubIndividuals, const RepresentationParameters& params, RNG::RNG& rng
        ) {};

        /**
         * \brief mutate a specific individual of an representation within a population
         * 
         * \param[in,out] individual the Individual to mutate.
         * \param[in,out] graph the graph to mutate.
         * \param[in] population the population to change the individuals.
         * \param[in] newSubIndividuals vector of new individuals of sub representation created while mutating the individual
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void mutateIndividual(
            const Individual& individual, EvoGraph::Graph& graph, Population& population, std::vector<std::reference_wrapper<const Individual>>& newSubIndividuals, const RepresentationParameters& params, RNG::RNG& rng) = 0;

        /**
         * \brief mutate new sub individuals of sub representations created during the evolution process of the current representation
         * 
         * \param[in] individuals vector of new individuals of sub representation created while mutating the individual
         * \param[in,out] graph the graph to mutate.
         * \param[in] population the population to change the individuals.
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         * \param[in] maxNbThreads Integer parameter controlling the number of
         * threads used for parallel execution. Possible values are:
         *   - default:  Let the runtime decide using
         *               std::thread::hardware_concurrency().
         *   - `0` and `1`: Do not use parallelism.
         *   - `n > 1`: Set the number of threads explicitly.
         */
        virtual void mutateSubIndividuals(std::vector<std::reference_wrapper<const Individual>>& individuals, EvoGraph::Graph& graph, Population& population, const RepresentationParameters& params, RNG::RNG& rng, uint64_t maxNbThreads) {};
    };
}; // namespace Mutator

#endif
