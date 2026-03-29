
#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <memory>
#include <vector>
#include <string>
#include <regex>

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
        std::unique_ptr<AgentManager> manager;

        /// Parameters used by the algorithm
        Learn::LearningParameters params;

        /// Pointer pointing to the default selector created if the algorithm is created without a defined selector
        std::unique_ptr<Selector::Selector> savedDefaultSelector;

        /// Selector used for the selection process
        std::optional<std::reference_wrapper<Selector::Selector>> selector;

        /// Mutator used by the algorithm
        std::unique_ptr<Mutator> mutator;

        /// Output informations
        std::unique_ptr<const Output::OutputHandler> outputs;

        /// Sub-algorithms used by the algorithm
        std::vector<std::unique_ptr<Algorithm>> subAlgorithms;
        /// Name of the algorithm.
        std::string algorithmName;
        /// Color of the algorithm.
        std::string algorithmColor;

        /// aggregated algorithms
        std::vector<std::reference_wrapper<const Algorithm>> aggregatedAlgorithms;
        
        /// Boolean to indicate if the algorithm has been initialize
        bool init = false;

        /**
         * \brief return the subAlgorithm corresponding to the id of the algorithm given.
         * 
         * \param[in] algorithmID id of the sub algorithm given.
         */
        Algorithm& getSubAlgorithm(uint64_t algorithmID);     
        
        
        /// Unique ID of the algorithm.
        uint64_t algorithmID;

        /**
         * \brief Incremente the algorithm ID counter and return the new value.
         */
        static uint64_t incrementeCounter();

        /**
         * \brief Reset the algorithm ID counter.
         *
         * This method set the ID counter to a new value.
         * It can quickly lead to segmentation fault if not used carefully.
         */
        static void resetAlgorithmIDCounter();
        friend struct ::CounterReset;

      public:

      
        /// Default polymorphic destructor
        virtual ~Algorithm() = default;

        // Disable copying to avoid accidental copies (use references or pointers instead).
        Algorithm(const Algorithm&) = delete;
        Algorithm& operator=(const Algorithm&) = delete;

        /**
         * \brief Main Algorithm constructor.
         * 
         * \param[in] params the LearningParameters used by the Algorithm.
         * \param[in] algorithmName name of the algorithm used.
         * \param[in] algorithmColor color of the algorithm used (during .dot files).
         */
        Algorithm(const Learn::LearningParameters& params, std::string algorithmName = "Algorithm", std::string algorithmColor = "#000000")
               : params{params}, algorithmName(algorithmName), algorithmColor(algorithmColor), algorithmID(incrementeCounter()) {};

        /**
         * \brief Return the name of the algorithm.
         */
        std::string getAlgorithmName() const { return this->algorithmName; }
        /**
         * \brief Return the color of the algorithm.
         */
        std::string getAlgorithmColor() const { return this->algorithmColor; }

        /**
         * \brief return the ID of the agent.
         */
        static uint64_t getAlgorithmIDCounter();

        /**
         * \brief Get the unique identifier of the Agent.
         *
         * \return the integer ID of the Agent.
         */
        virtual uint64_t getAlgorithmID() const;

        /**
         * \brief Set a new unique identifier to the Agent.
         *
         * \param[in] newID the new integer ID to set to the Agent.
         */
        virtual void setAlgorithmID(uint64_t newID);

        /**
         * \brief Add a sub-algorithm to the current algorithm.
         * 
         * \param[in] subAlgorithm the sub-algorithm to add.
         */
        void addSubAlgorithm(const Algorithm& subAlgorithm);

        /**
         * Getter for init status.
         */
        bool isInit() const;

        /**
         * \brief Method that aggregate another algorithm to this algorithm.
         * 
         * The algorithm need to be the same type.
         * This access allows for the manager to dupplicate an agent from the aggregated algorithm to its own agents.
         * 
         * \param[in] aggregatedAlgorithm the algorithm to aggregate.
         */
        void addAggregatedAlgorithm(const Algorithm& aggregatedAlgorithm);

        /**
         * \brief return the aggregated algorithms
         */
        const std::vector<std::reference_wrapper<const Algorithm>>& getAggregatedAlgorithms() const;

        /**
         * \brief return the subAlgorithm corresponding to the id of the algorithm given.
         * 
         * \param[in] algorithmID id of the sub algorithm given.
         */
        const Algorithm& cGetSubAlgorithm(uint64_t algorithmID) const;    

        /**
         * \brief return the specified aggregated algorithm
         * 
         * \param[in] algorithmID ID of the aggregated algorithm
         */
        const Algorithm& getAggregatedAlgorithm(uint64_t algorithmID) const;

        /// Constant getter for the graph
        virtual const EvoGraph::Graph& getGraph() const;

        /// Constant getter for the manager
        virtual const AgentManager& getManagerCst() const;

        /// Constant getter for the selector
        virtual const Selector::Selector& getSelectorCst() const;

        /// Getter for the manager
        virtual AgentManager& getManager();

        /// Indicate if the algorithm possess a selector 
        virtual bool hasSelector() const;

        /**
         * \brief set a selector to the algorithm
         * 
         * \param[in] selector selector set
         */
        virtual void setSelector(Selector::Selector& selector);

        /// Getter for the selector
        virtual Selector::Selector& getSelector();

        /// Getter for the mutator
        virtual Mutator& getMutator();
      
        /// Constant getter for the sub algorithms
        virtual std::vector<std::reference_wrapper<const Algorithm>> cGetSubAlgorithms() const;
      
        /// Getter for the sub algorithms
        virtual std::vector<std::reference_wrapper<Algorithm>> getSubAlgorithms();

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
         */
        virtual void initManager() = 0;

        
        /**
         * \brief Initialize the selector of the algorithm
         */
        virtual void initSelector();

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
        virtual void initSubAlgorithms(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph);

        /**
         * Initialize the algorithm
         * 
         * \param[in] rng deterministic random generator
         * \param[in] outputs outputs needed for the algorithm.
         * \param[in] dataSource input sources of the algorithm.
         * \param[in] graph the EvoGraph::Graph used by the algorithm.
         */
        virtual void initAlgorithm(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph);
        

        /**
         * \brief get the outputs
         */
        virtual const Output::OutputHandler& getOutputs() const;

        /**
         * \brief get the params
         */
        virtual const Learn::LearningParameters& getParams() const;

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
        * The returned map associate to each sub-algorithm id the set of agents used by this sub-algorithm.
        */
        virtual std::map<uint64_t, std::set<std::reference_wrapper<const Agent>>> getUsedSubAgents() const;

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

        /**
         * \brief Read and create an agent.
         * 
         * The pFile can be use to read the next lines that could be used by an algorithm.
         * 
         * If the agent normally uses a Team, it should be created empty.
         * The Team will be added in "linkAgentTeam" method.
         * 
         * \param[in] matches the match of the regex line.
         */
        virtual const Agent& readAgent(std::smatch& matches) = 0;

        /**
         * \brief Link an agent to a corresponding vertex
         * 
         * This method should be override if it is intended to be used, the basic implementation throw.
         * 
         * \param[in] agent the agent linked to the vertex.
         * \param[in] vertex the vertex linked to the agent.
         */
        virtual void linkAgentVertex(const Agent& agent, const EvoGraph::Vertex& vertex);

        /**
         * \brief Export the corresponding C code of the algorithm.
         */
        virtual void exportBestAgentCodeGen(const std::string& filename = "",
                           const std::string& path = "./");

        /**
         * \brief Export the corresponding C code of the algorithm.
         */
        virtual void exportSpecificAgentCodeGen(const Agent& agent, const std::string& filename = "",
                           const std::string& path = "./");

        /**
         * \brief Export the corresponding C code of the algorithm.
         */
        virtual void exportSpecificAgentsCodeGen(std::set<std::reference_wrapper<const Agent>> agents, const std::string& filename = "",
                           const std::string& path = "./");

        /**
         * \brief Export the corresponding dot file of the algorithm, and its sub algorithms
         */
        virtual void exportDotFile(const char* filePath);

        /**
         * \brief Export the corresponding dot file of the algorithm, and its sub algorithms
         */
        virtual void exportBestAgentDotFile(const char* filePath);

        /**
         * \brief Export the corresponding dot file of the algorithm, and its sub algorithms
         */
        virtual void exportSpecificAgentDotFile(const Agent& agent, const char* filePath);


        /**
         * \brief specific exporting of an agent for the code generation
         */
        virtual void printCodeGenAgents(std::ofstream& fileMain, std::ofstream& fileMainH, const std::set<std::reference_wrapper<const Agent>>& agents, std::map<uint64_t, std::set<std::reference_wrapper<const Agent>>>& subAgents) const = 0;

        /**
         * \brief Import the corresponding file
         */
        virtual void importDotFile(const char* filePath);

    };
    /**
     * \brief Comparison function to enable sorting of Algorithm with
     * STL.
     */
    bool operator<(const Algorithm& a, const Algorithm& b);

    /**
     * \brief Comparison function to enable sorting of Algorithm with
     * STL.
     */
    bool operator==(const Algorithm& a, const Algorithm& b);

    /**
     * \brief Comparison function to enable sorting of Algorithm with
     * STL.
     */
    bool operator!=(const Algorithm& a, const Algorithm& b);
}; // namespace Algorithm

#endif // ALGORITHM_H
