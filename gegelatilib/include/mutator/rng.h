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
	class RNG {
	protected:
		/// Mersenne twister MT19937 engine used for Random Number generation.
		std::mt19937_64 engine;

	public:
		/**
		* \brief Default seeding constructor for RNG.
		*
		* \param[in] seed the seed for the engine.
		*/
		RNG(uint64_t seed = 0) : engine(seed) {}

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
		* \brief Get a pseudo random double number between two bounds (included).
		*
		* \param[in] min the lower bound.
		* \param[in] max the upper bound.
		* \return an uniformely selected value between min and max includes.
		*/
		double getDouble(double min, double max);
	};
};

#endif