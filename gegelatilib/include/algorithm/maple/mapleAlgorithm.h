

#ifndef MAPLE_ALGORITHM_H
#define MAPLE_ALGORITHM_H

#include "algorithm/tpg/tpgAlgorithm.h"
#include "algorithm/maple/mapleManager.h"
#include "algorithm/maple/mapleMutator.h"

namespace Algorithm::Maple {

    /**
     * \brief Class representing a MapleAlgorithm, inherit from TPGAlgorithm
     * 
     * Maple agents can only contain action vertex. Each action contains a 
     */
    class MapleAlgorithm : public TPG::TPGAlgorithm
    {
        public:

            /**
             * \brief Main Algorithm constructor.
             * 
             * \param[in] params the LearningParameters used by the Algorithm.
             * \param[in] iSet the Instruction Set used by the LGPAlgorithm.
             * \param[in] algorithmName name of the algorithm used.
             */
            MapleAlgorithm(const Learn::LearningParameters& params, const Instructions::Set& iSet, std::string algorithmName = "MAPLE")
                : TPG::TPGAlgorithm(params, iSet, algorithmName){
            };

            
            /**
             * \brief Initialize the algorithm.
             */
            virtual void initAlgorithm(RNG::RNG& rng, std::shared_ptr<const Output::OutputHandler> outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph) override;

    };

} // namespace Algorithm::Maple


namespace Algorithm{
    /**
     * To make the algorithm accessible from algorithm namespace
     */
    using MapleAlgorithm = Maple::MapleAlgorithm;
}

#endif // MAPLE_ALGORITHM_H