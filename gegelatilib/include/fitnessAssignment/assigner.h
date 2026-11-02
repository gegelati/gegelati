
#ifndef FITNESS_ASSIGNMENT_H
#define FITNESS_ASSIGNMENT_H

#include "individual.h"
#include "metrics/metricMap.h"

 namespace FitnessAssignment {

    /**
     * \brief Base class of the methods converting the metrics measured during
     * evaluations into a scalar fitness value, higher being better.
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
             * \param[in] individualMetrics the metrics of each individual, indexed by individual ID.
             * \param[in] metricHash the hash of the required metric.
             * \return the fitness of each individual, indexed by individual ID.
             */
            virtual std::map<uint64_t, double> assignFitness(
                const std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>& individualMetrics,
                uint64_t metricHash = 0) const = 0;
    };
};

#endif // FITNESS_ASSIGNMENT_H