
#ifndef CGP_MUTATOR_H
#define CGP_MUTATOR_H

#include <vector>
#include <numeric>

#include "representation/lgp/lgpMutator.h"
#include "representation/cgp/cgpLineMutator.h"

namespace Representation::CGP {

    /**
     * \brief Class representing a CGPMutator
     */
    class CGPMutator : public LGP::LGPMutator
    {
    protected:

        
        /// CGPLineMutator used for mutating lines.
        CGPLineMutator cgpLineMutator;

    public:

        /**
         * \brief Main CGPMutator constructor.
         * 
         * \param[in] selector Reference to the current selector used by the representation.
         * \param[in] representationID id of the representation used.
         */
        CGPMutator(const Selector::Selector& selector, uint64_t representationID): LGPMutator(selector, representationID), cgpLineMutator() {};


        /**
         * \brief Check if the configuration of the mutator is valid according to the given parameters.
         * 
         * This method is called before initializing the population.
         * 
         * \param[in] params the Parameters for the mutation.
         * \param[in] outputs the OutputHandler of the population.
         */
        virtual bool isConfigurationValid(const RepresentationParameters& params, const Output::OutputHandler& outputs) const override;

        /**
         * \brief Initialize a random Individual.
         *
         * \param[in] agent The agent initialized
         * \param[in,out] graph the Graph.
         * \param[in] population the population to change the agents.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void initRandomSpecificAgent(const Individual& agent, EvoGraph::Graph& graph, Population& population, const RepresentationParameters& params, RNG::RNG& rng) override;
        

        /**
         * \brief Inherrit from LGP mutator
         */
        virtual void insertRandomLine(const LGP::LgpIndividual& agent, LGP::LGPPopulation& population, const RepresentationParameters& params, RNG::RNG& rng) override;

        /**
         * \brief mutate a specific agent of an representation within a population
         * 
         * \param[in,out] agents the Individual to crossover.
         * \param[in,out] graph the graph to mutate.
         * \param[in] population the population to change the agents.
         * \param[in] newSubAgents vector of new agents of sub representation created while crossing over the agents
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void crossoverAgents(
            std::array<std::reference_wrapper<const Individual>, 2> agents, EvoGraph::Graph& graph, Population& population, std::vector<std::reference_wrapper<const Individual>>& newSubAgents, const RepresentationParameters& params, RNG::RNG& rng
        ) override;

        /**
         * \brief mutate a specific CGPagent of an representation within a population
         * 
         * \param[in,out] agent the Individual to mutate.
         * \param[in] population the population to change the agents.
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual bool mutateLgpIndividual(
            const LGP::LgpIndividual& agent, LGP::LGPPopulation& population, const RepresentationParameters& params, RNG::RNG& rng) override;

        /**
         * \brief Alter a randomly selected Line in a given Program.
         *
         * If the given Program has more than 0 Line, this function selects a
         * Line (pseudo)-randomly in a given Program and calls the
         * Mutator::LineMutator:AlterCorrectLine function on it.
         * Random selection is based on the given RNG::RNG.
         *
         * \param[in,out] agent the Individual to mutate.
         * \param[in] lineIndex index of the line mutated.
         * \param[in] population the population to change the agents.
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         * \return true if a line was successfully altered, false if the
         *         Program has less than one line.
         */
        virtual bool alterRandomlyLine(const LGP::LgpIndividual& agent, size_t lineIndex, LGP::LGPPopulation& population, const RepresentationParameters& params, RNG::RNG& rng);

    };



}; // namespace Mutator

#endif // CGP_MUTATOR_H
