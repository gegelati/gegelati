
#ifndef LGP_MUTATOR_H
#define LGP_MUTATOR_H

#include <vector>
#include <numeric>

#include "algorithm/mutator.h"
#include "algorithm/lgp/lgpAgent.h"
#include "algorithm/lgp/lgpManager.h"
#include "algorithm/lgp/lgpLineMutator.h"
#include "mutator/programMutator.h"

namespace Algorithm::LGP {

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
         * \param[in] selector Reference to the current selector used by the algorithm.
         */
        LGPMutator(Selector::Selector& selector): Mutator(selector), lineMutator() {};


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
         * \brief Initialize LGP Population.
         *
         * \param[in,out] graph the initialized Graph.
         * \param[in] manager the manager to change the agents.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void initRandomPopulation(std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Learn::LearningParameters& params, RNG::RNG& rng) override;

        /**
         * \brief Initialize a random Agent.
         *
         * \param[in] agent The agent initialized
         * \param[in,out] graph the Graph.
         * \param[in] manager the manager to change the agents.
         * \param[in] params the Parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void initRandomSpecificAgent(std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Learn::LearningParameters& params, RNG::RNG& rng) override;
        


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
            std::vector<std::shared_ptr<const Agent>> agents, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, std::vector<std::shared_ptr<const Agent>>& newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng
        ) override;

        /**
         * \brief mutate a specific agent of an algorithm within a population
         * 
         * \param[in,out] agent the Agent to mutate.
         * \param[in,out] graph the graph to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] newSubAgents vector of new agents of sub algorithm created while mutating the agent
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void mutateAgent(
            std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, std::vector<std::shared_ptr<const Agent>>& newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng
        ) override;


        /**
         * \brief mutate a specific LGPagent of an algorithm within a population
         * 
         * \param[in,out] agent the Agent to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] params Probability parameters for the mutation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual bool mutateLGPAgent(
            std::shared_ptr<const LGPAgent> agent, std::shared_ptr<LGPManager> manager, const Learn::LearningParameters& params, RNG::RNG& rng
        );

        /**
         * \brief Deletes a randomly selected Line of the given Program.
         *
         * Unless a single Line (or less) remains in the given Progeam, this
         * function randomly selects a line of the Program and deletes it.
         * Random selection is based on the given RNG::RNG.
         *
         * \param[in,out] agent the Agent to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] rng Random Number Generator used in the mutation process.
         * \return true if a line could be added, false otherwise.
         *
         */
        bool deleteRandomLine(std::shared_ptr<const LGPAgent> agent, std::shared_ptr<LGPManager> manager, RNG::RNG& rng);

        /**
         * \brief Insert a new Line at a randomly selected position within the
         * given Program.
         *
         * This function randomly selects a position in the Program.lines and
         * insert a randomly initialized line (using Mutator::Line::
         * initRandomCorrectLine).
         * Random selection is based on the given RNG::RNG.
         *
         * \param[in,out] agent the Agent to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] rng Random Number Generator used in the mutation process.
         *
         */
        void insertRandomLine(std::shared_ptr<const LGPAgent> agent, std::shared_ptr<LGPManager> manager, RNG::RNG& rng);

        /**
         * \brief Swap two randomly selected instructions within the given
         * Program.
         *
         * This function selects two lines of the program randomly and swaps
         * them. If the given Program has less than two lines, nothing happens.
         * Random selection is based on the given RNG::RNG.
         *
         * \param[in,out] agent the Agent to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] rng Random Number Generator used in the mutation process.
         * \return true if the lines where successfully swapped, false if the
         *         Program has less than two lines.
         */
        bool swapRandomLines(std::shared_ptr<const LGPAgent> agent, std::shared_ptr<LGPManager> manager, RNG::RNG& rng);

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
        bool alterRandomLine(std::shared_ptr<const LGPAgent> agent, std::shared_ptr<LGPManager> manager, RNG::RNG& rng);

        /**
         * \brief Alter a program's constant.
         *
         * If the constants are used, this function selects one of them
         * in a pseudo-random way and modifies it
         * Random selection is based on the given RNG::RNG.
         *
         * \param[in,out] agent the Agent to mutate.
         * \param[in] manager the manager to change the agents.
         * \param[in] params the mutation parameters
         * \param[in] rng Random Number Generator used in the mutation process.
         * \return true if a constant was successfully altered, false if the
         *         Program has less than one line.
         */
        bool alterRandomConstant(std::shared_ptr<const LGPAgent> agent, std::shared_ptr<LGPManager> manager,
                                 const Learn::LearningParameters& params,
                                 RNG::RNG& rng);

        /**
         * \brief Specialization of mutateSubAgents method.
         * Doesnt do anything for LGP as LGP doesnt have subAgents.
         */
        void mutateSubAgents(
            std::vector<std::shared_ptr<const Agent>>& agents, std::shared_ptr<EvoGraph::Graph> graph, 
            std::shared_ptr<AgentManager> manager, const Learn::LearningParameters& params, 
            RNG::RNG& rng, uint64_t maxNbThreads) override {};
    };



}; // namespace Mutator

#endif // LGP_MUTATOR_H
