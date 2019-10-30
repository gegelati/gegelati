#ifndef PARAMETER_H
#define PARAMETER_H

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
#define PARAM_FLOAT_PRECISION (float)(1i16 / (float)(-INT16_MIN))

#endif