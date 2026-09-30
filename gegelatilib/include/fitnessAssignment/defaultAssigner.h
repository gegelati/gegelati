
#ifndef FITNESS_ASSIGNMENT_DEFAULT_H
#define FITNESS_ASSIGNMENT_DEFAULT_H

#include "fitnessAssignment/assigner.h"
#include "metrics/scalarMetric.h"

 namespace FitnessAssignment {

    /**
     * \brief TODO
     */
    class DefaultAssigner : public Assigner{
        public: 

            /**
             * \brief Default constructor
             */
            DefaultAssigner() {};
            
            /**
             * \brief method ranking the individuals based on their average metric value over iterations
             * 
             * \param[in] individualMetrics the individuals with their metrics.
             * \param[in] metricHash the hash of the required metric.
             */
            virtual std::map<std::shared_ptr<const Individual>, double, SharedLess<Individual>> assignFitness(
                const std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>>& individualMetrics,
                uint64_t metricHash) const override;
    };
};

#endif // FITNESS_ASSIGNMENT_DEFAULT_H