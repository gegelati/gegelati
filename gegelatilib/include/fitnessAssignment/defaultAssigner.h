
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
             *
             * \param[in] metricHash Hash used by the assigner to map from the metric to the fitness: should be a ScalarMetric.
             * \param[in] name name of the assigner, only used to print it (e.g. in the summary of an evolution).
             */
            explicit DefaultAssigner(uint64_t metricHash, std::string name = "DefaultAssigner") : Assigner(metricHash, std::move(name)) {};
            
            /**
             * \brief method ranking the individuals based on their average metric value over iterations
             * 
             * \param[in] individualMetrics the metrics of each individual, indexed by individual ID.
             * \return the fitness of each individual, indexed by individual ID.
             */
            virtual std::map<uint64_t, double> assignFitness(
                const std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>& individualMetrics) const override;
    };
};

#endif // FITNESS_ASSIGNMENT_DEFAULT_H