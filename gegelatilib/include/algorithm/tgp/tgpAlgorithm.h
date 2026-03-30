
#ifndef TGP_ALGORITHM_H
#define TGP_ALGORITHM_H

#include <memory>
#include <vector>

#include "algorithm/lgp/environment.h"

#include "algorithm/lgp/lgpAlgorithm.h"
#include "algorithm/tgp/tgpMutator.h"
namespace Algorithm::TGP {

    /**
     * \brief Class representing a TreeBased GPAlgorithm
     * 
     * For now, this class does not support instruction with types different from the environment (ex env with int data and instrunctions with double inputs)
     */
    class   TGPAlgorithm : public LGP::LGPAlgorithm
    {
        public:

            /**
             * \brief Main Algorithm constructor.
             * 
             * \param[in] params the LearningParameters used by the Algorithm.
             * \param[in] iSet the Instruction Set used by the LGPAlgorithm.
             * \param[in] algorithmName name of the algorithm used.
             * \param[in] algorithmColor name of the algorithm used.
             */
            TGPAlgorithm(const AlgorithmParameters& params, const Instructions::Set& iSet, std::string algorithmName = "TGP", std::string algorithmColor = "#d37217")
                : LGP::LGPAlgorithm(params, iSet, algorithmName, algorithmColor) {};

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
}; // namespace TGP::Algorithm


namespace Algorithm{
    /**
     * To make the aglrotihm accessible from algorithm namespace
     */
    using TGPAlgorithm = TGP::TGPAlgorithm;
}

#endif
