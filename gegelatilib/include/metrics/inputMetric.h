#ifndef INPUT_METRIC_H
#define INPUT_METRIC_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "metrics/metric.h"

#include "data/dataValue.h"

namespace Metrics {

    /**
     * @brief Metric that records a copy of the inputs given to an individual before each execution step.
     *
     * Every extraction stores its own copy of the inputs: nothing is shared or deduplicated, so the memory
     * used grows with the number of extractions and the size of the inputs.
     */
    class InputMetric : public Metric
    {
    protected:
        /// @brief Copies of the inputs of each execution step, in extraction order.
        std::vector<std::vector<Data::DataValue>> inputs;

    public:
        /// @brief Default constructor.
        InputMetric() = default;

        /// @brief Creates an empty instance of this metric.
        std::unique_ptr<Metric> cloneEmptyPtr() const override;

        /**
         * @brief Records the inputs provided before an execution step.
         * @param hash Unique hash identifying the evaluation.
         * @param individual Individual performing the step.
         * @param inputs Inputs provided to the execution step.
         * @param problem Problem in which the individual is evaluated.
         */
        void extractBeforeExecution(const uint64_t& hash, const Individual& individual, const std::vector<Data::DataValue>& inputs, const Evaluation::Problem& problem) override;

        /// @brief Returns a human-readable representation of the recorded inputs.
        std::string toString(std::string prefix = "") const override;

        /// @brief Returns the recorded inputs of each execution step, in extraction order.
        const std::vector<std::vector<Data::DataValue>>& getInputs() const;

        /// @brief Returns the static hash identifying this metric type.
        static uint64_t staticHash();
    };

} // namespace Metrics

#endif // INPUT_METRIC_H
