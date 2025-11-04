

#ifndef CLASSIFICATION_SELECTION_METRICS_H
#define CLASSIFICATION_SELECTION_METRICS_H

#include <numeric>

#include "selector/selectionMetrics.h"

namespace Selector {
    /**
     * \brief Class to extract metrics from either the agent or the environment.
     *
     * This metrics can be used to specify the selection of the selector.
     * This class does not implement any metrics, it need to be override by the different selection methods.
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
         * \brief Vector storing a size_t value per class representing the number of evaluation per class.
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
         * \param[in] scorePerClass the vector of score obtained by the agent per class.
         * \param[in] nbEvalPerClass the vector of number of evaluation per class.
         */
        ClassificationSelectionMetrics(const std::vector<double>& scorePerClass, const std::vector<size_t>& nbEvalPerClass)
            : SelectionMetrics(std::accumulate(scorePerClass.cbegin(), scorePerClass.cend(), 0.0) /
                      scorePerClass.size()), scorePerClass{scorePerClass}, nbEvalPerClass{nbEvalPerClass} {};

        /**
         * Return the score per class of the agent.
         */
        virtual const std::vector<double>& getScorePerClass() const;

        /**
         * Return the number of evaluation per class of the agent.
         */
        virtual const std::vector<size_t>& getNbEvalPerClassPerClass() const;

        /**
         * \brief Extract metrics from the agent in the learning environment.
         * 
         * This method is called at the end of every episode of the environment evaluation
         * 
         * \param[in] agent the TPGVertex representing the agent.
         * \param[in] learningEnvironment the learning environment in which the agent is evaluated.
         */
        void extractMetricsEpisode(const TPG::TPGVertex* agent, const Learn::LearningEnvironment& learningEnvironment) override;

        /**
         * \brief Polymorphic addition assignement operator for
         * SelectionMetrics.
         *
         * \throw std::runtime_error in case the other SelectionMetrics and
         * this have a different typeid.
         */
        virtual SelectionMetrics& operator+=(const SelectionMetrics& other) override;

        /**
         * \brief Polymorphic multiplication assignement operator for
         * SelectionMetrics.
         */
        virtual SelectionMetrics& operator*=(double factor) override;

    };

}; // namespace Selector

#endif // CLASSIFICATION_SELECTION_METRICS_H