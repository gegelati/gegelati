
#ifndef MAP_ELITES_DEFAULT_DESCRIPTORS_H
#define MAP_ELITES_DEFAULT_DESCRIPTORS_H

#include "algorithm/agent.h"
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
             * It computs the average of the absolute of the action values over
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
                    std::vector<double>& metrics, std::shared_ptr<const Algorithm::Agent> agent,
                    std::vector<double> actionValues,
                    const Learn::LearningEnvironment& learningEnvironment)
                    const override;

                /**
                 * \brief Specialisation of extractMetricsEpisode
                 */
                virtual void extractMetricsEpisode(
                    std::vector<double>& metrics, std::shared_ptr<const Algorithm::Agent> agent,
                    size_t nbStepsExecuted,
                    const Learn::LearningEnvironment& learningEnvironment)
                    const override;
            };

        }; // namespace DefaultDescriptors
    }; // namespace MapElites
}; // namespace Selector

#endif // MAP_ELITES_DEFAULT_DESCRIPTORS_H