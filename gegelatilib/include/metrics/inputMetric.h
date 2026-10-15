
#ifndef INPUT_METRIC_H
#define INPUT_METRIC_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "metrics/metric.h"

#include "data/dataValue.h"

namespace Metrics {

    /**
     * @brief Thread-safe cache that deduplicates identical input vectors.
     *
     * The cache stores weak pointers, so input vectors are released automatically when no metric references them.
     * Hash collisions are handled by checking equality before reusing an existing vector.
     */
    class InputCache
    {
    public:

        /**
         * @brief Returns shared immutable storage for the given inputs.
         * @param inputs Input vector to intern.
         * @return Shared pointer to an existing identical vector or a newly cloned vector.
         */
        static std::shared_ptr<const std::vector<Data::DataValue>> intern(const std::vector<Data::DataValue>& inputs);

    private:
        /// @brief Static mutex
        static std::mutex mutex;

        /// @brief Static cache
        static std::unordered_map<std::size_t, std::vector<std::weak_ptr<const std::vector<Data::DataValue>>>> cache;

        /// @brief Return if two dataValues vector have the same values 
        static bool equalInputs(const std::vector<Data::DataValue>& lhs, const std::vector<Data::DataValue>& rhs);

        /// @brief Clone the input 
        static std::vector<Data::DataValue> cloneInputs(const std::vector<Data::DataValue>& inputs);
    };

    /**
     * @brief Metric that records inputs before each evaluation step.
     *
     * Input vectors are stored through InputCache, allowing identical observations to share memory.
     * The metric retains shared ownership of its recorded vectors for as long as they are needed.
     */
    class InputMetric : public Metric
    {
    protected:
        /// @brief Input vectors recorded in extraction order.
        std::vector<std::shared_ptr<const std::vector<Data::DataValue>>> inputs;

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

        /// @brief Returns the static hash identifying this metric type.
        static uint64_t staticHash();

        /// @brief Returns the recorded input vectors through immutable shared pointers.
        const std::vector<std::shared_ptr<const std::vector<Data::DataValue>>>& getInputs() const;
    };

} // namespace Metrics

#endif // INPUT_METRIC_H
