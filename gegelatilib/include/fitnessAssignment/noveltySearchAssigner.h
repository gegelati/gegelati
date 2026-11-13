
#ifndef NOVELTY_SEARCH_ASSIGNER_H
#define NOVELTY_SEARCH_ASSIGNER_H

#include <memory>
#include <stdexcept>

#include "fitnessAssignment/assigner.h"
#include "fitnessAssignment/noveltyDescriptors/descriptor.h"

 namespace FitnessAssignment {

    /**
     * \brief Assigner for novelty search: the fitness of an individual is the average
     * distance between its descriptor and the descriptors of its nearest neighbors.
     */
    class NoveltySearchAssigner : public Assigner{
        protected:
        
            /// @brief The number of neighbors.
            size_t nbNeighbors; 

            /// @brief The function mapping the metrics of an individual to its descriptor values.
            std::shared_ptr<const Descriptors::Descriptor> descriptor;
        public:

            /**
             * \brief Default constructor
             *
             * \param[in] nbNeighbors the number of neighbors to measure.
             * \param[in] descriptor the function mapping the metrics of an individual to its descriptor values.
             * \param[in] metricHash hash of the metric the assigner reads (e.g. the one of its descriptor).
             * \param[in] name name of the assigner, only used to print it (e.g. in the summary of an evolution).
             */
            NoveltySearchAssigner(size_t nbNeighbors, std::shared_ptr<const Descriptors::Descriptor> descriptor, uint64_t metricHash, std::string name = "NoveltySearchAssigner")
                : Assigner(metricHash, std::move(name)), nbNeighbors{nbNeighbors}, descriptor{std::move(descriptor)} {
                if(this->descriptor == nullptr) {
                    throw std::invalid_argument("FitnessAssignment::NoveltySearchAssigner: the descriptor should not be null.");
                }
            };

            /// Return the number of neighbors of the novelty score.
            size_t getNbNeighbors() const { return this->nbNeighbors; }

            /// Return the descriptor.
            const Descriptors::Descriptor& getDescriptor() const { return *this->descriptor; }

            /// Inherited from Named, adds the number of neighbors and the details of the descriptor.
            virtual std::string getDetails() const override
            {
                return this->getName() + "(" + std::to_string(this->nbNeighbors) + " neighbors, " + this->descriptor->getDetails() + ")";
            }


            /**
             * \brief Compute the descriptor values of the individuals with the descriptor.
             *
             * \param[in] individualMetrics the metrics of each individual, indexed by individual ID.
             *
             * \return A map of each individual ID with the descriptor values. ID is used to reduce memory coset and increase lisibility
             */
            virtual std::map<uint64_t, std::vector<double>> computeDescriptors(
                const std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>& individualMetrics) const;
            
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
             * \param[in] individualMetrics the metrics of each individual, indexed by individual ID.
             * \return the fitness of each individual, indexed by individual ID.
             */
            virtual std::map<uint64_t, double> assignFitness(
                const std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>& individualMetrics) const override;
    };
};

#endif // NOVELTY_SEARCH_ASSIGNER_H