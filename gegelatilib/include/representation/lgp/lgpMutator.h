
#ifndef LGP_MUTATOR_H
#define LGP_MUTATOR_H

#include <vector>
#include <numeric>

#include "representation/mutator.h"
#include "representation/lgp/lgpIndividual.h"
#include "representation/lgp/lgpPopulation.h"
#include "representation/lgp/lgpLineMutator.h"

namespace Representation::LGP {

    /**
     * \brief Class representing a LGPMutator
     */
    class LGPMutator : public Mutator
    {
    protected:

        /// LGPLineMutator used for mutating LGP lines.
        LGPLineMutator lineMutator;

    public:

        /**
         * \brief Main LGPMutator constructor.
         * 
         * \param[in] selector Reference to the current selector used by the representation.
         * \param[in] representationID id of the representation used.
         */
        LGPMutator(const Selector::Selector& selector, uint64_t representationID): Mutator(selector, representationID), lineMutator() {};


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
         * \brief Initialize LGP Population.
         *
         * \param[in,out] graph the initialized Graph.
         * \param[in] population the population to change the individuals.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void initRandomPopulation(EvoGraph::Graph& graph, Population& population, const RepresentationParameters& params, RNG::RNG& rng) override;

        /**
         * \brief Initialize a random Individual.
         *
         * \param[in] individual The individual initialized
         * \param[in,out] graph the Graph.
         * \param[in] population the population to change the individuals.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void initRandomSpecificIndividual(const Individual& individual, EvoGraph::Graph& graph, Population& population, const RepresentationParameters& params, RNG::RNG& rng) override;
        


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
        ) override;

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
            const Individual& individual, EvoGraph::Graph& graph, Population& population, std::vector<std::reference_wrapper<const Individual>>& newSubIndividuals, const RepresentationParameters& params, RNG::RNG& rng
        ) override;


        /**
         * \brief mutate a specific LgpIndividual of an representation within a population
         * 
         * \param[in,out] individual the Individual to mutate.
         * \param[in] population the population to change the individuals.
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual bool mutateLgpIndividual(
            const LgpIndividual& individual, LGPPopulation& population, const RepresentationParameters& params, RNG::RNG& rng
        );

        /**
         * \brief Deletes a randomly selected Line of the given Program.
         *
         * Unless a single Line (or less) remains in the given Progeam, this
         * function randomly selects a line of the Program and deletes it.
         * Random selection is based on the given RNG::RNG.
         *
         * \param[in,out] individual the Individual to mutate.
         * \param[in] population the population to change the individuals.
         * \param[in] rng Random Number Generator used in the mutation process.
         * \return true if a line could be added, false otherwise.
         *
         */
        virtual bool deleteRandomLine(const LgpIndividual& individual, LGPPopulation& population, RNG::RNG& rng);

        /**
         * \brief Insert a new Line at a randomly selected position within the
         * given Program.
         *
         * This function randomly selects a position in the Program.lines and
         * insert a randomly initialized line (using Mutator::Line::
         * initRandomCorrectLine).
         * Random selection is based on the given RNG::RNG.
         *
         * \param[in,out] individual the Individual to mutate.
         * \param[in] population the population to change the individuals.
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         *
         */
        virtual void insertRandomLine(const LgpIndividual& individual, LGPPopulation& population, const RepresentationParameters& params, RNG::RNG& rng);

        /**
         * \brief Swap two randomly selected instructions within the given
         * Program.
         *
         * This function selects two lines of the program randomly and swaps
         * them. If the given Program has less than two lines, nothing happens.
         * Random selection is based on the given RNG::RNG.
         *
         * \param[in,out] individual the Individual to mutate.
         * \param[in] population the population to change the individuals.
         * \param[in] rng Random Number Generator used in the mutation process.
         * \return true if the lines where successfully swapped, false if the
         *         Program has less than two lines.
         */
        virtual bool swapRandomLines(const LgpIndividual& individual, LGPPopulation& population, RNG::RNG& rng);

        /**
         * \brief Alter a randomly selected Line in a given Program.
         *
         * If the given Program has more than 0 Line, this function selects a
         * Line (pseudo)-randomly in a given Program and calls the
         * Mutator::LineMutator:AlterCorrectLine function on it.
         * Random selection is based on the given RNG::RNG.
         *
         * \param[in,out] individual the Individual to mutate.
         * \param[in] population the population to change the individuals.
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         * \return true if a line was successfully altered, false if the
         *         Program has less than one line.
         */
        virtual bool alterRandomLine(const LgpIndividual& individual, LGPPopulation& population, const RepresentationParameters& params, RNG::RNG& rng);

        /**
         * \brief Alter a program's constant.
         *
         * If the constants are used, this function selects one of them
         * in a pseudo-random way and modifies it
         * Random selection is based on the given RNG::RNG.
         *
         * \param[in,out] individual the Individual to mutate.
         * \param[in] population the population to change the individuals.
         * \param[in] params the mutation parameters
         * \param[in] rng Random Number Generator used in the mutation process.
         * \return true if a constant was successfully altered, false if the
         *         Program has less than one line.
         */
        virtual bool alterRandomConstant(const LgpIndividual& individual, LGPPopulation& population,
                                 const RepresentationParameters& params,
                                 RNG::RNG& rng);

        /**
         * \brief change a random output
         *
         * \param[in,out] individual the Individual to mutate.
         * \param[in] population the population to change the individuals.
         * \param[in] location location is the index of the output changed
         * \param[in] params the mutation parameters
         * \param[in] rng Random Number Generator used in the mutation process.
         * \return true if the lines where successfully swapped, false if the
         *         Program has less than two lines.
         */
        virtual bool alterRandomOutputs(const LgpIndividual& individual, LGPPopulation& population, size_t location, const RepresentationParameters& params, RNG::RNG& rng);
    };



}; // namespace Mutator

#endif // LGP_MUTATOR_H
