/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2026) :
 *
 * Mickaël Dardaillon <mdardail@insa-rennes.fr> (2026)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

#ifndef TIMING_SELECTION_METRICS_H
#define TIMING_SELECTION_METRICS_H

#include "selector/selectionMetrics.h"

namespace Selector {
    /// @brief Decorator on SelectionMetrics class to add timing informations.
    class TimingSelectionMetrics : public SelectionMetrics
    {
      protected:
        /// @brief wrapped SelectionMetrics to time.
        std::shared_ptr<SelectionMetrics> wrapped;

        /// @brief Execution time of learning agent.
        double agentTime = 0;

        /// @brief Execution time of learning environment.
        double leTime = 0;

        /// @brief Number of actions performed.
        size_t nbActions = 0;

      public:
        /// @brief Default destructor
        virtual ~TimingSelectionMetrics() = default;

        /// @brief Return the execution time of learning agent.
        /// @return Execution time of learning agent.
        double getAgentTime() const
        {
            return agentTime;
        }

        /// @brief Return the execution time of learning environment.
        /// @return Execution time of learning environment.
        double getLeTime() const
        {
            return leTime;
        }

        /// @brief Return the number of actions performed.
        /// @return Number of actions performed.
        size_t getNbActions() const
        {
            return nbActions;
        }

        /**
         * Return the score of the agent.
         */
        double getScore() const override
        {
            return wrapped->getScore();
        };

        /**
         * Return the utility of the agent.
         */
        double getUtility() const override
        {
            return wrapped->getUtility();
        };

        /// @brief Constructor for the SelectionMetrics with timing.
        /// @param obj SelectionMetrics being timed.
        explicit TimingSelectionMetrics(std::shared_ptr<SelectionMetrics> obj)
            : wrapped(obj){};

        /// @brief Forward to wrapped method
        void extractMetricsStep(
            const TPG::TPGVertex* agent, const std::vector<double> actionValues,
            const Learn::LearningEnvironment& learningEnvironment) override;

        /// @brief Specialization of extractMetricsEpisode to add timings.
        /// \param[in] agent the TPGVertex representing the agent
        /// \param[in] nbStepsExecuted number of steps executed at the end of
        /// the episode
        /// \param[in] learningEnvironment the learning environment in which the
        /// agent is evaluated.
        /// @param[in] agentTimeEpisode execution time of learning agent.
        /// @param[in] leTimeEpisode execution time of learning environment.
        void extractMetricsEpisodeWithTiming(
            const TPG::TPGVertex* agent, size_t nbStepsExecuted,
            const Learn::LearningEnvironment& learningEnvironment,
            double agentTimeEpisode, double leTimeEpisode);

        /// @brief Specialization of weightedSum to add timings and nbActions.
        void weightedSum(std::shared_ptr<SelectionMetrics> other,
                         size_t nbEvaluation,
                         size_t nbEvaluationOther) override;
    };

    /// @brief Comparison function to enable sorting of SelectionMetrics with
    /// STL.
    bool operator<(std::shared_ptr<TimingSelectionMetrics> a,
                   std::shared_ptr<TimingSelectionMetrics> b);

    /// @brief Comparison function to enable sorting of SelectionMetrics with
    /// STL.
    bool operator<(std::shared_ptr<SelectionMetrics> a,
                   std::shared_ptr<TimingSelectionMetrics> b);

    /// @brief Comparison function to enable sorting of SelectionMetrics with
    /// STL.
    bool operator<(std::shared_ptr<TimingSelectionMetrics> a,
                   std::shared_ptr<SelectionMetrics> b);
}; // namespace Selector

#endif // TIMING_SELECTION_METRICS_H
