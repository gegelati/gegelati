
#ifndef REPRESENTATION_H
#define REPRESENTATION_H

#include <memory>
#include <vector>
#include <string>
#include <regex>

#include "representation/job.h"
#include "representation/individual.h"
#include "representation/population.h"
#include "representation/mutator.h"
#include "representation/policyStats.h"
#include "representation/repParameters.h"
#include "evoGraph/graph.h"
#include "selector/selectorFactory.h"

#include "outputInfo.h"
namespace Representation {
    /**
     * \brief Abstract class representing an Representation.
     *
     * Available representations are TPG, MAPLE, and LGP
     */
    class Representation
    {
      protected:

        /// Graph used by the representation
        std::shared_ptr<EvoGraph::Graph> graph;

        /// Graph used by the representation
        std::unique_ptr<Population> population;

        /// Parameters used by the representation
        std::unique_ptr<RepresentationParameters> params;

        /// Pointer pointing to the default selector created if the representation is created without a defined selector
        std::unique_ptr<Selector::Selector> savedDefaultSelector;

        /// Selector used for the selection process
        std::optional<std::reference_wrapper<Selector::Selector>> selector;

        /// Mutator used by the representation
        std::unique_ptr<Mutator> mutator;

        /// Output informations
        std::unique_ptr<const Output::OutputHandler> outputs;

        /// Sub-representations used by the representation
        std::vector<std::unique_ptr<Representation>> subRepresentations;
        /// Name of the representation.
        std::string representationName;
        /// Color of the representation.
        std::string representationColor;

        /// aggregated representations
        std::vector<std::reference_wrapper<const Representation>> aggregatedRepresentations;
        
        /// Boolean to indicate if the representation has been initialize
        bool init = false;

        /**
         * \brief return the subRepresentation corresponding to the id of the representation given.
         * 
         * \param[in] representationID id of the sub representation given.
         */
        Representation& getSubRepresentation(uint64_t representationID);     
        
        
        /// Unique ID of the representation.
        uint64_t representationID;

        /**
         * \brief Incremente the representation ID counter and return the new value.
         */
        static uint64_t incrementeCounter();

        /**
         * \brief Reset the representation ID counter.
         *
         * This method set the ID counter to a new value.
         * It can quickly lead to segmentation fault if not used carefully.
         */
        static void resetRepresentationIDCounter();
        friend struct ::CounterReset;

      public:

      
        /// Default polymorphic destructor
        virtual ~Representation() = default;

        // Disable copying to avoid accidental copies (use references or pointers instead).
        Representation(const Representation&) = delete;
        Representation& operator=(const Representation&) = delete;

        /**
         * \brief Main Representation constructor.
         * 
         * \param[in] parameters the LearningParameters used by the Representation.
         * \param[in] representationName name of the representation used.
         * \param[in] representationColor color of the representation used (during .dot files).
         */
        Representation(std::unique_ptr<RepresentationParameters> parameters = std::make_unique<RepresentationParameters>(), std::string representationName = "Representation", std::string representationColor = "#000000")
               : params{std::make_unique<RepresentationParameters>(*parameters)}, representationName(representationName), representationColor(representationColor), representationID(incrementeCounter()) {};

        /**
         * \brief Return the name of the representation.
         */
        std::string getRepresentationName() const { return this->representationName; }
        /**
         * \brief Return the color of the representation.
         */
        std::string getRepresentationColor() const { return this->representationColor; }

        /**
         * \brief return the ID of the agent.
         */
        static uint64_t getRepresentationIDCounter();

        /**
         * \brief Get the unique identifier of the Individual.
         *
         * \return the integer ID of the Individual.
         */
        virtual uint64_t getRepresentationID() const;

        /**
         * \brief Set a new unique identifier to the Individual.
         *
         * \param[in] newID the new integer ID to set to the Individual.
         */
        virtual void setRepresentationID(uint64_t newID);

        /**
         * \brief Add a sub-representation to the current representation.
         * 
         * \param[in] subRepresentation the sub-representation to add.
         */
        void addSubRepresentation(const Representation& subRepresentation);

        /**
         * Getter for init status.
         */
        bool isInit() const;

        /**
         * \brief Method that aggregate another representation to this representation.
         * 
         * The representation need to be the same type.
         * This access allows for the population to dupplicate an agent from the aggregated representation to its own agents.
         * 
         * \param[in] aggregatedRepresentation the representation to aggregate.
         */
        void addAggregatedRepresentation(const Representation& aggregatedRepresentation);

        /**
         * \brief return the aggregated representations
         */
        const std::vector<std::reference_wrapper<const Representation>>& getAggregatedRepresentations() const;

        /**
         * \brief return the subRepresentation corresponding to the id of the representation given.
         * 
         * \param[in] representationID id of the sub representation given.
         */
        const Representation& cGetSubRepresentation(uint64_t representationID) const;    

        /**
         * \brief return the specified aggregated representation
         * 
         * \param[in] representationID ID of the aggregated representation
         */
        const Representation& getAggregatedRepresentation(uint64_t representationID) const;

        /// Constant getter for the graph
        virtual const EvoGraph::Graph& getGraph() const;

        /// Constant getter for the population
        virtual const Population& getPopulationCst() const;

        /// Constant getter for the selector
        virtual const Selector::Selector& getSelectorCst() const;

        /// Getter for the population
        virtual Population& getPopulation();

        /// Indicate if the representation possess a selector 
        virtual bool hasSelector() const;

        /**
         * \brief set a selector to the representation
         * 
         * \param[in] selector selector set
         */
        virtual void setSelector(Selector::Selector& selector);

        /// Getter for the selector
        virtual Selector::Selector& getSelector();

        /// Getter for the mutator
        virtual Mutator& getMutator();
      
        /// Constant getter for the sub representations
        virtual std::vector<std::reference_wrapper<const Representation>> cGetSubRepresentations() const;
      
        /// Getter for the sub representations
        virtual std::vector<std::reference_wrapper<Representation>> getSubRepresentations();

        /**
         * \brief Get the current number of agents used by the representation.
         */
        virtual size_t getNbAgents() const;

        /**
         * \brief Get the current agents used by the representation.
         */
        virtual const std::vector<std::reference_wrapper<const Individual>> getAgents () const;

        /**
         * \brief method that indicate if the representation contains a specific agent.
         * 
         * \param[in] agent searched agent.
         */
        virtual bool containsAgent(const Individual& agent) const;

        /**
         * \brief Initialize the populationof the representation
         */
        virtual void initPopulation() = 0;

        
        /**
         * \brief Initialize the selector of the representation
         */
        virtual void initSelector();

        /**
         * \brief Initialize the mutator of the representation
         */
        virtual void initMutator() = 0;

        /**
         * \brief Initialize the sub-representations of the representation
         * 
         * \param[in] rng deterministic random generator
         * \param[in] outputs outputs needed for the representation.
         * \param[in] dataSource input sources of the representation.
         * \param[in] graph the EvoGraph::Graph used by the representation.
         */
        virtual void initSubRepresentations(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph);

        /**
         * Initialize the representation
         * 
         * \param[in] rng deterministic random generator
         * \param[in] outputs outputs needed for the representation.
         * \param[in] dataSource input sources of the representation.
         * \param[in] graph the EvoGraph::Graph used by the representation.
         */
        virtual void initRepresentation(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph);
        

        /**
         * \brief get the outputs
         */
        virtual const Output::OutputHandler& getOutputs() const;

        /**
         * \brief get the params
         */
        virtual const RepresentationParameters& getParams() const;

        /**
         * \brief Clear the representation of all its content.
         */
        virtual void clearRepresentation();

        /**
         * Copy and return a uniqure pointer of the representation
         */
        virtual std::unique_ptr<Representation> copy() const = 0;

        /**
         * Initialize the population of the representation
         * 
         * \param[in] rng deterministic random generator
         */
        virtual void initPopulation(RNG::RNG& rng);

        /**
         * \brief Method to start the population process of the representation
         * 
         * \param[in] rng deterministic random generator
         * \param[in] maxNbThreads maximum number of threads
         */
        virtual void populate(RNG::RNG& rng, size_t maxNbThreads);




        /**
        * \brief Get the agents that are currently used by the representation.
        * The returned map associate to each sub-representation id the set of agents used by this sub-representation.
        */
        virtual std::map<uint64_t, std::set<std::reference_wrapper<const Individual>>> getUsedSubAgents() const;

        /**
         * \brief Clear all the unused sub agents
         */
        virtual void clearUnusedSubAgents();

        /**
         * \brief Clear all the parts of agents that are not used, such as introns for LGPs
         */
        virtual void clearUnusedAgentParts() = 0;

        /**
         * \brief Takes a given Individual and creates a job containing it.
         *
         * \param[in] agent the Individual to be evaluated.
         * \param[in] mode the mode of the training, determining for example
         * if we generate values that we only need for training.
         * \param[in] rng deterministic random generator
         * \param[in] idx The index of the job, can be used to organize a map
         * for example.
         *
         * \return A job representing the agent.
         */
        virtual std::shared_ptr<Job> createJob(const Individual& agent, Learn::LearningMode mode, RNG::RNG& rng, int idx = 0) const;

        /**
         * \brief Create a PolicyStats object corresponding to the representation.
          * 
          * This method is used for the analysis of the policies, and the
          * returned PolicyStats object will be filled during the analyses.
          * The type of the returned PolicyStats object need to correspond to
          * the type of the representation, for example, a TPGRepresentation should return
          * a TPGPolicyStats.
         */
        virtual std::shared_ptr<PolicyStats> createPolicyStats() const = 0;

        /**
         * \brief Update the representation after evaluation of a set of jobs.
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
        virtual void printAgent(const Individual& agent, FILE* pFile, std::string offset, std::set<uint64_t>& printedAgentID, std::vector<std::reference_wrapper<const EvoGraph::Element>>& elementsToPrint) const = 0;

        /**
         * \brief Read and create an agent.
         * 
         * The pFile can be use to read the next lines that could be used by an representation.
         * 
         * If the agent normally uses a Team, it should be created empty.
         * The Team will be added in "linkAgentTeam" method.
         * 
         * \param[in] matches the match of the regex line.
         */
        virtual const Individual& readAgent(std::smatch& matches) = 0;

        /**
         * \brief Link an agent to a corresponding vertex
         * 
         * This method should be override if it is intended to be used, the basic implementation throw.
         * 
         * \param[in] agent the agent linked to the vertex.
         * \param[in] vertex the vertex linked to the agent.
         */
        virtual void linkAgentVertex(const Individual& agent, const EvoGraph::Vertex& vertex);

        /**
         * \brief Export the corresponding C code of the representation.
         */
        virtual void exportBestAgentCodeGen(const std::string& filename = "",
                           const std::string& path = "./");

        /**
         * \brief Export the corresponding C code of the representation.
         */
        virtual void exportSpecificAgentCodeGen(const Individual& agent, const std::string& filename = "",
                           const std::string& path = "./");

        /**
         * \brief Export the corresponding C code of the representation.
         */
        virtual void exportSpecificAgentsCodeGen(std::set<std::reference_wrapper<const Individual>> agents, const std::string& filename = "",
                           const std::string& path = "./");

        /**
         * \brief Export the corresponding dot file of the representation, and its sub representations
         */
        virtual void exportDotFile(const char* filePath);

        /**
         * \brief Export the corresponding dot file of the representation, and its sub representations
         */
        virtual void exportBestAgentDotFile(const char* filePath);

        /**
         * \brief Export the corresponding dot file of the representation, and its sub representations
         */
        virtual void exportSpecificAgentDotFile(const Individual& agent, const char* filePath);


        /**
         * \brief specific exporting of an agent for the code generation
         */
        virtual void printCodeGenAgents(std::ofstream& fileMain, std::ofstream& fileMainH, const std::set<std::reference_wrapper<const Individual>>& agents, std::map<uint64_t, std::set<std::reference_wrapper<const Individual>>>& subAgents) const = 0;

        /**
         * \brief Import the corresponding file
         */
        virtual void importDotFile(const char* filePath);

    };
    /**
     * \brief Comparison function to enable sorting of Representation with
     * STL.
     */
    bool operator<(const Representation& a, const Representation& b);

    /**
     * \brief Comparison function to enable sorting of Representation with
     * STL.
     */
    bool operator==(const Representation& a, const Representation& b);

    /**
     * \brief Comparison function to enable sorting of Representation with
     * STL.
     */
    bool operator!=(const Representation& a, const Representation& b);
}; // namespace Representation

#endif // REPRESENTATION_H
