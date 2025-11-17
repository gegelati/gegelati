
#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <memory>
#include <vector>

#include "algorithm/agent.h"
#include "learn/learningParameters.h"
#include "algorithm/mutator.h"
#include "tpg/tpgGraph.h"

namespace Algorithm {
    /**
     * \brief Abstract class representing an Algorithm.
     *
     * Available algorithms are TPG, MAPLE, and LGP
     */
    class Algorithm
    {
      protected:
        /// Mutator used by the algorithm
        std::shared_ptr<Mutator> mutator;

        /// Current agents used by the algorithm
        std::vector<std::unique_ptr<Agent>> agents;

        /// Graph used by the algorithm
        std::shared_ptr<TPG::TPGGraph> graph;

        /// Parameters used by the algorithm
        const Learn::LearningParameters& params;

      public:

        /**
         * \brief Main Algorithm constructor.
         * 
         * \param[in] params the LearningParameters used by the Algorithm.
         */
        Algorithm(const Learn::LearningParameters& params) : params{params} {};


        /**
         * Initialize the algorithm
         */
        void init(Mutator::RNG& rng);
    };
}; // namespace Algorithm

#endif
