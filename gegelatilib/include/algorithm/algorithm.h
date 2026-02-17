
#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <memory>
#include <vector>
#include <string>

#include "algorithm/job.h"
#include "algorithm/agent.h"
#include "algorithm/agentManager.h"
#include "algorithm/mutator.h"
#include "algorithm/policyStats.h"
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
        std::vector<std::unique_ptr<Algorithm>> subAlgorithms;
        /// Name of the algorithm.
        std::string algorithmName;

        /// aggregated algorithms
        std::vector<std::reference_wrapper<const Algorithm>> aggregatedAlgorithms;
        
        /**
         * \brief return the subAlgorithm corresponding to the name of the algorithm given.
         * 
         * \param[in] nameAlgorithm name of the algorithm given.
         */
        Algorithm& getSubAlgorithm(std::string nameAlgorithm);     

        /**
         * \brief return the subAlgorithm corresponding to the name of the algorithm given.
         * 
         * \param[in] nameAlgorithm name of the algorithm given.
         */
        const Algorithm& cGetSubAlgorithm(std::string nameAlgorithm) const;     

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
        void addSubAlgorithm(const Algorithm& subAlgorithm);

        /**
         * \brief Method that aggregate another algorithm to this algorithm.
         * 
         * The algorithm need to be the same type.
         * This access allows for the manager to dupplicate an agent from the aggregated algorithm to its own agents.
         * 
         * \param[in] aggregatedAlgorithm the algorithm to aggregate.
         */
        void addAggregatedAlgorithm(const Algorithm& aggregatedAlgorithm);

        /// Constant getter for the graph
        virtual std::shared_ptr<const EvoGraph::Graph> getGraph() const;

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
      
        /// Constant getter for the mutator
        virtual std::vector<std::reference_wrapper<const Algorithm>> getSubAlgorithms() const;

        /**
         * \brief Get the current number of agents used by the algorithm.
         */
        virtual size_t getNbAgents() const;

        /**
         * \brief Get the current agents used by the algorithm.
         */
        virtual const std::vector<std::reference_wrapper<const Agent>> getAgents () const;

        /**
         * \brief method that indicate if the algorithm contains a specific agent.
         * 
         * \param[in] agent searched agent.
         */
        virtual bool containsAgent(const Agent& agent) const;

        /**
         * \brief Initialize the managerof the algorithm
         * 
         * \param[in] outputs outputs needed for the algorithm.
         */
        virtual void initManager(std::shared_ptr<const Output::OutputHandler> outputs) = 0;

        /**
         * \brief Initialize the mutator of the algorithm
         */
        virtual void initMutator() = 0;

        /**
         * \brief Initialize the sub-algorithms of the algorithm
         * 
         * \param[in] rng deterministic random generator
         * \param[in] outputs outputs needed for the algorithm.
         * \param[in] dataSource input sources of the algorithm.
         * \param[in] graph the EvoGraph::Graph used by the algorithm.
         */
        virtual void initSubAlgorithms(RNG::RNG& rng, std::shared_ptr<const Output::OutputHandler> outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph);

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
         * \brief Clear the algorithm of all its content.
         */
        virtual void clearAlgorithm();

        /**
         * Copy and return a uniqure pointer of the algorithm
         */
        virtual std::unique_ptr<Algorithm> copy() const = 0;

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
        * \brief Get the agents that are currently used by the algorithm.
        * The returned map associate to each sub-algorithm name the set of agents used by this sub-algorithm.
        */
        virtual std::map<std::string, std::set<std::reference_wrapper<const Agent>>> getUsedSubAgents() const;

        /**
         * \brief Clear all the unused sub agents
         */
        virtual void clearUnusedSubAgents();

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
        virtual std::shared_ptr<Job> createJob(const Agent& agent, Learn::LearningMode mode, RNG::RNG& rng, int idx = 0) const;

        /**
         * \brief Create a PolicyStats object corresponding to the algorithm.
          * 
          * This method is used for the analysis of the policies, and the
          * returned PolicyStats object will be filled during the analyses.
          * The type of the returned PolicyStats object need to correspond to
          * the type of the algorithm, for example, a TPGAlgorithm should return
          * a TPGPolicyStats.
         */
        virtual std::shared_ptr<PolicyStats> createPolicyStats() const = 0;

        /**
         * \brief Update the algorithm after evaluation of a set of jobs.
         * 
         * \param[in] jobs the jobs that have been evaluated.
         * \param[in] mode the mode of the training, determining for example
         * if we generate values that we only need for training.
         */
        virtual void updateAfterEvaluation(const std::vector<std::shared_ptr<Job>>& jobs, Learn::LearningMode mode);

        /**
         * \brief Print the content of an agent in a file.
         * 
         * \param[in] agent the agent to print.
         * \param[in] pFile the file in which the content of the agent will be printed.
         * \param[in] offset the character chain used to control the indentation of the printed content
         * \param[in] printedAgentID the set of already printed agent IDs to avoid printing the same agent twice in case of multiple vertices or edges using the same agent program.
         * \param[in] elementsToPrint the list of elements to print, filled during this method.
         */
        virtual void printAgent(const Agent& agent, FILE* pFile, std::string offset, std::set<uint64_t>& printedAgentID, std::vector<std::reference_wrapper<const EvoGraph::Element>>& elementsToPrint) const = 0;

    };

    /**
     * \brief Comparison function to enable sorting of Algorithm with
     * STL.
     */
    bool operator==(const Algorithm& a, const Algorithm& b);
}; // namespace Algorithm

#endif // ALGORITHM_H
