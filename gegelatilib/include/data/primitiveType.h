#ifndef PRIMITIVE_TYPE_H
#define PRIMITIVE_TYPE_H

#include <typeinfo>
#include <type_traits>

namespace Data {
	/**
	* \brief Template class to simplify creation of SupportedType derivate for primitive types.
	* TODO: Remove (after cleaning up removal of SupportedType)
	*/
	template <class T> class PrimitiveType {
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
}

#endif // !PRIMITIVE_TYPE_H