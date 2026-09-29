#ifndef METRICS_MAP_H
#define METRICS_MAP_H

#include "metrics/metric.h"

namespace Metrics {

    /// @brief Struct storing a template of required metrics
    struct MetricMapTemplate {
        public:
            /// @brief Map of the empty metrics
            std::map<uint64_t, std::unique_ptr<Metric>> emptyMetrics;

            /// @brief Default constructor
            MetricMapTemplate() {};

            /// Constructor with single metric type
            MetricMapTemplate(std::unique_ptr<Metric> emptyMetric) {
                this->addRequiredMetric(std::move(emptyMetric));
            }
            
            /// Constructor with predifined metric map
            MetricMapTemplate(std::vector<std::unique_ptr<Metric>>& emptyMetrics) {
                for(auto& metric: emptyMetrics) {
                    this->addRequiredMetric(std::move(metric));
                }
            };
            
            /// @brief add a metric
            virtual void addRequiredMetric(std::unique_ptr<Metric> emptyMetric);

            /**
             * \brief Return the keys of the current metrics map
             */
            virtual std::set<uint64_t> getMetricHash() const;

            /**
             * \brief Return if the metric possess the required key.
             */
            virtual bool hasMetricHash(uint64_t key) const;

            /// Number of metrics.
            virtual size_t size() const;
    };

    /// Small struct storing multiple metrics, enabling to quickly do merge/copy operations on multiple metrics.
    class MetricMap {
        private:
            /// Metrics stored
            std::map<uint64_t, std::map<uint64_t, std::unique_ptr<Metric>>> metrics;

            /// @brief Current metrics measured.
            std::map<uint64_t, std::unique_ptr<Metric>> currentMetrics;

            /// metric map template
            std::shared_ptr<const MetricMapTemplate> metricTemplate;
        public:

            /// @brief The map is created with a metric map template.
            MetricMap(std::shared_ptr<const MetricMapTemplate> metricTemplate) : metricTemplate{metricTemplate} {
                this->resetCurrentMetrics();
            };

            /// Return the metric template
            const MetricMapTemplate& getTemplate() const;

            /**
             * \brief Reset the current metrics for a new evaluation.
             */
            void resetCurrentMetrics();

            /**
             * @brief Return the metric value measured of a specific hash of metric. 
             * 
             * \tparam T the type of the metric searched.
             */  
            template <typename T>
            std::map<uint64_t, const T*> getMetricValues(uint64_t metricHash) {
                std::map<uint64_t, const T*> metricMap;
                for(auto& [featureHash, featureMap]: this->metrics) {
                    if(featureMap.find(metricHash) != featureMap.end()) {
                        const T* metricCasted = dynamic_cast<const T*>(featureMap.at(metricHash).get());
                        if(metricCasted == nullptr) {
                            throw std::runtime_error("Metricmap:getMetricValues: metric failed to cast to required type.");
                        }
                        metricMap.insert(std::make_pair(featureHash, metricCasted));
                    }
                }
                return metricMap;
            };

            /// Extract for all metrics
            void extractBeforeExecution(
                const uint64_t& hash, const Individual& individual,
                const std::vector<Data::DataValue>& inputs, const Evaluation::Problem& problem);

            /// Extract for all metrics
            void extractAfterExecution(
                const uint64_t& hash, const Individual& individual, 
                const Data::DataValue& output, const Evaluation::Problem& problem);

            /// Extract for all metrics
            void extractionEnd(
                const uint64_t& hash, const Individual& individual, const Evaluation::Problem& problem);

            /// Merge with another metricMap (steal values of other)
            void merge(MetricMap& other);

            /**
             * \brief Print the content of the metric map.
             */
            virtual std::string toString(std::string prefix = "") const;
    };
    /**
     * \brief operator for printing
     */
    inline std::ostream& operator<<(std::ostream& os, const MetricMap& metricMap) {
        return os << metricMap.toString();
    }

};

#endif // METRICS_MAP_H