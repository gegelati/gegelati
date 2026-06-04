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

#ifndef MAP_ELITES_DESCRIPTORS_H
#define MAP_ELITES_DESCRIPTORS_H

#include "learn/learningEnvironment.h"
#include "tpg/tpgGraph.h"

namespace Selector {
    namespace MapElites {

        /**
         * \brief Abstract class representing a descriptor for Map Elites
         * selection.
         *
         * This class need to be inherrit to create specific descriptors.
         * Example of descriptors are given in
         * selector/mapElites/mapElitesDefaultDescriptors
         */
        class MapElitesDescriptor
        {

          protected:
            /// Number of descriptor
            size_t nbDescriptors;

            /// Minimum value in the archive
            double minRange;

            /// Maximum value in the archive
            double maxRange;

            /// Boolean value to check if the initDescriptor method has been
            /// executed.
            bool init = false;

          public:
            /**
             * \brief method that return if the initDescriptor method has been
             * executed
             */
            virtual bool isInit() const
            {
                return this->init;
            };

            /**
             * \brief initiate the descriptor attribute (such as nbDescriptors
             * and min/max ranges)
             *
             * \param[in] graph current graph
             * \param[in] learningEnvironment the learning environment used
             */
            virtual void initDescriptor(
                const TPG::TPGGraph& graph,
                const Learn::LearningEnvironment& learningEnvironment) = 0;

            /**
             * \brief abstract method returning the name of the descriptor.
             */
            virtual std::string getName() const = 0;

            /**
             * \brief Get the min and max range of the descriptor.
             */
            virtual std::pair<double, double> getMinAndMaxRange() const
            {
                return std::make_pair(this->minRange, this->maxRange);
            };

            /**
             * \brief Get the number of the descriptor.
             */
            virtual size_t getNbDescriptors() const
            {
                return this->nbDescriptors;
            };
            /**
             * \brief Extract the metrics at each step of the evaluation.
             *
             * \param[in] metrics vector of current extracted metrics
             * \param[in] agent the TPGVertex being evaluated.
             * \param[in] actionValues the action values taken by the agent at
             * this step.
             * \param[in] learningEnvironment the learning environment used to
             * get the metrics.
             */
            virtual void extractMetricsStep(
                std::vector<double>& metrics, const TPG::TPGVertex* agent,
                std::vector<double> actionValues,
                const Learn::LearningEnvironment& learningEnvironment) const {
                /* Empty because sub-class does not need to inherrit from it.*/
            };

            /**
             * \brief Extract the metrics at the end of the evaluation episode.
             *
             * \param[in] metrics vector of current extracted metrics
             * \param[in] agent the TPGVertex being evaluated.
             * \param[in] nbStepsExecuted number of steps executed at the end of
             * the episode.
             * \param[in] learningEnvironment the learning environment used to
             * get the metrics.
             */
            virtual void extractMetricsEpisode(
                std::vector<double>& metrics, const TPG::TPGVertex* agent,
                size_t nbStepsExecuted,
                const Learn::LearningEnvironment& learningEnvironment) const {
                /* Empty because sub-class does not need to inherrit from it.*/
            };
        };

    }; // namespace MapElites
}; // namespace Selector

#endif // MAP_ELITES_DESCRIPTORS_H
