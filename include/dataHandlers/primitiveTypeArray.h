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
		const size_t nbElements;

		/**
		* \brief Array storing the data of the PrimitiveTypeArray.
		*/
		std::vector<PrimitiveType<T>> data;

		/**
		* Check whether the given type of data can be accessed at the given address. Throws exception otherwise.
		*
		* \param[in] type the std::type_info of data.
		* \param[in] address the location of the data.
		* \throws std::invalid_argument if the given data type is not provided by the DataHandler.
		* \throws std::out_of_range if the given address is invalid for the given data type.
		*/
		void checkAddressAndType(const type_info& type, const size_t& address) const;

	public:
		/**
		*  \brief Constructor for the PrimitiveTypeArray class.
		*
		* \param[in] size the fixed number of elements of primitive type T contained in the PrimitiveTypeArray.
		*/
		PrimitiveTypeArray(size_t size = 8);

		size_t getAddressSpace(const std::type_info& type)  const;

		/**
		* \brief Sets all elements of the Array to 0 (or its equivalent for the given template param.
		*/
		void resetData();

		const SupportedType& getDataAt(const std::type_info& type, const size_t address) const;

		/**
		* \brief Set the data at the given address to the given value.
		*
		* This method is not (yet) part of the DataHandler class as for now, the
		* TPG engine does not need to update data of DataHandler, except for
		* the one used as registers, which are managed with a
		* PrimitiveTypeArray<double>.
		*
		* \param[in] type the std::type_info of data set.
		* \param[in] address the location of the data to set.
		* \param[in] value a const reference to the PrimitiveType holding a
		*            value of the data to write in the current PrimitiveTypeArray
		* \throws std::invalid_argument if the given data type is not handled by the DataHandler.
		* \throws std::out_of_range if the given address is invalid for the given data type.
		*/
		void setDataAt(const std::type_info& type, const size_t address, const PrimitiveType<T>& value);
	};

	template <class T> PrimitiveTypeArray<T>::PrimitiveTypeArray(size_t size) : nbElements{ size }, data(size) {
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
		for (PrimitiveType<T>& elt : this->data) {
			elt = 0;
		}
	}

	template<class T>
	void PrimitiveTypeArray<T>::checkAddressAndType(const type_info& type, const size_t& address) const
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
	}

	template<class T> const SupportedType& PrimitiveTypeArray<T>::getDataAt(const std::type_info& type, const size_t address) const
	{
		// Throw exception in case of invalid arguments.
		checkAddressAndType(type, address);

		return this->data[address];
	}

	template<class T> void PrimitiveTypeArray<T>::setDataAt(
		const std::type_info& type,
		const size_t address,
		const PrimitiveType<T>& value) {
		// Throw exception in case of invalid arguments.
		checkAddressAndType(type, address);

		this->data[address] = value;
	}
}

#endif 
