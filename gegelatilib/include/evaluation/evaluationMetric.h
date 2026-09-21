

#ifndef EVALUATION_METRICS_H
#define EVALUATION_METRICS_H

#include <typeindex>

#include "data/dataValue.h"
#include "evaluation/learningEnvironment.h"


/// For include
class Individual;

namespace Evaluation {
    /**
     * \brief Abstract class to extract any metrics from either the individual or the environment during an evaluation run.
     */
    class EvaluationMetric
    {
      protected:

        /// @brief Unique seed of this metric.
        size_t seed;

      public:
        /**
         * \brief Default constructor
         * 
         * \param[in] seed Unique seed of this metric.
         */
        EvaluationMetric(size_t seed) : seed{seed} {};

        /**
         * \brief Get the seed of the metric
         */
        size_t getSeed() const {return this->seed; }

        /**
         * \brief Method to dupplicate the current polymorphic metric, keeping the current parameter but forgetting the extractions.
         * 
         * \param[in] seed Unique seed of this metric.
         */
        virtual std::unique_ptr<EvaluationMetric> cloneEmptyUniquePtr(size_t seed) const = 0;
        

        /**
         * \brief Init the metrics for the individual in the learning environment.
         *
         * This method is called at the beginning of the evaluateJob method.
         *
         * \param[in] individual the individual representing the individual.
         * \param[in] learningEnvironment the learning environment in which the
         * individual is evaluated.
         */
        virtual void initExtraction(
            const Individual& individual,
            const Evaluation::LearningEnvironment& learningEnvironment) {
            /* Empty because sub-class does not need to inherrit from it.*/
        };

        /**
         * \brief Extract metrics from the individual in the learning environment.
         *
         * This method is called at every step of the environment evaluation.
         *
         * \param[in] individual the individual performing a step.
         * \param[in] learningEnvironment the learning environment in which the
         * individual is evaluated.
         */
        virtual void extractBeforeExecution(
            const Individual& individual,
            const Evaluation::LearningEnvironment& learningEnvironment) {
            /* Empty because sub-class does not need to inherrit from it.*/
        };

        /**
         * \brief Extract metrics from the individual in the learning environment.
         *
         * This method is called at every step of the environment evaluation.
         *
         * \param[in] individual the individual performing a step.
         * \param[in] action the action taken by the individual.
         * \param[in] learningEnvironment the learning environment in which the
         * individual is evaluated.
         */
        virtual void extractAfterExecution(
            const Individual& individual, const Data::DataView& action,
            const Evaluation::LearningEnvironment& learningEnvironment) {
            /* Empty because sub-class does not need to inherrit from it.*/
        };

        /**
         * \brief Extract metrics from the individual in the learning environment.
         *
         * This method is called at every step of the environment evaluation.
         *
         * \param[in] individual the individual performing a step.
         * \param[in] learningEnvironment the learning environment in which the
         * individual is evaluated.
         */
        virtual void extractAfterStep(
            const Individual& individual,
            const Evaluation::LearningEnvironment& learningEnvironment) {
            /* Empty because sub-class does not need to inherrit from it.*/
        };

        /**
         * \brief Extract metrics from the individual in the learning environment.
         *
         * This method is called at the end of every episode of the environment
         * evaluation.
         *
         * \param[in] individual the individual performing the run.
         * \param[in] nbStepsExecuted number of steps executed at the end of the
         * episode.
         * \param[in] learningEnvironment the learning environment in
         * which the individual is evaluated.
         */
        virtual void extractMetricRun(
            const Individual& individual, size_t nbStepsExecuted,
            const Evaluation::LearningEnvironment& learningEnvironment) {
            /* Empty because sub-class does not need to inherrit from it.*/
        };

        /**
         * \brief Print the content of the metric.
         */
        virtual std::string toString(std::string prefix = "") const = 0;


        /**
         * \brief Return the typeId of the metric
         */
        virtual std::type_index typeId() const noexcept = 0;
    };
        
    /**
     * \brief operator for printing
     */
    inline std::ostream& operator<<(std::ostream& os, const EvaluationMetric& metric) {
        return os << metric.toString();
    }


}; // namespace Evaluation

#endif // EVALUATION_METRICS_H