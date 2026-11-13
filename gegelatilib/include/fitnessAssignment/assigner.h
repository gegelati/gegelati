
#ifndef FITNESS_ASSIGNMENT_H
#define FITNESS_ASSIGNMENT_H

#include "individual.h"
#include "util/named.h"
#include "metrics/metricMap.h"

 namespace FitnessAssignment {

    /**
     * \brief Base class of the methods converting the metrics measured during
     * evaluations into a scalar fitness value, higher being better.
     */
    class Assigner : public Util::Named {
        protected:

            /// @brief Hash used by the assigner to map from the metric to the fitness.
            /// Future updates could extend this single value to a vector if need for multiple metrics are required in later updates.
            uint64_t metricHash;

        public: 

            /// Default polymorphic destructor
            virtual ~Assigner() = default;

            /**
             * \brief Default constructor
             *
             * \param[in] metricHash Hash used by the assigner to map from the metric to the fitness.
             * \param[in] name name of the assigner, only used to print it (see Util::Named).
             */
            Assigner(uint64_t metricHash, std::string name = "Assigner") : Util::Named(std::move(name)), metricHash{metricHash} {};
            
            // Disable copying to avoid accidental copies (use references or pointers instead).
            Assigner(const Assigner&) = delete;
            Assigner& operator=(const Assigner&) = delete;

            /// \brief return the metric hash of the Assigner
            const uint64_t& getMetricHash() const {return this->metricHash; };
            
            /**
             * \brief method setting score to the individuals based on their evaluation metrics
             * 
             * \param[in] individualMetrics the metrics of each individual, indexed by individual ID.
             * \return the fitness of each individual, indexed by individual ID.
             */
            virtual std::map<uint64_t, double> assignFitness(
                const std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>& individualMetrics) const = 0;
    };
};

#endif // FITNESS_ASSIGNMENT_H