
#ifndef LGP_ALGORITHM_H
#define LGP_ALGORITHM_H

#include <memory>
#include <vector>

#include "algorithm/algorithm.h"
#include "algorithm/lgp/lgpManager.h"
#include "algorithm/lgp/lgpMutator.h"
#include "algorithm/lgp/lgpAgent.h"

namespace Algorithm::LGP {

    /**
     * \brief Abstract class representing a LGPAlgorithm
     */
    class LGPAlgorithm : public Algorithm
    {
        protected:

            /// Archive used during the training process
            std::shared_ptr<Archive> archive;

            /// Environment for executing LGP 
            std::shared_ptr<Environment> env;

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
            LGPAlgorithm(std::shared_ptr<EvoGraph::Graph> graph, const Learn::LearningParameters& params, size_t nbOutputs, const Instructions::Set& iSet, std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources, std::string algorithmName = "LGP")
                : Algorithm(graph, params, std::make_shared<LGPManager>(), std::make_shared<LGPMutator>(), nbOutputs, algorithmName), 
                    archive(std::make_shared<Archive>(params.archiveSize, params.archivingProbability)), 
                    env(std::make_shared<Environment>(iSet, params, dataSources)) {

                std::dynamic_pointer_cast<LGPManager>(this->manager)->init(archive, env, nbOutputs);
            };

            /**
             * \brief Method executing an Agent and outputting action values.
             * 
             * \param[in] agent The agent which is evaluated.
             */
            virtual std::vector<double> executeAgent(std::shared_ptr<const Agent> agent) const override;

            /**
             * \brief Get the Environment used by the LGPAlgorithm.
             */
            std::shared_ptr<const Environment> getEnvironment() const;

            /**
             * \brief Get the Archive used by the LGPAlgorithm.
             */
            std::shared_ptr<const Archive> getArchive() const;

            
            /**
             * \brief Clear all the parts of agents that are not used, such as introns for LGPs
             */
            virtual void clearUnusedAgentParts() override {};

            /**
             * \brief Create a new ExecutionEngine for this Algorithm.
             * 
             * \return a shared pointer to the created ExecutionEngine.
             */
            virtual std::shared_ptr<ExecutionEngine> createExecutionEngine() override;
        };
}; // namespace LGP_Algorithm

#endif
