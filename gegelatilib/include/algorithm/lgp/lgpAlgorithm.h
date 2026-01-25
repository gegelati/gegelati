
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
             * \param[in] iSet the Instruction Set used by the LGPAlgorithm.
             * \param[in] algorithmName name of the algorithm used.
             */
            LGPAlgorithm(const Learn::LearningParameters& params, const Instructions::Set& iSet, std::string algorithmName = "LGP")
                : Algorithm(params, algorithmName), iSet{iSet} {};


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
            virtual void initAlgorithm(RNG::RNG& rng, std::shared_ptr<const Output::OutputHandler> outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph) override;
        };
}; // namespace LGP_Algorithm


namespace Algorithm{
    /**
     * To make the aglrotihm accessible from algorithm namespace
     */
    using LGPAlgorithm = LGP::LGPAlgorithm;
}

#endif
