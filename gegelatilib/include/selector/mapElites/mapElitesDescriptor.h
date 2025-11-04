
#ifndef MAP_ELITES_DESCRIPTORS_H
#define MAP_ELITES_DESCRIPTORS_H

#include "tpg/tpgGraph.h"
#include "learn/learningEnvironment.h"


namespace Selector {
    namespace MapElites {

        /// \brief Structure representing a descriptor for Map Elites selection.
        struct MapElitesDescriptor
        {
            
            public:

                /**
                 * \brief Get the min and max range of the descriptor.
                 * 
                 * \param[in] learningEnvironment the learning environment used to get the ranges.
                 * \return a pair containing the min and max range.
                 */
                virtual std::pair<size_t, size_t> getMinAndMaxRange(const Learn::LearningEnvironment& learningEnvironment) const = 0;

                /**
                 * \brief Get the size of the descriptor.
                 * 
                 * \param[in] learningEnvironment the learning environment used to get the size.
                 * \return the size of the descriptor.
                 */
                virtual size_t getSize(const Learn::LearningEnvironment& learningEnvironment) const = 0;

                /**
                 * \brief Extract the metrics at each step of the evaluation.
                 * 
                 * \param[in] agent the TPGVertex being evaluated.
                 * \param[in] actionValues the action values taken by the agent at this step.
                 * \param[in] learningEnvironment the learning environment used to get the metrics.
                 * \return a vector of double values representing the metrics extracted.
                 */
                virtual std::vector<double> extractMetricsStep(const TPG::TPGVertex* agent, std::vector<double> actionValues, const Learn::LearningEnvironment& learningEnvironment) const = 0;

                /**
                 * \brief Extract the metrics at the end of the evaluation episode.
                 * 
                 * \param[in] agent the TPGVertex being evaluated.
                 * \param[in] learningEnvironment the learning environment used to get the metrics.
                 * \return a vector of double values representing the metrics extracted.
                 */
                virtual std::vector<double> extractMetricsEpisode(const TPG::TPGVertex* agent, const Learn::LearningEnvironment& learningEnvironment) const = 0;
        };

    }; // namespace MapElites
}; // namespace Selector

#endif // MAP_ELITES_DESCRIPTORS_H