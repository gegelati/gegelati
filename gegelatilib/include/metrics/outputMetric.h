

#ifndef OUTPUT_METRIC_H
#define OUTPUT_METRIC_H

#include <map>

#include "metrics/metric.h"



namespace Metrics {

    /**
     * \brief Metric extracting the inputs
     */
    class OutputMetric: public Metric
    {

      protected:
        /// \brief Current map of features extracted.
        std::vector<Data::DataValue> outputs;

      public:
        /**
         * \brief Default constructor
         */
        OutputMetric() {};

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
            const uint64_t& hash, const Individual& individual, 
            const Data::DataValue& output, const Evaluation::Problem& problem) override;


        /**
         * \brief Print the content of the metric.
         */
        virtual std::string toString(std::string prefix = "") const override;
    };


}; // namespace Metrics

#endif // PREDICTION_METRIC_H