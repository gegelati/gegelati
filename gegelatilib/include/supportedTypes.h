#ifndef SUPPORTED_TYPES_H
#define SUPPORTED_TYPES_H

#include <typeinfo>

/**
* \brief Base class to declare all types of Instruction operands, and provided by HandlerData.
*/
class SupportedType {
public:
	/**
	* \brief Virtual destructor to force class polymorphism.
	*/
	virtual ~SupportedType() {};
};

/**
* \brief Template class to simplify creation of SupportedType derivate for primitive types.
*/
template <class T> class PrimitiveType : public SupportedType {
	static_assert(std::is_fundamental<T>::value, "Template class AddPrimitiveType<T> can only be used for primitive types.");

protected:
	/// copied value of the primitive object.
	T value;

public:
	/// Default constructor
	PrimitiveType() : value{ 0 } {};

	/// Cast operator back to the primitive type for convenient use.
	operator T() const { return value; };

	/// Assignment operator from primitive type.
	T& operator =(const T& other) { value = other;  return this->value; };

	/// Copy constructor.
	PrimitiveType(const T& other) :value{ other } {};
};


#endif

