
#ifndef SPECIES_ALGORITHM_H
#define SPECIES_ALGORITHM_H

#include <memory>
#include <vector>

#include "algorithm/tpg/archive.h"
#include "algorithm/algorithm.h"
#include "algorithm/species/speciesPolicyStats.h"
#include "algorithm/species/speciesManager.h"
#include "algorithm/species/speciesMutator.h"
#include "algorithm/species/speciesAgent.h"
#include "algorithm/species/speciesJob.h"


namespace Algorithm::Species {

    /**
     * \brief Abstract class representing a SpeciesAlgorithm
     */
    class SpeciesAlgorithm : public Algorithm
    {
        protected:

            /// ID of the program algorithm associated with the Species agents.
            uint64_t programAlgorithmID;

            /// Archive used during the training process
            std::unique_ptr<Archive> archive;

            /// Vertex from which the species algorithm starts.
            std::optional<std::reference_wrapper<const EvoGraph::Vertex>> rootVertex;


        public:

            /**
             * \brief Main Algorithm constructor.
             * 
             * \param[in] params the LearningParameters used by the Algorithm.
             * \param[in] programAlgorithm the sub-algorithm used to manipulate programs.
             * \param[in] algorithmName name of the algorithm used.
             * \param[in] algorithmColor name of the algorithm used.
             */
            SpeciesAlgorithm(const Learn::LearningParameters& params, const Algorithm& programAlgorithm, std::string algorithmName = "Species", std::string algorithmColor = "#ff10d3")
                : Algorithm(params, algorithmName, algorithmColor), archive{std::make_unique<Archive>(params.archiveSize, params.archivingProbability)} {
                this->setProgramAlgorithm(programAlgorithm);
            };


            /**
             * \brief init a new species from this current species.
             */
            std::unique_ptr<Algorithm> initNewSpecies(RNG::RNG& rng);

            /// Indicate if manager contains a root vertex
            bool hasRootVertex() const;

            /// @brief Getter for the root vertex 
            const EvoGraph::Vertex& getRootVertex() const;

            /**
             * \brief setter for the root vertex
             * 
             * \param[in] newRootVertex
             */
            void setRootVertex(const EvoGraph::Vertex& newRootVertex);

            /**
             * \brief calculate the porportion allowed to the algorithm
             */
            double calculateProportion(int position, int totalAlgos);
            
            /**
             * \brief Add the program sub-algorithm to the SpeciesAlgorithm.
             * 
             */
            void setProgramAlgorithm(const Algorithm& programAlgorithm);

            
            /**
             * \brief Clear the algorithm of all its content.
             */
            virtual void clearAlgorithm() override;

            /**
             * \brief Get the Archive used by the LGPAlgorithm.
             */
            const Archive& getArchive() const;
            
            /**
             * \brief Clear all the parts of agents that are not used, such as introns for LGPs
             */
            virtual void clearUnusedAgentParts() override {};


            /**
             * \brief Initialize the managerof the algorithm
             * 
             * \param[in] outputs outputs needed for the algorithm.
             */
            virtual void initManager() override;

            /**
             * \brief Initialize the mutator of the algorithm
             */
            virtual void initMutator() override;

            /**
             * Override of the initial method.
             * Will call the Algorithm::initAlgorithm method, and then create the species structure.
             * If the root is not already set, it will be a single team link to a single action.
             */
            virtual void initAlgorithm(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph) override;

            /**
             * \brief Initialize the sub-algorithms of the algorithm
             * 
             * \param[in] rng deterministic random generator
             * \param[in] outputs outputs needed for the algorithm.
             * \param[in] dataSource input sources of the algorithm.
             * \param[in] graph the EvoGraph::Graph used by the algorithm.
             */
            virtual void initSubAlgorithms(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph) override;


            /**
             * Copy and return a uniqure pointer of the algorithm
             */
            virtual std::unique_ptr<Algorithm> copy() const override;

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
            virtual std::shared_ptr<Job> createJob(const Agent& agent, Learn::LearningMode mode, RNG::RNG& rng, int idx = 0) const override;

            /**
             * \brief Inherited method to create the policy stats of the algorithm
             * 
             * This policy stats contains a program sub policy stats
             */
            virtual std::shared_ptr<PolicyStats> createPolicyStats() const override;
            
            /**
             * \brief Inherited method to update the algorithm after evaluation of a set of jobs.
             * 
             * Does the merging of the archives after evaluation.
             */
            virtual void updateAfterEvaluation(const std::vector<std::shared_ptr<Job>>& jobs, Learn::LearningMode mode) override;

            /**
             * \brief Inherited method to clear all the unused sub agents
             * 
             * The sub agents are the agents used by the program sub-algorithm.
             */
            virtual std::map<uint64_t, std::set<std::reference_wrapper<const Agent>>> getUsedSubAgents() const override;

            /**
             * Print the species structure.
             */
            virtual void initialPrint(FILE* pFile, std::string offset, std::vector<std::reference_wrapper<const EvoGraph::Element>>& elementsToPrint) const override;

            /** 
             * \brief Inherited method to print a Speciesagent.
             * 
             * The Species agent prints the vertex it points to.
             */
            void printAgent(const Agent& agent, FILE* pFile, std::string offset, std::set<uint64_t>& printedAgentID, std::vector<std::reference_wrapper<const EvoGraph::Element>>& elementsToPrint, std::vector<std::reference_wrapper<const Agent>>& agentsToPrint) const override;

            /**
             * \brief Inherited method to read a SpeciesAgent.
             */
            virtual const Agent& readAgent(std::smatch& matches) override;
            
    };
}; // namespace SPECIES_Algorithm

namespace Algorithm{
    /**
     * To make the aglrotihm accessible from algorithm namespace
     */
    using SpeciesAlgorithm = Species::SpeciesAlgorithm;
}

#endif
