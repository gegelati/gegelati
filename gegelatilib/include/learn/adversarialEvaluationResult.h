/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
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

#ifndef ADVERSARIALEVALUATIONRESULT_H
#define ADVERSARIALEVALUATIONRESULT_H

#include <vector>

#include "learn/evaluationResult.h"

namespace Learn {
    /**
     * \brief Class for storing all results of a policy evaluation in
     * in adversarial mode with an AdversarialLearningEnvironment.
     *
     * The main difference with the base EvaluationResult class is that there
     * are several results. Indeed, in adversarial mode there are several roots
     * in a single simulation and, as a consequence, there are several results
     * at the end.
     */
    class AdversarialEvaluationResult : public EvaluationResult {
    protected:
        /// The scores of the roots, in the order in which they participated.
        std::vector<double> scores;

    public:
        /**
         * \brief Base constructor of EvaluationResult, allowing to set scores
         * and the number of evaluations.
         *
         * @param res The scores of the roots in the order.
         * @param nbEval The number of evaluations that have been done to get
         * these scores. Default is 1 as we can guess user only did 1 iteration.
         */
        AdversarialEvaluationResult(std::initializer_list<double> res,
                size_t nbEval=1):
                EvaluationResult(*res.begin(),nbEval),scores(res)
        {}

        /**
         * \brief Simple getter of the score of a single root, given its index.
         *
         * @param index The index of the root in the results list.
         * @return The score corresponding to this index.
         */
        double getScoreOf(int index)
        {
            return scores[index];
        }

        /**
         * \brief Polymorphic addition assignement operator for
         * AdversariEalvaluationResult.
         *
         * \throw std::runtime_error in case the other
         * AdversarialEvaluationResult and this have a different typeid or size.
         */
        virtual EvaluationResult& operator+=(const EvaluationResult& other){
            // Type Check (Must be done in all override)
            // This test will succeed in child class.
            const std::type_info& thisType = typeid(*this);
            if (typeid(other) != thisType) {
                throw std::runtime_error("Type mismatch between EvaluationResults.");
            }

            auto otherConverted = (AdversarialEvaluationResult&) other;

            // Size Check
            if (otherConverted.scores.size() != this->scores.size()) {
                throw std::runtime_error("Size mismatch between AdversarialEvaluationResults.");
            }

            // If the added type is Learn::EvaluationResult
            // Weighted addition of results
            for(int i=0; i<scores.size(); i++){
                this->scores[i] = this->scores[i] * (double)this->nbEvaluation +
                               otherConverted.scores[i] * (double)otherConverted.nbEvaluation;
            }

            this->result /= (double)this->nbEvaluation + (double)otherConverted.nbEvaluation;
            // Addition ot nbEvaluation
            this->nbEvaluation += otherConverted.nbEvaluation;

            return *this;
        }

        /// Inherited from EvaluationResult and unused in the case of a vector.
        double getResult() const override
        {
            return scores[0];
        }

    };
}

#endif
