
#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <memory>
#include <vector>
#include <string>

#include "algorithm/job.h"
#include "algorithm/agent.h"
#include "algorithm/agentManager.h"
#include "algorithm/mutator.h"
#include "learn/learningParameters.h"
#include "evoGraph/graph.h"
#include "selector/selectorFactory.h"

#include "outputInfo.h"
namespace Algorithm {
    /**
     * \brief Abstract class representing an Algorithm.
     *
     * Available algorithms are TPG, MAPLE, and LGP
     */
    class Algorithm
    {
      protected:

        /// Graph used by the algorithm
        std::shared_ptr<EvoGraph::Graph> graph;

        /// Graph used by the algorithm
        std::shared_ptr<AgentManager> manager;

        /// Parameters used by the algorithm
        const Learn::LearningParameters& params;

        /// Selector used for the selection process
        std::shared_ptr<Selector::Selector> selector;
        /// Mutator used by the algorithm
        std::shared_ptr<Mutator> mutator;

        /// Output informations
        std::shared_ptr<const Output::OutputHandler> outputs;

        /// Sub-algorithms used by the algorithm
        std::vector<std::shared_ptr<Algorithm>> subAlgorithms;
        /// Name of the algorithm.
        std::string algorithmName;
        
        /**
         * \brief return the subAlgorithm corresponding to the name of the algorithm given.
         * 
         * \param[in] nameAlgorithm name of the algorithm given.
         */
        std::shared_ptr<Algorithm> getSubAlgorithm(std::string nameAlgorithm);     

        /**
         * \brief return the subAlgorithm corresponding to the name of the algorithm given.
         * 
         * \param[in] nameAlgorithm name of the algorithm given.
         */
        std::shared_ptr<const Algorithm> cGetSubAlgorithm(std::string nameAlgorithm) const;     
      public:

        /**
         * \brief Main Algorithm constructor.
         * 
         * \param[in] params the LearningParameters used by the Algorithm.
         * \param[in] algorithmName name of the algorithm used.
         * 
         */
        Algorithm(const Learn::LearningParameters& params, std::string algorithmName)
               : params{params}, algorithmName(algorithmName) {
              };

        /**
         * \brief Return the name of the algorithm.
         */
        std::string getAlgorithmName() const { return this->algorithmName; }

        /**
         * \brief Add a sub-algorithm to the current algorithm.
         * 
         * \param[in] subAlgorithm the sub-algorithm to add.
         */
        void addSubAlgorithm(std::shared_ptr<Algorithm> subAlgorithm);


        /// Constant getter for the manager
        virtual std::shared_ptr<const AgentManager> getManagerCst() const;

        /// Constant getter for the selector
        virtual std::shared_ptr<const Selector::Selector> getSelectorCst() const;

        /// Getter for the manager
        virtual std::shared_ptr<AgentManager> getManager();

        /// Getter for the selector
        virtual std::shared_ptr<Selector::Selector> getSelector();

        /// Getter for the mutator
        virtual std::shared_ptr<Mutator> getMutator();
      
        /**
         * \brief Get the current number of agents used by the algorithm.
         */
        virtual size_t getNbAgents() const;

        /**
         * \brief Get the current agents used by the algorithm.
         */
        virtual const std::vector<std::shared_ptr<const Agent>> getAgents () const;

        /**
         * \brief method that indicate if the algorithm contains a specific agent.
         * 
         * \param[in] agent searched agent.
         */
        virtual bool containsAgent(std::shared_ptr<const Agent> agent) const;

        /**
         * Initialize the algorithm
         * 
         * \param[in] rng deterministic random generator
         * \param[in] outputs outputs needed for the algorithm.
         * \param[in] dataSource input sources of the algorithm.
         * \param[in] graph the EvoGraph::Graph used by the algorithm.
         */
        virtual void initAlgorithm(RNG::RNG& rng, std::shared_ptr<const Output::OutputHandler> outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph);

        /**
         * Initialize the population of the algorithm
         * 
         * \param[in] rng deterministic random generator
         */
        virtual void initPopulation(RNG::RNG& rng);

        /**
         * \brief Method to start the population process of the algorithm
         * 
         * \param[in] rng deterministic random generator
         * \param[in] maxNbThreads maximum number of threads
         */
        virtual void populate(RNG::RNG& rng, size_t maxNbThreads);



        /**
         * \brief Method executing an Agent and outputting values.
         * 
         * \param[in] agent The agent which is evaluated.
         */
        virtual std::vector<double> executeAgent(std::shared_ptr<const Agent> agent) const = 0;

        /**
         * \brief Clear all the parts of agents that are not used, such as introns for LGPs
         */
        virtual void clearUnusedAgentParts() = 0;

        /**
         * \brief Takes a given Agent and creates a job containing it.
         *
         * \param[in] agent the Agent to be evaluated.
         * \param[in] mode the mode of the training, determining for example
         * if we generate values that we only need for training.
         * \param[in] rng deterministic random generator
         * \param[in] idx The index of the job, can be used to organize a map
         * for example.
         *
         * \return A job representing the agent.
         */
        virtual std::shared_ptr<Job> createJob(std::shared_ptr<const Agent> agent, Learn::LearningMode mode, RNG::RNG& rng, int idx = 0) const;

        /**
         * \brief active the current job.
         * 
         * \param[in] job current job to active
         */
        virtual void activeJob(Job& job);


    };
}; // namespace Algorithm

#endif // ALGORITHM_H
