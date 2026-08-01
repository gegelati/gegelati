
#ifndef TGP_MUTATOR_H
#define TGP_MUTATOR_H

#include <vector>
#include <numeric>

#include "representation/lgp/lgpMutator.h"
#include "representation/tgp/tgpLineMutator.h"

namespace Representation::TGP {

    /**
     * \brief Class representing a TGPMutator
     */
    class TGPMutator : public LGP::LGPMutator
    {
    protected:

        
        /// TGPLineMutator used for mutating lines.
        TGPLineMutator tgpLineMutator;

    public:

        /**
         * \brief Main TGPMutator constructor.
         * 
         * \param[in] selector Reference to the current selector used by the representation.
         * \param[in] representationID id of the representation used.
         */
        TGPMutator(const Selector::Selector& selector, uint64_t representationID): LGPMutator(selector, representationID), tgpLineMutator() {};


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
         * \param[in] individual The individual initialized
         * \param[in,out] graph the Graph.
         * \param[in] population the population to change the individuals.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void initRandomSpecificIndividual(const Individual& individual, EvoGraph::Graph& graph, Population& population, const RepresentationParameters& params, RNG::RNG& rng) override;
        

        /**
         * \brief Insert random sub tree
         * 
         * \param[in,out] individual the Individual to mutate.
         * \param[in] destinationIndexLine destination index of the line that should be inserted
         * \param[in] maxDepthTree maximum depth of the sub tree added
         * \param[in] population the population to change the individuals.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         *
         */
        virtual void insertRandomSubTree(const LGP::LgpIndividual& individual, size_t destinationIndexLine, size_t maxDepthTree, LGP::LGPPopulation& population, const RepresentationParameters& params, RNG::RNG& rng);

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
         * \brief mutate a specific TGPindividual of an representation within a population
         * 
         * \param[in,out] individual the Individual to mutate.
         * \param[in] population the population to change the individuals.
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual bool mutateLgpIndividual(
            const LGP::LgpIndividual& individual, LGP::LGPPopulation& population, const RepresentationParameters& params, RNG::RNG& rng) override;

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
        virtual bool alterRandomLine(const LGP::LgpIndividual& individual, LGP::LGPPopulation& population, const RepresentationParameters& params, RNG::RNG& rng) override;

        /**
         * \brief destroy the subtree of the corresponding index
         * 
         * \param[in,out] individual the Individual to mutate.
         * \param[in] idxSubTree index of the subtree destroyed
         * \param[in] population the population to change the individuals.
         */
        virtual void destroySubTree(const LGP::LgpIndividual& individual, size_t idxSubTree, LGP::LGPPopulation& population);

        /**
         * \brief method returning if the node at the corresponding index is pointing to some sub trees.
         */
        std::vector<bool> hasSubTree(const LGP::LgpIndividual& individual, size_t idx);

        /**
         * \brief Method returning the index of the line in the individual corresponding to the destination index indicated
         */
        size_t getIndexLineFromDest(const LGP::LgpIndividual& individual, size_t destIdx);

        /**
         * \brief return the depth of the destination index
         */
        size_t getNodeDepth(size_t destIndex);

        /**
         * \brief return the real depth of the destination index, meaning the number of actual nodes below this nodes
         */
        size_t getRealNodeDepth(const LGP::LgpIndividual& individual, size_t destIndex);

        /**
         * \brief change a random output
         *
         * \param[in,out] individual the Individual to mutate.
         * \param[in] population the population to change the individuals.
         * \param[in] location location is the index of the output changed
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         * \return true if the lines where successfully swapped, false if the
         *         Program has less than two lines.
         */
        virtual bool alterRandomOutputs(const LGP::LgpIndividual& individual, LGP::LGPPopulation& population, size_t location, const RepresentationParameters& params, RNG::RNG& rng) override;

        /**
         * \brief change the index of a node.
         * This method will change the index of the node by the depth index
         * 
         * It will also control if the node(line) as some subtree, and if yes, will change the index too.
         */
        void changeNodeIndex(const LGP::LgpIndividual& individual, LGP::LGPPopulation& population, size_t lineIndex, size_t destIndex);

    };



}; // namespace Mutator

#endif // TGP_MUTATOR_H
