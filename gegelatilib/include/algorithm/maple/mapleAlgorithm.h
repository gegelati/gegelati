

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
             * \param[in] programAlgorithm the sub-algorithm used to manipulate programs.
             * \param[in] parameters the LearningParameters used by the Algorithm.
             * \param[in] algorithmName name of the algorithm used.
             * \param[in] algorithmColor name of the algorithm used.
             */
            MapleAlgorithm(const Algorithm& programAlgorithm, std::unique_ptr<AlgorithmParameters> parameters = std::make_unique<AlgorithmParameters>(), std::string algorithmName = "MAPLE", std::string algorithmColor = "#ffd900")
                : TPG::TPGAlgorithm(programAlgorithm, std::move(parameters), algorithmName, algorithmColor){
            };

            
            /**
             * \brief Initialize the managerof the algorithm
             * 
             * \param[in] outputs outputs needed for the algorithm.
             */
            virtual void initManager() override;

            /**
             * \brief Initialize the mutator of the algorithm
             */
            virtual void initMutator() override;

            
            /**
             * \brief Initialize the algorithm.
             */
            virtual void initAlgorithm(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph) override;

            /**
             * \brief Initialize the sub-algorithms of the algorithm
             * 
             * \param[in] rng deterministic random generator
             * \param[in] outputs outputs needed for the algorithm.
             * \param[in] dataSource input sources of the algorithm.
             * \param[in] graph the EvoGraph::Graph used by the algorithm.
             */
            virtual void initSubAlgorithms(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph) override;
            
            /**
             * Copy and return a uniqure pointer of the algorithm
             */
            virtual std::unique_ptr<Algorithm> copy() const override;

            
            /**
             * \brief inherrit from algorithm class
             */
            virtual void printCodeGenAgents(std::ofstream& fileMain, std::ofstream& fileMainH, const std::set<std::reference_wrapper<const Agent>>& agents, std::map<uint64_t, std::set<std::reference_wrapper<const Agent>>>& subAgents) const;
            
    };

} // namespace Algorithm::Maple


namespace Algorithm{
    /**
     * To make the algorithm accessible from algorithm namespace
     */
    using MapleAlgorithm = Maple::MapleAlgorithm;
}

#endif // MAPLE_ALGORITHM_H