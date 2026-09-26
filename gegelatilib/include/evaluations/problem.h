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

#ifndef EVALUATIONS_PROBLEM_H
#define EVALUATIONS_PROBLEM_H

#include <map>
#include <queue>
#include <inttypes.h>
#include <queue>

#include "mutator/rng.h"
#include "evaluations/metric.h"
#include "individual.h"

namespace Evaluations {

    /**
     * \brief Class used to control the learning steps of a Graph within
     * a given Problem.
     */
    class Problem
    {
      protected:
      
        /// Input dimensions
        std::vector<Dimensions::Requirement> inputDimensions;

        /// Output dimension
        Dimensions::Requirement outputDimension;

        /// Unique seed of the problem
        uint64_t problemSeed;

      public:
        /**
         * \brief Constructor for Problem.
         * 
         * \param[in] inputDimensions the dimensions of the input sources.
         * \param[in] outputDimension the dimensions of the output source.
         * \param[in] problemSeed unique seed of the problem.
         */
        Problem(const std::vector<Dimensions::Requirement>& inputDimensions, const Dimensions::Requirement& outputDimension, uint64_t problemSeed = 0) : inputDimensions(inputDimensions), outputDimension(outputDimension), problemSeed(problemSeed)  {};

        /// Default destructor for polymorphism
        virtual ~Problem() = default;

        /**
         * \brief get the input dimensions of the EvaluationAgent.
         */
        virtual const std::vector<Dimensions::Requirement>& getInputDimensions() const;

        /**
         * \brief get the output dimension of the EvaluationAgent.
         */
        virtual const Dimensions::Requirement& getOutputDimension() const;

        /**
         * \brief return string of dimension summary
         */
        virtual std::string summary() const;
        
        /**
         * \brief return the unique seed of the problem.
         */
        uint64_t getProblemSeed() const;

        /**
         * \brief return the maximum hash acceptable during evaluation
         * 
         * Default is uint64_t maximum value
         */
        virtual uint64_t maxHash() const;        

        /**
         * \brief TODO
         *
         * \param[in] individual The individual whose genotype is evaluted.
         * \param[in] features list of features to extract from the individual.
         * \param[in] hashes list of hash to use to set the seed of the evaluation
         */
        virtual void extractFeatures(
            const Individual& individual,
            const std::map<size_t, std::unique_ptr<Feature>>& features,
            const std::set<uint64_t>& hashes) const = 0;

    };
}; // namespace Learn

#endif
