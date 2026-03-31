
#ifndef TPG_ALGORITHM_H
#define TPG_ALGORITHM_H

#include <memory>
#include <vector>

#include "algorithm/tpg/archive.h"
#include "algorithm/algorithm.h"
#include "algorithm/tpg/tpgPolicyStats.h"
#include "algorithm/tpg/tpgManager.h"
#include "algorithm/tpg/tpgMutator.h"
#include "algorithm/tpg/tpgAgent.h"
#include "algorithm/tpg/tpgJob.h"

#include "learn/learningEnvironment.h"

namespace Algorithm::TPG {

    /**
     * \brief Abstract class representing a TPGAlgorithm
     */
    class TPGAlgorithm : public Algorithm
    {
        protected:

            /// ID of the program algorithm associated with the TPG agents.
            uint64_t programAlgorithmID;

            /// TPGArchive used during the training process
            std::unique_ptr<TPGArchive> archive;

        public:

            /**
             * \brief Main Algorithm constructor.
             * 
             * \param[in] programAlgorithm the sub-algorithm used to manipulate programs.
             * \param[in] parameters the LearningParameters used by the Algorithm.
             * \param[in] algorithmName name of the algorithm used.
             * \param[in] algorithmColor name of the algorithm used.
             */
            TPGAlgorithm(const Algorithm& programAlgorithm, std::unique_ptr<AlgorithmParameters> parameters = std::make_unique<AlgorithmParameters>(), std::string algorithmName = "TPG", std::string algorithmColor = "#A0FF33")
                : Algorithm(std::move(parameters), algorithmName, algorithmColor) {
                archive = std::make_unique<TPGArchive>(this->params->tpg.archiveSize, this->params->tpg.archivingProbability);
                this->setProgramAlgorithm(programAlgorithm);
            };

            /**
             * \brief Add the program sub-algorithm to the TPGAlgorithm.
             * 
             */
            void setProgramAlgorithm(const Algorithm& programAlgorithm);


            /**
             * \brief Get the TPGArchive used by the LGPAlgorithm.
             */
            const TPGArchive& getArchive() const;
            
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
             * \brief Inherited method to print a TPGagent.
             * 
             * The TPG agent prints the vertex it points to.
             */
            void printAgent(const Agent& agent, FILE* pFile, std::string offset, std::set<uint64_t>& printedAgentID, std::vector<std::reference_wrapper<const EvoGraph::Element>>& elementsToPrint) const override;

            /**
             * \brief Inherited method to read a TPGAgent.
             */
            virtual const Agent& readAgent(std::smatch& matches) override;

            /**
             * \brief inherrit from algorithm class
             */
            virtual void printCodeGenAgents(std::ofstream& fileMain, std::ofstream& fileMainH, const std::set<std::reference_wrapper<const Agent>>& agents, std::map<uint64_t, std::set<std::reference_wrapper<const Agent>>>& subAgents) const;

            /**
             * \brief Link an agent to a corresponding vertex
             * 
             * \param[in] agent the agent linked to the vertex.
             * \param[in] vertex the vertex linked to the agent.
             */
            virtual void linkAgentVertex(const Agent& agent, const EvoGraph::Vertex& vertex) override;
            
    };
}; // namespace TPG_Algorithm

namespace Algorithm{
    /**
     * To make the aglrotihm accessible from algorithm namespace
     */
    using TPGAlgorithm = TPG::TPGAlgorithm;
}

#endif
