
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
    class   LGPAlgorithm : public Algorithm
    {
        protected:

            /// Archive used during the training process
            std::shared_ptr<Archive> archive;

            /// Environment for executing LGP 
            std::shared_ptr<Environment> env;

            /// Instruction Set used by the LGPAlgorithm
            const Instructions::Set& iSet;

        public:

            /**
             * \brief Main Algorithm constructor.
             * 
             * \param[in] params the LearningParameters used by the Algorithm.
             * \param[in] nbOutputs number of outputs that will be usable for
             * interacting with this LearningEnviromnent.
             * \param[in] iSet the Instruction Set used by the LGPAlgorithm.
             * \param[in] algorithmName name of the algorithm used.
             */
            LGPAlgorithm(const Learn::LearningParameters& params, size_t nbOutputs, const Instructions::Set& iSet, std::string algorithmName = "LGP")
                : Algorithm(params, nbOutputs, algorithmName), iSet{iSet} {};

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
             * \brief Clear all the parts of agents that are not used, such as introns for LGPs
             */
            virtual void clearUnusedAgentParts() override {};


            
            /**
             * \brief Initialize the algorithm.
             */
            virtual void init(RNG::RNG& rng, Learn::LearningEnvironment& le, std::shared_ptr<EvoGraph::Graph> graph) override;
        };
}; // namespace LGP_Algorithm


namespace Algorithm{
    /**
     * To make the aglrotihm accessible from algorithm namespace
     */
    using LGPAlgorithm = LGP::LGPAlgorithm;
}

#endif
