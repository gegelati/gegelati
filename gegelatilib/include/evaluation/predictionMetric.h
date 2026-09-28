

#ifndef EVALUATIONS_PREDICTION_METRIC_H
#define EVALUATIONS_PREDICTION_METRIC_H

#include "evaluation/metric.h"
#include "evaluation/classificationProblem.h"



namespace Evaluation {

    /**
     * \brief Abstract class to extract any metrics from either the individual or the environment during an evaluation run.
     */
    class PredictionMetric: public Metric
    {

      protected:
        /// \brief Current prediction
        bool currentPrediction;

      public:
        /**
         * \brief Default constructor
         */
        PredictionMetric() {};

        /**
         * \brief clone returning predictionMetric
         */
        virtual std::unique_ptr<Metric> cloneEmptyPtr() const override;


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
         * \brief Stores the prediction
         *
         * \param[in] hash unique hash identifying the evaluation.
         * \param[in] individual the individual performing a step.
         * \param[in] problem the problem in which the individual is evaluated.
         */
        virtual void extractionEndAndComputeMetric(
            const uint64_t& hash,
            const Individual& individual,
            const Problem& problem) override;

        /**
         * \brief Print the content of the metric.
         */
        virtual std::string toString(std::string prefix = "") const override;
    };


}; // namespace Evaluation

#endif // EVALUATIONS_PREDICTION_METRIC_H