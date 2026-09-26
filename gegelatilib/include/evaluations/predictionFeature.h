

#ifndef EVALUATIONS_PREDICTION_FEATURE_H
#define EVALUATIONS_PREDICTION_FEATURE_H

#include "evaluations/feature.h"
#include "evaluations/classificationProblem.h"



namespace Evaluations {

    /**
     * \brief Abstract class to extract any metrics from either the individual or the environment during an evaluation run.
     */
    class PredictionFeature: public Feature
    {

      public:
        /**
         * \brief Default constructor
         */
        PredictionFeature() {};


        /**
         * \brief Extract metrics from the individual in the problem.
         *
         * This method is called at every step of the evaluation, after execution.
         *
         * \param[in] hash unique hash identifying the evaluation.
         * \param[in] individual the individual performing a step.
         * \param[in] output the output of the individual execution individual.
         * \param[in] problem the problem in which the individual is evaluated.
         */
        virtual void extractAfterExecution(
            const uint64_t& hash,
            const Individual& individual, 
            const Data::DataValue& output,
            const Problem& problem) override;

        /**
         * \brief Print the content of the metric.
         */
        virtual std::string toString(std::string prefix = "") const override;
    };


}; // namespace Evaluation

#endif // EVALUATIONS_PREDICTION_FEATURE_H