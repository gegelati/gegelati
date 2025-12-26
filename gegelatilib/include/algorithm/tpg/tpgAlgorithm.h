
#ifndef TPG_ALGORITHM_H
#define TPG_ALGORITHM_H

#include <memory>
#include <vector>

#include "archive.h"
#include "algorithm/algorithm.h"
#include "algorithm/tpg/tpgManager.h"
#include "algorithm/tpg/tpgMutator.h"
#include "algorithm/tpg/tpgAgent.h"
#include "algorithm/lgp/lgpAlgorithm.h"
#include "algorithm/tpg/tpgJob.h"

#include "learn/learningEnvironment.h"

namespace Algorithm::TPG {

    /**
     * \brief Abstract class representing a TPGAlgorithm
     */
    class TPGAlgorithm : public Algorithm
    {
        protected:

            /// Name of the program algorithm associated with the TPG agents.
            std::string programAlgorithmName;

            /// Archive used during the training process
            std::shared_ptr<Archive> archive;

        public:

            /**
             * \brief Main Algorithm constructor.
             * 
             * \param[in] params the LearningParameters used by the Algorithm.
             * \param[in] iSet the Instruction Set used by the LGPAlgorithm.
             * \param[in] algorithmName name of the algorithm used.
             */
            TPGAlgorithm(const Learn::LearningParameters& params, const Instructions::Set& iSet, std::string algorithmName = "TPG")
                : Algorithm(params, algorithmName), archive{std::make_shared<Archive>(params.archiveSize, params.archivingProbability)}{
                this->addLGPAlgorithm(params, iSet);
            };

            /**
             * \brief Add a LGP sub-algorithm to the TPGAlgorithm.
             * 
             * \param[in] params the LearningParameters used by the Algorithm.
             * \param[in] iSet the Instruction Set used by the LGPAlgorithm.
             */
            void addLGPAlgorithm(const Learn::LearningParameters& params, const Instructions::Set& iSet);

            /**
             * \brief Method executing an Agent and outputting action values.
             * 
             * \param[in] agent The agent which is evaluated.
             */
            virtual std::vector<double> executeAgent(std::shared_ptr<const Agent> agent) const override;

            /**
             * \brief Get the Archive used by the LGPAlgorithm.
             */
            std::shared_ptr<const Archive> getArchive() const;
            
            /**
             * \brief Clear all the parts of agents that are not used, such as introns for LGPs
             */
            virtual void clearUnusedAgentParts() override {};

            /**
             * \brief Initialize the algorithm.
             */
            virtual void initAlgorithm(RNG::RNG& rng, std::shared_ptr<const Output::OutputHandler> outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph) override;

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
             * \brief active the current job by using the archive seed of the job and setting it to the algorithm archive.
             */
            virtual void activeJob(Job& job) override;
    };
}; // namespace TPG_Algorithm

namespace Algorithm{
    /**
     * To make the aglrotihm accessible from algorithm namespace
     */
    using TPGAlgorithm = TPG::TPGAlgorithm;
}

#endif
