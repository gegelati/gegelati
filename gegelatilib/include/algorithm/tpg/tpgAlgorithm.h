
#ifndef TPG_ALGORITHM_H
#define TPG_ALGORITHM_H

#include <memory>
#include <vector>

#include "algorithm/algorithm.h"
#include "algorithm/tpg/tpgManager.h"
#include "algorithm/tpg/tpgMutator.h"
#include "algorithm/tpg/tpgAgent.h"
#include "algorithm/lgp/lgpAlgorithm.h"


namespace Algorithm::TPG {

    /**
     * \brief Abstract class representing a TPGAlgorithm
     */
    class TPGAlgorithm : public Algorithm
    {
        protected:

            /// Name of the program algorithm associated with the TPG agents.
            std::string programAlgorithmName;

        public:

            /**
             * \brief Main Algorithm constructor.
             * 
             * \param[in] graph graph used by the learning agent
             * \param[in] params the LearningParameters used by the Algorithm.
             * \param[in] nbOutputs number of outputs that will be usable for
             * interacting with this LearningEnviromnent.
             * \param[in] iSet the Instruction Set used by the LGPAlgorithm.
             * \param[in] dataSources the DataSources used by the LGPAlgorithm.
             * \param[in] algorithmName name of the algorithm used.
             */
            TPGAlgorithm(std::shared_ptr<EvoGraph::Graph> graph, const Learn::LearningParameters& params, size_t nbOutputs, const Instructions::Set& iSet, std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources, std::string algorithmName = "TPG")
                : Algorithm(graph, params, std::make_shared<TPGManager>(), std::make_shared<TPGMutator>(), nbOutputs, algorithmName) {
                this->addLGPAlgorithm(graph, params, nbOutputs, iSet, dataSources);
            };

            /**
             * \brief Add a LGP sub-algorithm to the TPGAlgorithm.
             * 
             * \param[in] graph graph used by the learning agent
             * \param[in] params the LearningParameters used by the Algorithm.
             * \param[in] nbOutputs number of outputs that will be usable for
             * interacting with this LearningEnviromnent.
             * \param[in] iSet the Instruction Set used by the LGPAlgorithm.
             * \param[in] dataSources the DataSources used by the LGPAlgorithm.
             */
            void addLGPAlgorithm(std::shared_ptr<EvoGraph::Graph> graph, const Learn::LearningParameters& params, size_t nbOutputs, const Instructions::Set& iSet, std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources);

            /**
             * \brief Method executing an Agent and outputting action values.
             * 
             * \param[in] agent The agent which is evaluated.
             */
            virtual std::vector<double> executeAgent(std::shared_ptr<const Agent> agent) const override;

            
            /**
             * \brief Clear all the parts of agents that are not used, such as introns for LGPs
             */
            virtual void clearUnusedAgentParts() override {};
    };
}; // namespace TPG_Algorithm

#endif
