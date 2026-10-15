
#ifndef FITNESS_ASSIGNMENT_CLASSIFICATION_F1_H
#define FITNESS_ASSIGNMENT_CLASSIFICATION_F1_H

#include "fitnessAssignment/assigner.h"

#include "metrics/predictionMetric.h"
#include "evaluation/dataset.h"

 namespace FitnessAssignment {

    /**
     * \brief TODO
     */
    class ClassificationF1Assigner : public Assigner{
        
        protected:

        public: 


            /**
             * \brief Default constructor
             */
            ClassificationF1Assigner() {};
            
            /**
             * \brief method ranking the individuals based on their average metric value over iterations in classification tasks
             * 
             * \param[in] individualMetrics the individuals with their metrics.
             * \param[in] metricHash the hash of the required metric.
             */
            virtual std::map<std::shared_ptr<const Individual>, double, SharedLess<Individual>> assignFitness(
                const std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>>& individualMetrics,
                uint64_t metricHash) const override;
    };
};

#endif // FITNESS_ASSIGNMENT_CLASSIFICATION_F1_H