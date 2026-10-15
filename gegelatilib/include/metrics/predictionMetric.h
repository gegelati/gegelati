

#ifndef PREDICTION_METRIC_H
#define PREDICTION_METRIC_H

#include "metrics/scalarMetric.h"
#include "evaluation/predictionProblem.h"



namespace Metrics {

    /**
     * \brief Abstract class to extract any metrics from either the individual or the environment during an evaluation run.
     */
    class PredictionMetric: public ScalarMetric
    {

      protected:

        /// target of the prediction
        std::shared_ptr<const Data::DataValue> target;
        /// target of the prediction
        std::shared_ptr<const Data::DataValue> prediction;

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
            const Evaluation::Problem& problem) override;

        /**
         * \brief Print the content of the metric.
         */
        virtual std::string toString(std::string prefix = "") const override;

        /**
         * \brief return the static hash of a prediction metric (equal for all prediction metric since there is not hyperparameters)
         */
        static uint64_t staticHash();

        /// @brief Get the value predicted.
        virtual const Data::DataValue& getPrediction() const;

        /// @brief Get the value targetted. 
        virtual const Data::DataValue& getTarget() const;
    };


}; // namespace Metrics

#endif // PREDICTION_METRIC_H