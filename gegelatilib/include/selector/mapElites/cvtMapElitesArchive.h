#ifndef CVT_MAP_ELITES_ARCHIVE_H
#define CVT_MAP_ELITES_ARCHIVE_H

#include "mapElitesArchive.h"
#include <gegelati.h>

namespace Selector {
    namespace MapElites {

        /**
         * \brief CVT Map Elites Archive class
         *
         * This class implements a CVT (Centroidal Voronoi Tessellation) based
         * Map Elites Archive It inherits from the MapElitesArchive class.
         */
        class CvtMapElitesArchive : public MapElitesArchive
        {
          protected:
            /// Number of centroids
            size_t nbCentroids;

            /// Number of iterations for initialization
            size_t nbIterationInit;

            /// Number of dots for initialization
            size_t nbDotsInit;

            /// CVT parameters
            double a1;

            /// CVT parameters
            double b1;

            /// CVT parameters
            double a2;

            /// CVT parameters
            double b2;

            /// Centroids of the CVT
            std::vector<std::vector<double>> centroids;

          public:
            /**
             * \brief Constructor of the CVT Map Elites Archive
             *
             * \param[in] rng Random number generator
             * \param[in] nbCentroids Number of centroids
             * \param[in] nbDescriptors Number of descriptors
             * \param[in] minValue Minimum value for the descriptors
             * \param[in] maxValue Maximum value for the descriptors
             * \param[in] nbIterationInit Number of iterations for
             * initialization
             * \param[in] nbDotsInit Number of dots for initialization
             * \param[in] a1 CVT parameter
             * \param[in] b1 CVT parameter
             * \param[in] a2 CVT parameter
             * \param[in] b2 CVT parameter
             */
            CvtMapElitesArchive(RNG::RNG& rng, size_t nbCentroids,
                                size_t nbDescriptors, double minValue,
                                double maxValue, size_t nbIterationInit,
                                size_t nbDotsInit, double a1, double b1,
                                double a2, double b2)
                : MapElitesArchive(0, nbDescriptors, minValue, maxValue),
                  nbCentroids{nbCentroids}, nbIterationInit{nbIterationInit},
                  nbDotsInit{nbDotsInit}, a1{a1}, b1{b1}, a2{a2}, b2{b2}
            {
                centroids.resize(nbCentroids);
                archive.resize(nbCentroids);
                initialize_cvt(rng);
            }

            /**
             * \brief Calculate squared Euclidean distance between two points
             *
             * \param[in] a First point
             * \param[in] b Second point
             */
            double dist_squared(const std::vector<double>& a,
                                const std::vector<double>& b);

            /**
             * \brief Vector addition
             *
             * \param[in] a First vector
             * \param[in] b Second vector
             */
            std::vector<double> add(const std::vector<double>& a,
                                    const std::vector<double>& b);

            /**
             * \brief Scalar multiplication
             *
             * \param[in] a Vector
             * \param[in] s Scalar
             */
            std::vector<double> scalar_mult(const std::vector<double>& a,
                                            double s);

            /**
             * \brief Compute the average of a set of points
             *
             * \param[in] points Set of points
             */
            std::vector<double> average(
                const std::vector<std::vector<double>>& points);

            /**
             * \brief Generate a random point within the descriptor bounds
             *
             * \param[in] rng Random number generator
             */
            std::vector<double> random_point(RNG::RNG& rng);

            /**
             * \brief Find the nearest centroid to a given point
             *
             * \param[in] point The point to find the nearest centroid for
             * \param[in] centroids The list of centroids
             */
            size_t nearest(const std::vector<double>& point,
                           const std::vector<std::vector<double>>& centroids);

            /**
             * \brief Initialize the CVT centroids using Lloyd's algorithm
             *
             * \param[in] rng Random number generator
             */
            void initialize_cvt(RNG::RNG& rng);

            /**
             * \brief Get the index for given descriptors
             *
             * \param[in] descriptors The descriptors to get the index for
             */
            size_t getIndexForDescriptor(
                const std::vector<double>& descriptors) const;

            /**
             * \brief Get the archive content at given descriptors
             */
            const std::pair<std::shared_ptr<Learn::EvaluationResult>,
                            std::optional<std::reference_wrapper<const Algorithm::Agent>>>&
            getArchiveFromDescriptors(
                const std::vector<double>& descriptors) const override;

            /**
             * \brief Set the archive content at given descriptors
             */
            void setArchiveFromDescriptors(
                const Algorithm::Agent& agent,
                std::shared_ptr<Learn::EvaluationResult> eval,
                const std::vector<double>& descriptors) override;

            /**
             * \brief Return the centroids used by the CVT.
             */
            virtual const std::vector<std::vector<double>>& getCentroids()
                const;
        };
    }; // namespace MapElites
}; // namespace Selector

#endif