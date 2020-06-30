/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019)
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

#ifndef RNG_H
#define RNG_H

#include <random>

namespace Mutator {

    /**
     * Class containing the (pseudo) Random Number Generator facilities to be
     * used in the TPG framework.
     *
     * This class currently provides a wrapper around the mt19937_64 engine
     * and all methods generating random numbers adopt a uniform distribution.
     */
    class RNG
    {
      protected:
        /// Mersenne twister MT19937 engine used for Random Number generation.
        std::mt19937_64 engine;

      public:
        /**
         * \brief Default seeding constructor for RNG.
         *
         * \param[in] seed the seed for the engine.
         */
        RNG(uint64_t seed = 0) : engine(seed)
        {
        }

        /**
         * \brief Set the seed of the random number generator.
         *
         * \param[in] seed integer value for generating random numbers.
         */
        void setSeed(uint64_t seed);

        /**
         * \brief Get a pseudo random int number between two bounds (included).
         *
         * \param[in] min the lower bound.
         * \param[in] max the upper bound.
         * \return an uniformely selected value between min and max includes.
         */
        uint64_t getUnsignedInt64(uint64_t min, uint64_t max);

        /**
         * \brief Get a pseudo random double number between two bounds
         * (included).
         *
         * \param[in] min the lower bound.
         * \param[in] max the upper bound.
         * \return an uniformely selected value between min and max includes.
         */
        double getDouble(double min, double max);
    };
}; // namespace Mutator

#endif
