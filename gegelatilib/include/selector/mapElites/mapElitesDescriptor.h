
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