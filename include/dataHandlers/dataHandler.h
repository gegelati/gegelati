#ifndef DATA_HANDLER_H
#define DATA_HANDLER_H

#include <functional>
#include <typeinfo>
#include <unordered_set>

#include "supportedTypes.h"

namespace DataHandlers {

	/**
	* \brief Hashing lambda expression used to enable creation of std::unordered_set of SupportedType.
	*/
	auto hash = [](const std::reference_wrapper<const std::type_info>& n) { return typeid(n.get()).hash_code(); };
	
	/**
	* \brief Equality test lambda expression used to enable creation of std::unordered_set of SupportedType.
	*/
	auto equal = [](const  std::reference_wrapper<const std::type_info>& n1, const  std::reference_wrapper<const std::type_info>& n2) { return n1.get() == n2.get(); };

	/**
	* \brief Base class for all sources of data to be accessed by a TPG Instruction executed within a Program.
	*/
	class DataHandler {

	protected:
		/**
		* \brief List of the types of the operands needed to execute the instruction.
		*/
		std::unordered_set<std::reference_wrapper<const std::type_info>, decltype(hash), decltype(equal) > handledTypes;

	public:
		/**
		* \brief Default constructor of the DataHandler class.
		*/
		DataHandler() : handledTypes(8, hash ,equal) {}

		/**
		* \brief Check a given DataHandler can provide data for the given data type.
		*
		* \param[in] type the std::type_info whose availability in the DataHandler is being tested.
		* \return true if the DataHandler can provide data for the given data type, and false otherwise.
		*/
		bool canProvide(const std::type_info& type) const;
	};
}

#endif