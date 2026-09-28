

#ifndef SCORE_METRICS_H
#define SCORE_METRICS_H

#include "metrics/metric.h"
#include "evaluation/reinforcementProblem.h"



namespace Metrics {

    /**
     * \brief Abstract class of metric specifically extracting a score stored as a double.
     */
    class ScoreMetric: public Metric
    {
      protected:

        /// @brief Score of the metric
        double score = 0.0;

      public:
        /**
         * \brief Default constructor
         */
        ScoreMetric() {};

        /**
         * \brief Return the score of the metric.s
         */
        virtual double getScore() const;
    };


}; // namespace Metrics

#endif // REWARD_METRICS_H