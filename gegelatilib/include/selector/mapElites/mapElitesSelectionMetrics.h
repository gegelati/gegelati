/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2025) :
 *
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
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
