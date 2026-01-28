
#ifndef ATPG_ALGORITHM_H
#define ATPG_ALGORITHM_H

#include <memory>
#include <vector>

#include "archive.h"
#include "algorithm/tpg/tpgAlgorithm.h"
#include "algorithm/atpg/atpgManager.h"
#include "algorithm/atpg/atpgMutator.h"
#include "algorithm/atpg/atpgAgent.h"

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
             * \param[in] algorithmName name of the algorithm used.
             */
            ATPGAlgorithm(const Learn::LearningParameters& params, std::string algorithmName = "ATPG")
                : TPGAlgorithm(params, algorithmName) {};


            /**
             * \brief Main Algorithm constructor.
             * 
             * \param[in] params the LearningParameters used by the Algorithm.
             * \param[in] contextProgramAlgorithm the sub-algorithm used to manipulate context programs.
             * \param[in] actionProgramAlgorithm the sub-algorithm used to manipulate action programs.
             * \param[in] algorithmName name of the algorithm used.
             */
            ATPGAlgorithm(const Learn::LearningParameters& params, std::shared_ptr<Algorithm> contextProgramAlgorithm, std::shared_ptr<Algorithm> actionProgramAlgorithm, std::string algorithmName = "TPG")
                : ATPGAlgorithm(params, algorithmName){
                this->setProgramAlgorithm(contextProgramAlgorithm);
                this->setActionProgramAlgorithm(actionProgramAlgorithm);
            };

            /**
             * \brief Add the action program sub-algorithm to the TPGAlgorithm.
             * 
             */
            void setActionProgramAlgorithm(std::shared_ptr<Algorithm> programAlgorithm);

            /**
             * \brief Initialize the algorithm.
             */
            virtual void initAlgorithm(RNG::RNG& rng, std::shared_ptr<const Output::OutputHandler> outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph) override;            
    };
}; // namespace ATPG_Algorithm

namespace Algorithm{
    /**
     * To make the aglrotihm accessible from algorithm namespace
     */
    using ATPGAlgorithm = ATPG::ATPGAlgorithm;
}

#endif
