
#ifndef TPG_ALGORITHM_H
#define TPG_ALGORITHM_H

#include <memory>
#include <vector>

#include "algorithm/algorithm.h"
#include "algorithm/tpg/tpgManager.h"
#include "algorithm/tpg/tpgMutator.h"
#include "algorithm/tpg/tpgAgent.h"

namespace Algorithm {

    namespace TPG {
        /**
         * \brief Abstract class representing an Algorithm.
         *
         * Available algorithms are TPG, MAPLE, and LGP
         */
        class TPGAlgorithm : public Algorithm
        {
            protected:

                /// Archive used during the training process
                Archive archive;

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
                TPGAlgorithm(std::shared_ptr<EvoGraph::Graph> graph, const Learn::LearningParameters& params, size_t nbActions)
                    : Algorithm(graph, params, std::make_shared<TPGManager>(), std::make_shared<TPGMutator>(), nbActions) {};

                /**
                 * \brief Method executing an Agent and outputting action values.
                 * 
                 * \param[in] agent The agent which is evaluated.
                 */
                virtual std::vector<double> executeAgent(std::shared_ptr<const Agent> agent) const;
        };
    };
}; // namespace TPG_Algorithm

#endif
