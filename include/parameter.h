#ifndef PARAMETER_H
#define PARAMETER_H

/**
* \brief The parameter type is an union of 4 byte primitive types.
*
* The union construct is chosen to allow casting from one type to the other 
* when interpreting the parameters contained within an programLine.
*/
union Parameter {
	/// Integer value of the union.
	int32_t i;
	/// Float value of the union.
	float f;

	/// Assignment Operator for int
	Parameter& operator =(const int& other) { this->i = other;  return *this; };
	/// Copy constructor operator for int
	Parameter(const int& other) : i{ other } {};
	/// Cast to int
	operator int() { return this->i; };

	/// Assignment Operator for float
	Parameter& operator =(const float& other) { this->f = other;  return *this; };
	/// Copy constructor operator for float
	Parameter(const float& other) : f{ other } {};
	/// Cast to float
	operator float() { return this->f; };
};

/// Maximum int value for an integral Parameter
#define PARAM_INT_MAX INT32_MAX

/// Minimum int value for an integral Parameter
#define PARAM_INT_MIN INT32_MIN

#endif