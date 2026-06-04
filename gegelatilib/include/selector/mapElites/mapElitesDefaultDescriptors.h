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

#ifndef MAP_ELITES_DEFAULT_DESCRIPTORS_H
#define MAP_ELITES_DEFAULT_DESCRIPTORS_H

#include "selector/mapElites/mapElitesDescriptor.h"
#include "tpg/tpgGraph.h"

namespace Selector {

    namespace MapElites {

        namespace DefaultDescriptors {

            /**
             * Class for ActionValues descriptor.
             *
             * This descriptor is designed for continuous environments (an
             * update could be made to be used on discrete environments too but
             * it might not be useful).
             *
             * It computes the average of the absolute of the action values over
             * an episode.
             */
            class ActionValues : public MapElitesDescriptor
            {
              public:
                /**
                 * \brief Specialisation of initDescriptor
                 */
                virtual void initDescriptor(const TPG::TPGGraph& graph,
                                            const Learn::LearningEnvironment&
                                                learningEnvironment) override;

                /**
                 * \brief Specialisation of getName
                 */
                virtual std::string getName() const override;

                /**
                 * \brief Specialisation of extractMetricsStep
                 */
                virtual void extractMetricsStep(
                    std::vector<double>& metrics, const TPG::TPGVertex* agent,
                    std::vector<double> actionValues,
                    const Learn::LearningEnvironment& learningEnvironment)
                    const override;

                /**
                 * \brief Specialisation of extractMetricsEpisode
                 */
                virtual void extractMetricsEpisode(
                    std::vector<double>& metrics, const TPG::TPGVertex* agent,
                    size_t nbStepsExecuted,
                    const Learn::LearningEnvironment& learningEnvironment)
                    const override;
            };

        }; // namespace DefaultDescriptors
    }; // namespace MapElites
}; // namespace Selector

#endif // MAP_ELITES_DEFAULT_DESCRIPTORS_H
