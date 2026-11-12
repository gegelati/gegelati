

#ifndef OUTPUT_METRIC_H
#define OUTPUT_METRIC_H

#include <map>

#include "metrics/vectorMetric.h"



namespace Metrics {

    /**
     * \brief Metric extracting the outputs of an evaluation.
     *
     * The values of the metric are the outputs made of a single double, in the order of the evaluation.
     * Descriptors (see Descriptors::Descriptor) are in charge of summarizing them, e.g. with their mean.
     */
    class OutputMetric: public VectorMetric
    {

      protected:
        /// \brief Current map of features extracted.
        std::vector<Data::DataValue> outputs;

      public:
        /**
         * \brief Default constructor
         */
        OutputMetric() : VectorMetric(0) {};

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

        /**
         * \brief return the static hash of a prediction metric (equal for all prediction metric since there is not hyperparameters)
         */
        static uint64_t staticHash();

        /**
         * \brief return the outputs extracted
         */
        const std::vector<Data::DataValue>& getOutputs() const;
    };


}; // namespace Metrics

#endif // PREDICTION_METRIC_H