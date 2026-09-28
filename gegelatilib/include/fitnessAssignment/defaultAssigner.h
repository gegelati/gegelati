
#ifndef FITNESS_ASSIGNMENT_DEFAULT_H
#define FITNESS_ASSIGNMENT_DEFAULT_H

#include "fitnessAssignment/assigner.h"
#include "metrics/scoreMetric.h"

 namespace FitnessAssignment {

    /**
     * \brief TODO
     */
    class DefaultAssigner : public Assigner{
        public: 

            /// Default polymorphic destructor
            virtual ~DefaultAssigner() = default;

            /**
             * \brief Default constructor
             */
            DefaultAssigner() {};
            
            // Disable copying to avoid accidental copies (use references or pointers instead).
            DefaultAssigner(const DefaultAssigner&) = delete;
            DefaultAssigner& operator=(const DefaultAssigner&) = delete;
            
            /**
             * \brief method ranking the individuals based on their average metric value over iterations
             * 
             * \param[in] individualMetrics the individuals with their metrics.
             * \param[in] metricHash the hash of the required metric.
             */
            virtual std::vector<std::pair<double, std::shared_ptr<const Individual>>> assignFitness(
                const std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>>& individualMetrics,
                uint64_t metricHash) const override;
    };
};

#endif // FITNESS_ASSIGNMENT_DEFAULT_H