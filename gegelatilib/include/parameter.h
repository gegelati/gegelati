/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019)
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

#ifndef PARAMETER_H
#define PARAMETER_H

#include <cstdint>
#include <cmath>
/**
* \brief The parameter type is an 2 byte primitive type.
*
* Conversion to float value between -1 and ~0.9999 is possible.
*/
class Parameter {
public:
	/// Integer value of the class.
	int16_t i;

	/// Assignment Operator for char16_t
	Parameter& operator =(const int16_t& other) { this->i = other;  return *this; };
	/// Copy constructor operator for char16_t
	Parameter(const int16_t& other) : i{ other } {};
	/// Cast to int
	operator int16_t() const { return this->i; };
	/// Assignment Operator for float
	Parameter& operator =(const float& other) {
		if (other > ((float)INT16_MAX / (float)(-INT16_MIN))) {
			this->i = INT16_MAX;  return *this;
		}
		else if (other < -1.0f) {
			this->i = INT16_MIN;
		}
		else {
			this->i = (uint16_t)(roundf(other * (-INT16_MIN)));
		}
		return *this;
	};
	/// Copy constructor operator for char16_t
	Parameter(const float& other) {
		this->operator=(other);
	};
	/// Cast to float
	operator float() const { return (float)this->i / (float)(-INT16_MIN); };
};

/// Maximum int value for an integral Parameter
#define PARAM_INT_MAX INT16_MAX

/// Minimum int value for an integral Parameter
#define PARAM_INT_MIN INT16_MIN

/// Precision in float point set/get
#define PARAM_FLOAT_PRECISION (float)(int16_t(1) / (float)(-INT16_MIN))

#endif
