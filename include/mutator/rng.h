#ifndef RNG_H
#define RNG_H

#include <random>

namespace Mutator {

	/**
	* Static class containing the (pseudo) Random Number Generator facilities to be
	* used in the TPG framework.
	*
	* This namespace currently provides a wrapper around the mt19937_64 engine
	* and all methods generating random numbers adopt a uniform distribution.
	*/
	class RNG {
	public:

		/// Mersenne twister MT19937 engine used for Random Number generation.
		static std::mt19937_64 engine;

		/// Delete the constructor.
		RNG() = delete;

		/**
		* \brief Set the seed of the random number generator.
		*
		* \param[in] the seed value for generating random numbers.
		*/
		static void setSeed(uint64_t seed);

		/**
		* \brief Get a pseudo random int number between two bounds (included).
		*
		* \param[in] min the lower bound.
		* \param[in] max the upper bound.
		* \return an uniformely selected value between min and max includes.
		*/
		static uint64_t getUnsignedInt64(uint64_t min, uint64_t max);
	};
};

#endif