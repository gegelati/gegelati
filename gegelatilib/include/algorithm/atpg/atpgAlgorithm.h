
#ifndef ATPG_ALGORITHM_H
#define ATPG_ALGORITHM_H

#include <memory>
#include <vector>

#include "algorithm/atpg/atpgPolicyStats.h"
#include "algorithm/tpg/tpgAlgorithm.h"
#include "algorithm/atpg/atpgManager.h"
#include "algorithm/atpg/atpgMutator.h"
#include "algorithm/tpg/tpgAgent.h"

#include "learn/learningEnvironment.h"

namespace Algorithm::ATPG {

    /**
     * \brief Abstract class representing an ActionTPGAlgorithm
     * 
     * ActionTPG works as classic TPG, with an addition program set on action vertices to take continuous actions
     */
    class ATPGAlgorithm : public TPG::TPGAlgorithm
    {
        protected:

            /// ID of the program algorithm associated with the TPG agents.
            uint64_t actionProgramAlgorithmID;

        public:


            /**
             * \brief Main Algorithm constructor.
             * 
             * \param[in] contextProgramAlgorithm the sub-algorithm used to manipulate context programs.
             * \param[in] actionProgramAlgorithm the sub-algorithm used to manipulate action programs.
             * \param[in] parameters the LearningParameters used by the Algorithm.
             * \param[in] algorithmName name of the algorithm used.
             * \param[in] algorithmColor name of the algorithm used.
             */
            ATPGAlgorithm(const Algorithm& contextProgramAlgorithm, const Algorithm& actionProgramAlgorithm, std::unique_ptr<AlgorithmParameters> parameters = std::make_unique<AlgorithmParameters>(), std::string algorithmName = "ATPG", std::string algorithmColor = "#335ce2")
                : TPGAlgorithm(contextProgramAlgorithm, std::move(parameters), algorithmName, algorithmColor){
                this->setActionProgramAlgorithm(actionProgramAlgorithm);
            };

            /**
             * \brief Add the action program sub-algorithm to the TPGAlgorithm.
             * 
             */
            void setActionProgramAlgorithm(const Algorithm& programAlgorithm);

            /**
             * \brief Add an aggregated action program algorithm to the TPGAlgorithm.
             * 
             * \param[in] programAlgorithm the aggregated action program algorithm.
             */
            void addAggregatedActionProgramAlgorithm(const Algorithm& programAlgorithm);

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
             * \brief Inherited method to clear all the unused sub agents
             * 
             * The sub agents are the agents used by the program sub-algorithm and the action sub-algorithm.
             */
            virtual std::map<uint64_t, std::set<std::reference_wrapper<const Agent>>> getUsedSubAgents() const override;

            /**
             * \brief Inherited method to create the policy stats of the algorithm
             * 
             * This policy stats contains a program sub policy stats
             */
            virtual std::shared_ptr<PolicyStats> createPolicyStats() const override;


            /**
             * \brief inherrit from algorithm class
             */
            virtual void printCodeGenAgents(std::ofstream& fileMain, std::ofstream& fileMainH, const std::set<std::reference_wrapper<const Agent>>& agents, std::map<uint64_t, std::set<std::reference_wrapper<const Agent>>>& subAgents) const;

            /**
             * Copy and return a uniqure pointer of the algorithm
             */
            virtual std::unique_ptr<Algorithm> copy() const override;
    };
}; // namespace ATPG_Algorithm

namespace Algorithm{
    /**
     * To make the aglrotihm accessible from algorithm namespace
     */
    using ATPGAlgorithm = ATPG::ATPGAlgorithm;
}

#endif
