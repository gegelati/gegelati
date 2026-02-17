

#ifndef MAP_ELITES_ARCHIVE_H
#define MAP_ELITES_ARCHIVE_H

#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "learn/evaluationResult.h"
#include "evoGraph/graph.h"

namespace Selector {
    namespace MapElites {

        /**
         * \brief Class representing a Map Elites archive.
         *
         * The archive is represented as a multi-dimensional grid, where each
         * cell can store an EvaluationResult and its corresponding agent.
         */
        class MapElitesArchive
        {
          protected:
            /// Number of bins per descriptor
            uint64_t nbBinPerDescriptor;

            /// Number of descriptors for this archive
            uint64_t nbDescriptors;

            /// Minimum value for the descriptors
            double minValue;

            /// Maximum value for the descriptors
            double maxValue;

            /// Limits of the bins for the descriptors
            std::vector<double> archiveLimits;

            /// The archive storing evaluation results and their corresponding
            /// agent
            std::vector<std::pair<std::shared_ptr<Learn::EvaluationResult>,
                                  std::optional<std::reference_wrapper<const Algorithm::Agent>>>>
                archive;

          public:
            /**
             * \brief Constructor of the MapElitesArchive
             *
             * \param[in] nbBinPerDescriptor Number of bins per descriptor
             * \param[in] nbDescriptors Number of descriptors for this archive
             * \param[in] minValue Minimum value for the descriptors
             * \param[in] maxValue Maximum value for the descriptors
             */
            MapElitesArchive(size_t nbBinPerDescriptor, size_t nbDescriptors,
                             double minValue, double maxValue)
                : nbBinPerDescriptor{nbBinPerDescriptor},
                  nbDescriptors{nbDescriptors}, minValue{minValue},
                  maxValue{maxValue}
            {

                if (nbBinPerDescriptor > 0 && nbDescriptors > 0) {
                    archive.resize(std::pow(nbBinPerDescriptor, nbDescriptors));
                }

                for (size_t idx = 1; idx <= nbBinPerDescriptor; ++idx) {
                    archiveLimits.push_back((double)idx *
                                                (maxValue - minValue) /
                                                (double)nbBinPerDescriptor +
                                            minValue);
                }
            }

            /**
             * \brief Get the size of the archive
             */
            uint64_t size() const;

            /**
             * \brief Get the dimensions of the archive
             */
            std::pair<uint64_t, uint64_t> getDimensions() const;

            /**
             * \brief get the archive limits
             */
            std::vector<double> getArchiveLimits() const;

            /**
             * \brief Get all the archive content
             */
            virtual const std::vector<
                std::pair<std::shared_ptr<Learn::EvaluationResult>,
                          std::optional<std::reference_wrapper<const Algorithm::Agent>>>>&
            getAllArchive() const;

            /**
             * \brief Get the archive content at given indices
             *
             * \param[in] indices the indices to get the archive content from
             */
            virtual const std::pair<std::shared_ptr<Learn::EvaluationResult>,
                                    std::optional<std::reference_wrapper<const Algorithm::Agent>>>&
            getArchiveAt(const std::vector<uint64_t>& indices) const;

            /**
             * \brief Get the archive content at given descriptors
             *
             * \param[in] descriptors the descriptors to get the archive content
             * from
             */
            virtual const std::pair<std::shared_ptr<Learn::EvaluationResult>,
                                    std::optional<std::reference_wrapper<const Algorithm::Agent>>>&
            getArchiveFromDescriptors(
                const std::vector<double>& descriptors) const;

            /**
             * \brief Set the archive content at given indices
             *
             * \param[in] agent the Agent to set in the archive
             * \param[in] eval the EvaluationResult to set in the archive
             * \param[in] indices the indices to set the archive content at
             */
            virtual void setArchiveAt(
                const Algorithm::Agent& agent,
                std::shared_ptr<Learn::EvaluationResult> eval,
                const std::vector<uint64_t>& indices);

            /**
             * \brief Set the archive content at given descriptors
             *
             * \param[in] agent the Agent to set in the archive
             * \param[in] eval the EvaluationResult to set in the archive
             * \param[in] descriptors the descriptors to set the archive content
             * at
             */
            virtual void setArchiveFromDescriptors(
                const Algorithm::Agent& agent,
                std::shared_ptr<Learn::EvaluationResult> eval,
                const std::vector<double>& descriptors);

            /**
             * \brief Get the index in one dimension of the archive from a value
             *
             * \param[in] value the value to get the index from
             */
            virtual uint64_t getIndexArchive(double value) const;

            /**
             * \brief Compute the linear index from multi-dimensional indices
             *
             * \param[in] indices the multi-dimensional indices to compute the
             * linear index from
             */
            virtual uint64_t computeLinearIndex(
                const std::vector<uint64_t>& indices) const;

            /**
             * \brief Compute the multi-dimensional indices from linear index
             *
             * \param[in] index the linear index to compute
             */
            virtual std::vector<uint64_t> computeIndices(uint64_t index) const;

            /**
             * \brief Check if the archive contains a agent
             *
             * \param[in] agent the agent to check
             */
            virtual bool containsAgent(const Algorithm::Agent& agent) const;

            /**
             * \brief Remove a agent from the archive if its number of
             * evaluation is below maxNbEvaluation
             *
             * \param[in] agent the agent to remove
             * \param[in] maxNbEvaluation the maximum number of evaluation
             * allowed
             */
            virtual void removeAgentFromArchiveIfNotComplete(
                const Algorithm::Agent& agent, size_t maxNbEvaluation);

            /**
             * \brief Remove a agent from the archive
             *
             * \param[in] agent the agent to remove
             * \param[in] maxNbEvaluation the maximum number of evaluation
             * allowed
             */
            virtual void removeAgentFromArchive(const Algorithm::Agent& agent,
                                               size_t maxNbEvaluation);

            /**
             * \brief Return a set with the current vectors in the archive.
             */
            virtual std::set<std::reference_wrapper<const Algorithm::Agent>> getVerticesInArchive()
                const;
        };

    }; // namespace MapElites
}; // namespace Selector

#endif // MAP_ELITES_SELECTOR_H