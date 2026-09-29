

#ifndef INPUT_METRIC_H
#define INPUT_METRIC_H

#include <map>

#include "metrics/metric.h"



namespace Metrics {

    /**
     * \brief Metric extracting the inputs
     */
    class InputMetric: public Metric
    {

      protected:
        /// \brief Current vector of features extracted.
        std::vector<std::vector<Data::DataValue>> inputs;

      public:
        /**
         * \brief Default constructor
         */
        InputMetric() {};

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
         * \param[in] inputs inputs of the problem
         * \param[in] problem the problem in which the individual is evaluated.
         */
        virtual void extractBeforeExecution(
            const uint64_t& hash, const Individual& individual, 
            const std::vector<Data::DataValue>& inputs, const Evaluation::Problem& problem) override;

        /**
         * \brief Print the content of the metric.
         */
        virtual std::string toString(std::string prefix = "") const override;

        /**
         * \brief return the static hash of a prediction metric (equal for all prediction metric since there is not hyperparameters)
         */
        static uint64_t staticHash();

        /**
         * \brief return the inputs extracted
         */
        const std::vector<std::vector<Data::DataValue>>& getInputs() const;
    };


}; // namespace Metrics

#endif // PREDICTION_METRIC_H