/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
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

#ifndef EVALUATION_RESULT_H
#define EVALUATION_RESULT_H

#include <cmath>
#include <memory>
#include <map>

#include "evaluation/evaluationMetric.h"

namespace Evaluation {
    /**
     * \brief class for storing multiple EvaluationMetric corresponding to an individual
     */
    class EvaluationResult
    {
      protected:
        /// @brief vector of the evaluationMetrics performed on an individual.
        std::map<std::type_index, std::vector<std::unique_ptr<EvaluationMetric>>> evaluationMetrics;

      public:
        /**
         * \brief Virtual destructor for polymorphism.
         */
        virtual ~EvaluationResult() = default;

        // Disable copying to avoid accidental copies (use references or pointers instead).
        EvaluationResult(const EvaluationResult&) = delete;
        EvaluationResult& operator=(const EvaluationResult&) = delete;

        /// @brief Default empty constructor.
        EvaluationResult() {};

        /**
         * \brief Add an evaluation metric to the evaluation result.
         * 
         * \param[in] metric evaluation metric added.
         */
        virtual void addEvaluationMetric(std::unique_ptr<EvaluationMetric> metric);

        /**
         * \brief Method to get the number of EvaluationMetric type registered.
         */
        virtual size_t getSize() const;

        /**
         * \brief Return true if the result contains the typeIndex.
         * 
         * \param[in] index index controlled
         */
        virtual bool hasTypeIndex(const std::type_index& index) const;

        /**
         * \brief Method to get the vector of EvaluationMetric
         * 
         * \param[in] index index controlled
         */
        virtual const std::vector<std::unique_ptr<EvaluationMetric>>& getEvaluationMetricsAt(const std::type_index& index) const;

        /**
         * \brief Return all the evaluation metrics.
         */
        virtual const std::map<std::type_index, std::vector<std::unique_ptr<EvaluationMetric>>>& getEvaluationMetrics() const;

        /**
         * \brief Print all the evaluation metric evaluated.
         */
        virtual std::string toString(std::string prefix = "") const;
    };
        
    /**
     * \brief operator for printing
     */
    inline std::ostream& operator<<(std::ostream& os, const EvaluationResult& res) {
        return os << res.toString();
    }
} // namespace Learn

#endif
