
#ifndef LGP_ALGORITHM_H
#define LGP_ALGORITHM_H

#include <memory>
#include <vector>

#include "algorithm/algorithm.h"
#include "algorithm/lgp/lgpManager.h"
#include "algorithm/lgp/lgpMutator.h"
#include "algorithm/lgp/lgpAgent.h"

namespace Algorithm {

    namespace LGP {
        /**
         * \brief Abstract class representing a LGPAlgorithm
         */
        class LGPAlgorithm : public Algorithm
        {
            protected:

                /// Archive used during the training process
                Archive archive;

                /// Environment for executing LGP 
                Environment env;

            public:

                /**
                 * \brief Main Algorithm constructor.
                 * 
                 * \param[in] graph graph used by the learning agent
                 * \param[in] params the LearningParameters used by the Algorithm.
                 * \param[in] manager Manager of the algorithm to store and maintain agents
                 * \param[in] nbActions number of actions that will be usable for
                 * interacting with this LearningEnviromnent.
                 */
                LGPAlgorithm(std::shared_ptr<EvoGraph::Graph> graph, const Learn::LearningParameters& params, const Instructions::Set& iSet, size_t nbOutputs, std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources)
                    : archive(params.archiveSize, params.archivingProbability), Algorithm(graph, params, std::make_shared<LGPManager>(env, archive, nbOutputs), std::make_shared<LGPMutator>(), nbOutputs), env(iSet, params, dataSources) {};

                /**
                 * \brief Method executing an Agent and outputting action values.
                 * 
                 * \param[in] agent The agent which is evaluated.
                 */
                virtual std::vector<double> executeAgent(std::shared_ptr<const Agent> agent) const override;

                const Environment& getEnvironment() const;

                const Archive& getArchive() const;
        };
    };
}; // namespace LGP_Algorithm

#endif
