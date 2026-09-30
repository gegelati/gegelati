
#ifndef NOVELTY_SEARCH_ASSIGNER_H
#define NOVELTY_SEARCH_ASSIGNER_H

#include "fitnessAssignment/assigner.h"
#include "metrics/vectorMetric.h"

 namespace FitnessAssignment {

    /**
     * \brief TODO
     */
    class NoveltySearchAssigner : public Assigner{
        protected:
        
            /// @brief The number of neighbors.
            size_t nbNeighbors; 
        public:

            /**
             * \brief Default constructor
             */
            NoveltySearchAssigner(size_t nbNeighbors): nbNeighbors{nbNeighbors} {};

            /**
             * \brief Compute the descriptor values of the individuals based on the vector metric required
             * 
             * \param[in] individualMetrics the individuals with their metrics.
             * \param[in] metricHash the hash of the required metric.
             * 
             * \return A map of each individual ID with the descriptor values. ID is used to reduce memory coset and increase lisibility
             */
            std::map<uint64_t, std::vector<double>> computeDescriptors(
                const std::map<std::shared_ptr<const Individual>, std::unique_ptr<Metrics::MetricMap>, SharedLess<Individual>>& individualMetrics,
                uint64_t metricHash) const;
            
            /**
             * \brief Compute the distances between every individuals based on their descriptors values
             * 
             * \param[in] descriptors the map of descriptor values;
             * 
             * \return a map of map linking every individual to all his neighbors
             */
            std::map<uint64_t, std::map<uint64_t, double>> computeDistances(std::map<uint64_t, std::vector<double>> descriptors) const;

            /**
             * \brief compute the N nearest neighbors of every individual based on the distances matrix.
             * 
             * \param[in] distances the distances matrix
             */
            std::map<uint64_t, std::vector<uint64_t>> computeNeighbors(std::map<uint64_t, std::map<uint64_t, double>> distances) const;

            /**
             * \brief Compute novelty scores based on the distances
             * 
             * \param[in] distances the distances matrix
             * \param[in] neighbors the neighbors of each individual
             */
            std::map<uint64_t, double> computeNoveltyScores(
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