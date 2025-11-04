

#ifndef SELECTION_METRICS_H
#define SELECTION_METRICS_H

#include "tpg/tpgGraph.h"
#include "learn/learningEnvironment.h"

namespace Selector {
    /**
     * \brief Class to extract metrics from either the agent or the environment.
     *
     * This metrics can be used to specify the selection of the selector.
     * This class does not implement any metrics, it need to be override by the different selection methods.
     */
    class SelectionMetrics
    {
      protected:

        /**
         * Score obtained by the agent at the end of an episode
         */
        double score = 0;

        /**
         * Double value for the utility. Utility is used only if the
         * LearningEnvironment override the specific method. The utility
         * represent a score needed for logs but not for learning.
         *
         * It can be used for comparison with different score rules for
         * example.
         */
        double utility = 0;

      public:

        /**
         * \brief Default constructor
         */
        SelectionMetrics() = default;

        /**
         * \brief Constructor with score and utility initialization.
         *
         * \param[in] score the score obtained by the agent.
         * \param[in] utility the utility obtained by the agent.
         */
        SelectionMetrics(double score, double utility = 0)
            : score{score}, utility{utility} {};

        /**
         * Return the score of the agent.
         */
        virtual double getScore() const;

        /**
         * Return the utility of the agent.
         */
        virtual double getUtility() const;

        /**
         * \brief Extract metrics from the agent in the learning environment.
         * 
         * This method is called at every step of the environment evaluation
         * 
         * \param[in] agent the TPGVertex representing the agent.
         * \param[in] learningEnvironment the learning environment in which the agent is evaluated.
         */
        virtual void extractMetricsStep(const TPG::TPGVertex* agent, const Learn::LearningEnvironment& learningEnvironment);

        /**
         * \brief Extract metrics from the agent in the learning environment.
         * 
         * This method is called at the end of every episode of the environment evaluation
         * 
         * \param[in] agent the TPGVertex representing the agent.
         * \param[in] learningEnvironment the learning environment in which the agent is evaluated.
         */
        virtual void extractMetricsEpisode(const TPG::TPGVertex* agent, const Learn::LearningEnvironment& learningEnvironment);

        /**
         * \brief Polymorphic addition assignement operator for
         * SelectionMetrics.
         *
         * \throw std::runtime_error in case the other SelectionMetrics and
         * this have a different typeid.
         */
        virtual SelectionMetrics& operator+=(const SelectionMetrics& other);

        /**
         * \brief Polymorphic multiplication assignement operator for
         * SelectionMetrics.
         */
        virtual SelectionMetrics& operator*=(double factor);

        /**
         * \brief Polymorphic division assignement operator for
         * SelectionMetrics.
         */
        virtual SelectionMetrics& operator/=(double factor);

    };
    
    /**
     * \brief Comparison function to enable sorting of SelectionMetrics with
     * STL.
     */
    bool operator<(std::shared_ptr<Selector::SelectionMetrics> a, std::shared_ptr<Selector::SelectionMetrics> b);

}; // namespace Selector

#endif // SELECTION_METRICS_H