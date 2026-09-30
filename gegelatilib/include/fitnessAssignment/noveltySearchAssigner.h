
#ifndef NOVELTY_SEARCH_ASSIGNER_H
#define NOVELTY_SEARCH_ASSIGNER_H

#include "fitnessAssignment/assigner.h"
#include "metrics/vectorMetric.h"
#include "metrics/scalarMetric.h"

 namespace FitnessAssignment {

    /**
     * \brief TODO
     */
    class NoveltySearchAssigner : public Assigner{
        protected:
        
            /// @brief The number of neighbors.
            size_t nbNeighbors; 

            /// @brief Indicate if the metrics over features are computed with average or a list.
            bool averageFeature;
        public:

            /**
             * \brief Default constructor
             * 
             * \param[in] nbNeighbors the number of neighbors to measure.
             * \param[in] averageFeature Indicate if the metrics over features are computed with average or a list.
             */
            NoveltySearchAssigner(size_t nbNeighbors, bool averageFeature = true): nbNeighbors{nbNeighbors}, averageFeature{averageFeature} {};

            /**
             * \brief Method to compute the descriptor when faced to multiple features:
             * This method compute the average of the vector metric over each feature.
             * 
             * \param[in] vectorMetrics the map of vector metrics averaged
             */
            virtual std::vector<double> computeOneDescriptorAvgVector(const std::map<uint64_t, const Metrics::VectorMetric*>& vectorMetrics) const;

            /**
             * \brief Method to compute the descriptor when faced to multiple features:
             * This method compute the list of descriptors as the list of each scalar over feature
             * 
             * \param[in] scalarMetrics the map of scalar metrics listed
             */
            virtual std::vector<double> computeOneDescriptorListScalar(const std::map<uint64_t, const Metrics::ScalarMetric*>& scalarMetrics) const;

            /**
             * \brief Compute the descriptor values of the individuals based on the vector metric required
             * 
             * \param[in] individualMetrics the individuals with their metrics.
             * \param[in] metricHash the hash of the required metric.
             * 
             * \return A map of each individual ID with the descriptor values. ID is used to reduce memory coset and increase lisibility
             */
            virtual std::map<uint64_t, std::vector<double>> computeDescriptors(
                const std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>>& individualMetrics,
                uint64_t metricHash) const;
            
            /**
             * \brief Compute the distances between every individuals based on their descriptors values
             * 
             * \param[in] descriptors the map of descriptor values;
             * 
             * \return a map of map linking every individual to all his neighbors
             */
            virtual std::map<uint64_t, std::map<uint64_t, double>> computeDistances(std::map<uint64_t, std::vector<double>> descriptors) const;

            /**
             * \brief compute the N nearest neighbors of every individual based on the distances matrix.
             * 
             * \param[in] distances the distances matrix
             */
            virtual std::map<uint64_t, std::vector<uint64_t>> computeNeighbors(std::map<uint64_t, std::map<uint64_t, double>> distances) const;

            /**
             * \brief Compute novelty scores based on the distances
             * 
             * \param[in] distances the distances matrix
             * \param[in] neighbors the neighbors of each individual
             */
            virtual std::map<uint64_t, double> computeNoveltyScores(
                std::map<uint64_t, std::map<uint64_t, double>> distances,
                std::map<uint64_t, std::vector<uint64_t>> neighbors) const;

            /**
             * \brief method ranking the individuals based on their average metric value over iterations
             * 
             * \param[in] individualMetrics the individuals with their metrics.
             * \param[in] metricHash the hash of the required metric.
             */
            virtual std::map<std::shared_ptr<const Individual>, double, SharedLess<Individual>> assignFitness(
                const std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>>& individualMetrics,
                uint64_t metricHash) const override;
    };
};

#endif // FITNESS_ASSIGNMENT_DEFAULT_H