/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
 * Pierre-Yves Le Rolland-Raumer <plerolla@insa-rennes.fr> (2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2023 - 2025)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

#ifndef EVALUATIONS_AGENT_H
#define EVALUATIONS_AGENT_H

#include <map>
#include <queue>
#include <inttypes.h>
#include <set>

#include "mutator/rng.h"
#include "individual.h"
#include "data/hash.h"

#include "evaluation/problem.h"
#include "evaluation/metric.h"

namespace Evaluation {
    
    /// @brief Modes
    enum class Mode
    {
        TRAINING,
        VALIDATION,
        TESTING
    };

    /**
     * \brief Class used to control the learning steps of a Graph within
     * a given Problem.
     */
    class Evaluator
    {
      protected:

      public:
        /**
         * \brief Constructor for EvaluationAgent.
         * 
         */
        Evaluator() {};

        /// Default destructor for polymorphism
        virtual ~Evaluator() = default;

        /**
         * \brief compute the hashes used the evaluation.
         */
        std::set<uint64_t> computeEvaluationHashes(
            uint64_t nbIterations, uint64_t generationNumber, Mode mode,
            uint64_t problemSeed, uint64_t problemMaxHash) const;

        /**
         * \brief Evaluate all individual of the representations.
         *
         * This method calls the evaluateIndividual method for every individual
         * of the representations. The method returns a sorted map associating each
         * individual to its average score.
         *
         * \param[in] individuals The individuals whose genotypes are evaluted.
         * \param[in] problem problem
         * \param[in] metrics problem
         * \param[in] nbIterations problem
         * \param[in] generationNumber the integer number of the current
         * generation.
         * \param[in] mode the LearningMode to use during the policy
         * evaluation.
         */
        virtual std::map<std::shared_ptr<const Individual>, std::unique_ptr<MetricMap>, SharedLess<Individual>> evaluateIndividuals(
            const std::set<std::shared_ptr<const Individual>, SharedLess<Individual>>& individuals, 
            Problem& problem, const MetricMap& metrics,
            size_t nbIterations, uint64_t generationNumber,
            Mode mode) const;
    };
}; // namespace Learn

#endif
