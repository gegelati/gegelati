
#ifndef FITNESS_ASSIGNMENT_DEFAULT_H
#define FITNESS_ASSIGNMENT_DEFAULT_H

#include "fitnessAssignment/assigner.h"
#include "metrics/scalarMetric.h"

 namespace FitnessAssignment {

    /**
     * \brief Assigner using the average of a scalar metric (e.g. the sum of rewards
     * for reinforcement problems) over all the evaluations as fitness.
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
             * \param[in] individualMetrics the metrics of each individual, indexed by individual ID.
             * \param[in] metricHash the hash of the required metric.
             * \return the fitness of each individual, indexed by individual ID.
             */
            virtual std::map<uint64_t, double> assignFitness(
                const std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>& individualMetrics,
                uint64_t metricHash) const override;
    };
};

#endif // FITNESS_ASSIGNMENT_DEFAULT_H