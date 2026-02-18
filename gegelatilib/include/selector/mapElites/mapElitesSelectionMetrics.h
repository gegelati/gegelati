

#ifndef MAP_ELITES_SELECTION_METRICS_H
#define MAP_ELITES_SELECTION_METRICS_H

#include <numeric>

#include "selector/mapElites/mapElitesDescriptor.h"
#include "selector/selectionMetrics.h"

namespace Selector {

    namespace MapElites {

        /**
         * \brief Class to extract metrics from either the agent or the
         * environment.
         *
         * This metrics can be used to specify the selection of the selector.
         * This class does not implement any metrics, it need to be override by
         * the different selection methods.
         */
        class MapElitesSelectionMetrics : public SelectionMetrics
        {
          protected:
            /**
             * \brief Map storing a descriptor and a Vector storing a double
             * value per descriptor (i.e. per Action) of a classification
             * LearningEnvironment.
             */
            std::map<std::shared_ptr<const MapElitesDescriptor>,
                     std::vector<double>>
                mapDescriptors;

          public:
            /**
             * \brief Default constructor of MapEliteselectionMetrics with a
             * vector of descriptors.
             *
             * \param[in] descriptors the vector of descriptors to use.
             */
            MapElitesSelectionMetrics(
                std::vector<std::shared_ptr<const MapElitesDescriptor>>
                    descriptors)
            {
                for (auto descriptor : descriptors) {
                    mapDescriptors.insert({descriptor, {}});
                }
            }

            /**
             * \brief Constructor of MapEliteselectionMetrics with a vector of
             * scores.
             *
             * \param[in] score the score obtained by the agent.
             * \param[in] mapDescriptors the map of descriptors associated to
             * their descriptor values.
             */
            MapElitesSelectionMetrics(
                double score,
                const std::map<std::shared_ptr<const MapElitesDescriptor>,
                               std::vector<double>>& mapDescriptors)
                : SelectionMetrics(score), mapDescriptors(mapDescriptors){};

            /**
             * \brief Constructor of MapEliteselectionMetrics with a vector of
             * scores.
             *
             * \param[in] score the score obtained by the agent.
             */
            MapElitesSelectionMetrics(double score) : SelectionMetrics(score){};

            /**
             * \brief Get a const ref to the scorePerClass attribute.
             */
            const std::map<std::shared_ptr<const MapElitesDescriptor>,
                           std::vector<double>>&
            getMapDescriptors() const;

            /**
             * \brief Specialization of the initialisation of the metrics.
             */
            void initMetrics(
                const TPG::TPGVertex* agent,
                const Learn::LearningEnvironment& learningEnvironment) override;

            /**
             * \brief Specialization of the extraction of the metrics at the end
             * of an episode.
             */
            void extractMetricsStep(
                const TPG::TPGVertex* agent, std::vector<double> actionValues,
                const Learn::LearningEnvironment& learningEnvironment) override;

            /**
             * \brief Specialization of the extraction of the metrics at the end
             * of an episode.
             */
            void extractMetricsEpisode(
                const TPG::TPGVertex* agent, size_t nbStepsExecuted,
                const Learn::LearningEnvironment& learningEnvironment) override;

            /**
             * \brief Specialization of weightedSum method to add the score per
             * class and nbEvalPerClass
             */
            virtual void weightedSum(std::shared_ptr<SelectionMetrics> other,
                                     size_t nbEvaluation,
                                     size_t nbEvaluationOther) override;
        };
    } // namespace MapElites

}; // namespace Selector

#endif // MAP_ELITES_SELECTION_METRICS_H