#ifndef PRIMITIVE_TYPE_ARRAY
#define PRIMITIVE_TYPE_ARRAY

#include "dataHandler.h"

namespace DataHandlers {
	/**
	* DataHandler for manipulating arrays of a primitive data type.
	*/
	template <class T> class PrimitiveTypeArray : public DataHandler {
		static_assert(std::is_fundamental<T>::value, "Template class PrimitiveTypeArray<T> can only be used for primitive types.");

	protected:
		/**
		* \brief Number of elements contained in the Array.
		*
		* Although this may seem redundant with the data.size() method, this attribute is here to 
		* make it possible to check whether the size of the data vector was modified throughout
		* the lifetime of the PrimitiveTypeArray. (Which should not be possible.)
		*/
		const unsigned int nbElements;

		/**
		* \brief Array storing the data of the PrimitiveTypeArray.
		*/
		std::vector<T> data;

	public:
		/**
		*  \brief Constructor for the PrimitiveTypeArray class.
		*
		* \param[in] size the fixed number of elements of primitive type T contained in the PrimitiveTypeArray.
		*/
		PrimitiveTypeArray(unsigned int size = 8);

	};

	template <class T> PrimitiveTypeArray<T>::PrimitiveTypeArray(unsigned int size) : nbElements{size}, data(size) {
		this->handledTypes.insert(typeid(PrimitiveType<T>));
	}
}

#endif 
