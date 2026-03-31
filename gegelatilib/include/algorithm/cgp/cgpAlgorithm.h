
#ifndef CGP_ALGORITHM_H
#define CGP_ALGORITHM_H

#include <memory>
#include <vector>

#include "algorithm/lgp/environment.h"

#include "algorithm/lgp/lgpAlgorithm.h"
#include "algorithm/cgp/cgpMutator.h"
namespace Algorithm::CGP {

    /**
     * \brief Class representing a CGPAlgorithm
     * 
     * For now, this class does not support instruction with types different from the environment (ex env with int data and instrunctions with double inputs)
     */
    class   CGPAlgorithm : public LGP::LGPAlgorithm
    {
        public:

            /**
             * \brief Main Algorithm constructor.
             * 
             * \param[in] iSet the Instruction Set used by the LGPAlgorithm.
             * \param[in] parameters the LearningParameters used by the Algorithm.
             * \param[in] algorithmName name of the algorithm used.
             * \param[in] algorithmColor name of the algorithm used.
             */
            CGPAlgorithm(const Instructions::Set& iSet, std::unique_ptr<AlgorithmParameters> parameters = std::make_unique<AlgorithmParameters>(), std::string algorithmName = "CGP", std::string algorithmColor = "#98a02c")
                : LGP::LGPAlgorithm(iSet, std::move(parameters), algorithmName, algorithmColor) {};

            /**
             * \brief Initialize the mutator of the algorithm
             */
            virtual void initMutator() override;

            /**
             * \brief Initialize the algorithm.
             */
            virtual void initAlgorithm(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph) override;
            /**
             * Copy and return a uniqure pointer of the algorithm
             */
            virtual std::unique_ptr<Algorithm> copy() const override;
        };
}; // namespace CGP::Algorithm


namespace Algorithm{
    /**
     * To make the aglrotihm accessible from algorithm namespace
     */
    using CGPAlgorithm = CGP::CGPAlgorithm;
}

#endif
