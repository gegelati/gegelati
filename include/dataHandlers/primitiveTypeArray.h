#ifndef PRIMITIVE_TYPE_ARRAY
#define PRIMITIVE_TYPE_ARRAY

#include <sstream>
 
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
		std::vector<PrimitiveType<T>> data;

	public:
		/**
		*  \brief Constructor for the PrimitiveTypeArray class.
		*
		* \param[in] size the fixed number of elements of primitive type T contained in the PrimitiveTypeArray.
		*/
		PrimitiveTypeArray(unsigned int size = 8);

		size_t getAddressSpace(const std::type_info& type)  const;

		/**
		* \brief Sets all elements of the Array to 0 (or its equivalent for the given template param.
		*/
		void resetData();

		const SupportedType& getDataAt(const std::type_info& type, const size_t address) const;
	};

	template <class T> PrimitiveTypeArray<T>::PrimitiveTypeArray(unsigned int size) : nbElements{ size }, data(size) {
		this->providedTypes.push_back(typeid(PrimitiveType<T>));
	}

	template<class T> size_t PrimitiveTypeArray<T>::getAddressSpace(const std::type_info& type) const
	{
		if (type == typeid(PrimitiveType<T>)) {
			return this->nbElements;
		}
		// Default case
		return 0;
	}

	template<class T> void PrimitiveTypeArray<T>::resetData()
	{
		for (PrimitiveType<T> & elt : this->data) {
			elt = 0;
		}
	}

	template<class T> const SupportedType& PrimitiveTypeArray<T>::getDataAt(const std::type_info& type, const size_t address) const
	{
		size_t addressSpace = this->getAddressSpace(type);
		// check type
		if (addressSpace == 0) {
			std::stringstream  message;
			message << "Data type " << type.name() << " cannot be accessed in a " << typeid(*this).name() << ".";
			throw std::invalid_argument(message.str());
		}

		// check location
		if (address >= addressSpace) {
			std::stringstream  message;
			message << "Data type " << type.name() << " cannot be accessed at address " << address << ", address space size is " << addressSpace + ".";
			throw std::out_of_range(message.str());
		}

		return this->data[address];
	}
}

#endif 
