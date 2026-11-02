
#ifndef FITNESS_ASSIGNMENT_CLASSIFICATION_F1_H
#define FITNESS_ASSIGNMENT_CLASSIFICATION_F1_H

#include "fitnessAssignment/assigner.h"

#include "metrics/predictionMetric.h"
#include "evaluation/dataset.h"

 namespace FitnessAssignment {

    /**
     * \brief Assigner for classification tasks, using the macro-averaged F1 score
     * computed from the PredictionMetric of each individual as fitness.
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
             * \param[in] individualMetrics the metrics of each individual, indexed by individual ID.
             * \param[in] metricHash the hash of the required metric.
             * \return the fitness of each individual, indexed by individual ID.
             */
            virtual std::map<uint64_t, double> assignFitness(
                const std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>& individualMetrics,
                uint64_t metricHash) const override;
    };
};

#endif // FITNESS_ASSIGNMENT_CLASSIFICATION_F1_H