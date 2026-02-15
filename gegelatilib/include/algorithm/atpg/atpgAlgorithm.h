
#ifndef ATPG_ALGORITHM_H
#define ATPG_ALGORITHM_H

#include <memory>
#include <vector>

#include "archive.h"
#include "algorithm/tpg/tpgAlgorithm.h"
#include "algorithm/atpg/atpgManager.h"
#include "algorithm/atpg/atpgMutator.h"

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

            /// Name of the program algorithm associated with the TPG agents.
            std::string actionProgramAlgorithmName;

        public:


            /**
             * \brief Main Algorithm constructor.
             * 
             * \param[in] params the LearningParameters used by the Algorithm.
             * \param[in] contextProgramAlgorithm the sub-algorithm used to manipulate context programs.
             * \param[in] actionProgramAlgorithm the sub-algorithm used to manipulate action programs.
             * \param[in] algorithmName name of the algorithm used.
             */
            ATPGAlgorithm(const Learn::LearningParameters& params, const Algorithm& contextProgramAlgorithm, const Algorithm& actionProgramAlgorithm, std::string algorithmName = "ATPG")
                : TPGAlgorithm(params, contextProgramAlgorithm, algorithmName){
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
            virtual void initManager(std::shared_ptr<const Output::OutputHandler> outputs) override;

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
            virtual void initSubAlgorithms(RNG::RNG& rng, std::shared_ptr<const Output::OutputHandler> outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph) override;

            
            /**
             * \brief Inherited method to clear all the unused sub agents
             * 
             * The sub agents are the agents used by the program sub-algorithm and the action sub-algorithm.
             */
            virtual std::map<std::string, std::set<std::reference_wrapper<const Agent>>> getUsedSubAgents() const override;

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
