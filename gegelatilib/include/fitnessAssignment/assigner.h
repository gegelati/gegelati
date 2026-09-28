
#ifndef FITNESS_ASSIGNMENT_H
#define FITNESS_ASSIGNMENT_H

#include "individual.h"
#include "metrics/metricMap.h"

 namespace FitnessAssignment {

    /**
     * \brief TODO
     */
    class Assigner {
        public: 

            /// Default polymorphic destructor
            virtual ~Assigner() = default;

            /**
             * \brief Default constructor
             */
            Assigner() {};
            
            // Disable copying to avoid accidental copies (use references or pointers instead).
            Assigner(const Assigner&) = delete;
            Assigner& operator=(const Assigner&) = delete;
            
            /**
             * \brief method setting score to the individuals based on their evaluation metrics
             * 
             * \param[in] individualMetrics the individuals with their metrics.
             * \param[in] metricHash the hash of the required metric.
             */
            virtual std::vector<std::pair<double, std::shared_ptr<const Individual>>> assignFitness(
                const std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>>& individualMetrics,
                uint64_t metricHash = 0) const = 0;
    };
};

#endif // FITNESS_ASSIGNMENT_H