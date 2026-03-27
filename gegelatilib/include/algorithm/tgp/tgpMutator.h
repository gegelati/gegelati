
#ifndef TGP_MUTATOR_H
#define TGP_MUTATOR_H

#include <vector>
#include <numeric>

#include "algorithm/lgp/lgpMutator.h"
#include "algorithm/tgp/tgpLineMutator.h"

namespace Algorithm::TGP {

    /**
     * \brief Class representing a TGPMutator
     */
    class TGPMutator : public LGP::LGPMutator
    {
    protected:

        
        /// TGPLineMutator used for mutating lines.
        TGPLineMutator tgpLineMutator;

        /// @brief max depth of the tgp
        size_t maxDepth = 4;
        /// @brief max depth of the tgp
        size_t maxInitDepth = 4;

    public:

        /**
         * \brief Main TGPMutator constructor.
         * 
         * \param[in] selector Reference to the current selector used by the algorithm.
         * \param[in] algorithmID id of the algorithm used.
         */
        TGPMutator(const Selector::Selector& selector, uint64_t algorithmID): LGPMutator(selector, algorithmID), tgpLineMutator() {};


        /**
         * \brief Check if the configuration of the mutator is valid according to the given parameters.
         * 
         * This method is called before initializing the population.
         * 
         * \param[in] params the Parameters for the mutation.
         * \param[in] outputs the OutputHandler of the manager.
         */
        virtual bool isConfigurationValid(const Learn::LearningParameters& params, const Output::OutputHandler& outputs) const override;

        /**
         * \brief Initialize a random Agent.
         *
         * \param[in] agent The agent initialized
         * \param[in,out] graph the Graph.
         * \param[in] manager the manager to change the agents.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void initRandomSpecificAgent(const Agent& agent, EvoGraph::Graph& graph, AgentManager& manager, const Learn::LearningParameters& params, RNG::RNG& rng) override;
        

        /**
         * \brief Insert random sub tree
         * 
         * \param[in,out] agent the Agent to mutate.
         * \param[in] destinationIndexLine destination index of the line that should be inserted
         * \param[in] maxDepthTree maximum depth of the sub tree added
         * \param[in] manager the manager to change the agents.
         * \param[in] rng Random Number Generator used in the mutation process.
         *
         */
        virtual void insertRandomSubTree(const LGP::LGPAgent& agent, size_t destinationIndexLine, size_t maxDepthTree, LGP::LGPManager& manager, RNG::RNG& rng);

        /**
         * \brief mutate a specific agent of an algorithm within a population
         * 
         * \param[in,out] agents the Agent to crossover.
         * \param[in,out] graph the graph to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] newSubAgents vector of new agents of sub algorithm created while crossing over the agents
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void crossoverAgents(
            std::array<std::reference_wrapper<const Agent>, 2> agents, EvoGraph::Graph& graph, AgentManager& manager, std::vector<std::reference_wrapper<const Agent>>& newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng
        ) override;

        /**
         * \brief mutate a specific TGPagent of an algorithm within a population
         * 
         * \param[in,out] agent the Agent to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual bool mutateLGPAgent(
            const LGP::LGPAgent& agent, LGP::LGPManager& manager, const Learn::LearningParameters& params, RNG::RNG& rng) override;

        /**
         * \brief Alter a randomly selected Line in a given Program.
         *
         * If the given Program has more than 0 Line, this function selects a
         * Line (pseudo)-randomly in a given Program and calls the
         * Mutator::LineMutator:AlterCorrectLine function on it.
         * Random selection is based on the given RNG::RNG.
         *
         * \param[in,out] agent the Agent to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] rng Random Number Generator used in the mutation process.
         * \return true if a line was successfully altered, false if the
         *         Program has less than one line.
         */
        virtual bool alterRandomLine(const LGP::LGPAgent& agent, LGP::LGPManager& manager, RNG::RNG& rng) override;

        /**
         * \brief destroy the subtree of the corresponding index
         * 
         * \param[in,out] agent the Agent to mutate.
         * \param[in] idxSubTree index of the subtree destroyed
         * \param[in] manager the manager to change the agents.
         */
        virtual void destroySubTree(const LGP::LGPAgent& agent, size_t idxSubTree, LGP::LGPManager& manager);

        /**
         * \brief method returning if the node at the corresponding index is pointing to some sub trees.
         */
        std::array<bool, 2> hasSubTree(const LGP::LGPAgent& agent, size_t idx);

        /**
         * \brief Method returning the index of the line in the agent corresponding to the destination index indicated
         */
        size_t getIndexLineFromDest(const LGP::LGPAgent& agent, size_t destIdx);

        /**
         * \brief return the depth of the destination index
         */
        size_t getNodeDepth(size_t destIndex);

        /**
         * \brief return the real depth of the destination index, meaning the number of actual nodes below this nodes
         */
        size_t getRealNodeDepth(const LGP::LGPAgent& agent, size_t destIndex);

        /**
         * \brief change a random output
         *
         * \param[in,out] agent the Agent to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] location location is the index of the output changed
         * \param[in] rng Random Number Generator used in the mutation process.
         * \return true if the lines where successfully swapped, false if the
         *         Program has less than two lines.
         */
        virtual bool alterRandomOutputs(const LGP::LGPAgent& agent, LGP::LGPManager& manager, size_t location, RNG::RNG& rng) override;

        /**
         * \brief change the index of a node.
         * This method will change the index of the node by the depth index
         * 
         * It will also control if the node(line) as some subtree, and if yes, will change the index too.
         */
        void changeNodeIndex(const LGP::LGPAgent& agent, LGP::LGPManager& manager, size_t lineIndex, size_t destIndex);

    };



}; // namespace Mutator

#endif // TGP_MUTATOR_H
