

#ifndef EVALUATIONS_METRIC_H
#define EVALUATIONS_METRIC_H

#include <typeindex>
#include <set>
#include <map>

#include "data/dataValue.h"
#include "data/hash.h"


/// For include
class Individual;

namespace Evaluation {

    class Problem;


    /**
     * \brief Abstract class to extract any metrics from either the individual or the environment during an evaluation run.
     */
    class Metric
    {
      protected:

        /**
         * \brief map of metrics computed. 
         * 
         * Each Metric is saved as a dataValue, with a unique hash key.
         */
        std::map<uint64_t, Data::DataValue> metrics;

      public:
        /**
         * \brief Default constructor
         */
        Metric() {};

        /**
         * \brief Clone the metric object with an empty metrics map.
         */
        virtual std::unique_ptr<Metric> cloneEmptyPtr() const = 0;
        
        /**
         * \brief Return the keys of the current metrics map
         */
        virtual std::set<uint64_t> getKeys() const;

        /**
         * \brief Return if the metric possess the required key.
         */
        virtual bool hasKey(uint64_t key) const;

        /**
         * \brief return the metric at the required key.
         * 
         * \throw if the key is not in the map
         */
        virtual const Data::DataValue& getMetricAt(uint64_t key) const;

        /**
         * \brief return the number of metrics measured
         */
        virtual size_t size() const;

        /**
         * \brief Add the metrics extracted
         * 
         * If both "this" and "other" contains a same key, the metric of "other" override "this"
         */
        virtual void merge(Metric& other);

        /**
         * \brief Extract metrics from the individual in the problem.
         *
         * This method is called at every step of the evaluation, before execution.
         *
         * \param[in] hash unique hash identifying the evaluation.
         * \param[in] individual the individual performing a step.
         * \param[in] inputs inputs of the problem
         * \param[in] problem the problem in which the individual is evaluated.
         */
        virtual void extractBeforeExecution(
            const uint64_t& hash,
            const Individual& individual,
            const std::vector<Data::DataView>& inputs,
            const Problem& problem) {
            /* Empty because sub-class does not need to inherrit from it.*/
        };

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
            const Problem& problem) {
            /* Empty because sub-class does not need to inherrit from it.*/
        };

        /**
         * \brief Extract metrics from the individual in the learning environment.
         *
         * This method is called at every step of the environment evaluation.
         *
         * \param[in] hash unique hash identifying the evaluation.
         * \param[in] individual the individual performing a step.
         * \param[in] problem the problem in which the individual is evaluated.
         */
        virtual void extractionEndAndComputeMetric(
            const uint64_t& hash,
            const Individual& individual,
            const Problem& problem) = 0;

        /**
         * \brief Print the content of the metric.
         */
        virtual std::string toString(std::string prefix = "") const = 0;

        /**
         * \brief Compute the hash of the metric. Default is computed from std::type_index.
         */
        virtual uint64_t hash() const noexcept;
    };

    /// Small struct storing multiple metrics, enabling to quickly do merge/copy operations on multiple metrics.
    struct MetricMap {
        public:
            /// Metrics stored
            std::map<uint64_t, std::unique_ptr<Metric>> metrics;

            /// Constructor with single metric
            MetricMap(std::unique_ptr<Metric> metric) {
                metrics.insert(std::make_pair(metric->hash(), std::move(metric)));
            }

            /// Constructor with predifined metric map
            MetricMap(std::map<uint64_t, std::unique_ptr<Metric>>& metrics) : metrics(std::move(metrics)) {};

            /// Merge with another metricMap (steal values of other)
            void merge(MetricMap& other);

            /// Clone the metric map by cloning the contained metrics
            std::unique_ptr<MetricMap> clone() const;

            /// Extract for all metrics
            void extractBeforeExecution(
                const uint64_t& hash, const Individual& individual,
                const std::vector<Data::DataView>& inputs, const Problem& problem);

            /// Extract for all metrics
            void extractAfterExecution(
                const uint64_t& hash, const Individual& individual, 
                const Data::DataValue& output, const Problem& problem);

            /// Extract for all metrics
            void extractionEndAndComputeMetric(
                const uint64_t& hash, const Individual& individual, const Problem& problem);
    };
        
    /**
     * \brief operator for printing
     */
    inline std::ostream& operator<<(std::ostream& os, const Metric& metric) {
        return os << metric.toString();
    }


}; // namespace Evaluation

#endif // EVALUATIONS_METRIC_H