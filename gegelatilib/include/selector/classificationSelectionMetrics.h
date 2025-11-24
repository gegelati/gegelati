

#ifndef CLASSIFICATION_SELECTION_METRICS_H
#define CLASSIFICATION_SELECTION_METRICS_H

#include <numeric>

#include "selector/selectionMetrics.h"

namespace Selector {
    /**
     * \brief Class to extract metrics from either the agent or the environment.
     *
     * This metrics can be used to specify the selection of the selector.
     * This class does not implement any metrics, it need to be override by the
     * different selection methods.
     */
    class ClassificationSelectionMetrics : public SelectionMetrics
    {
      protected:
        /**
         * \brief Vector storing a double score per class (i.e. per Action) of
         * a classification LearningEnvironment.
         */
        std::vector<double> scorePerClass;

        /**
         * \brief Vector storing a size_t value per class representing the
         * number of evaluation per class.
         */
        std::vector<size_t> nbEvalPerClass;

      public:
        /**
         * \brief Default constructor
         */
        ClassificationSelectionMetrics() = default;

        /**
         * \brief Constructor with score and utility initialization.
         *
         * \param[in] scorePerClass the vector of score obtained by the agent
         * per class.
         * \param[in] nbEvalPerClass the vector of number of
         * evaluation per class.
         */
        ClassificationSelectionMetrics(
            const std::vector<double>& scorePerClass,
            const std::vector<size_t>& nbEvalPerClass)
            : SelectionMetrics(std::accumulate(scorePerClass.cbegin(),
                                               scorePerClass.cend(), 0.0) /
                               scorePerClass.size()),
              scorePerClass{scorePerClass}, nbEvalPerClass{nbEvalPerClass}
        {
            if (scorePerClass.size() != nbEvalPerClass.size()) {
                throw std::runtime_error("Number of class missmatch.");
            }
        };

        /**
         * Return the score per class of the agent.
         */
        virtual const std::vector<double>& getScorePerClass() const;

        /**
         * Return the number of evaluation per class of the agent.
         */
        virtual const std::vector<size_t>& getNbEvalPerClassPerClass() const;

        /**
         * \brief Specialization of the initialisation of the metrics.
         */
        void initMetrics(
            std::shared_ptr<const Algorithm::Agent> agent,
            const Learn::LearningEnvironment& learningEnvironment) override;

        /**
         * \brief Specialization of the extraction of the metrics at the end of
         * an episode.
         */
        void extractMetricsEpisode(
            std::shared_ptr<const Algorithm::Agent> agent, size_t nbStepsExecuted,
            const Learn::LearningEnvironment& learningEnvironment) override;

        /**
         * \brief Specialization of weightedSum method to add the score per
         * class and nbEvalPerClass
         */
        virtual void weightedSum(std::shared_ptr<SelectionMetrics> other,
                                 size_t nbEvaluation,
                                 size_t nbEvaluationOther) override;
    };

}; // namespace Selector

#endif // CLASSIFICATION_SELECTION_METRICS_H