
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
             *
             * \param[in] name name of the assigner, only used to print it (e.g. in the summary of an evolution).
             */
            explicit ClassificationF1Assigner(std::string name = "ClassificationF1Assigner") : Assigner(std::move(name)) {};
            
            /**
             * \brief Compute the macro-averaged F1 score of each individual from its prediction metrics (see Metrics::PredictionMetric).
             * 
             * \param[in] individualMetrics the metrics of each individual, indexed by individual ID.
             * \param[in] metricHash unused: the prediction metrics are read.
             * \return the fitness of each individual, indexed by individual ID.
             */
            virtual std::map<uint64_t, double> assignFitness(
                const std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>& individualMetrics,
                uint64_t metricHash) const override;
    };
};

#endif // FITNESS_ASSIGNMENT_CLASSIFICATION_F1_H