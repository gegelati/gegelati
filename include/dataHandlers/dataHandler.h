#ifndef DATA_HANDLER_H
#define DATA_HANDLER_H

#include <typeinfo>
#include <vector>

#include "supportedTypes.h"

namespace std {
	/*
	* \brief  Equality operator to std to enable use of standard algorithm on vectors of reference_wrapper of const std::type_info.
	*/
	bool operator==(const std::reference_wrapper<const std::type_info>& r0, const std::reference_wrapper<const std::type_info>& r1);
}

namespace DataHandlers {
	/**
	* \brief Base class for all sources of data to be accessed by a TPG Instruction executed within a Program.
	*/
	class DataHandler {

	protected:
		/**
		* \brief List of the types of the operands needed to execute the instruction.
		*
		* Because std::unordered_set was too complex to use (because it does not support std::reference_wrapper easily), std::vector is used instead.*
		* Adding the same type several type to the list of providedType will lead to undefined behavior.
		*/
		std::vector<std::reference_wrapper<const std::type_info>> providedTypes;

	public:
		/**
		* \brief Default constructor of the DataHandler class.
		*/
		DataHandler() : providedTypes() {}

		/**
		* \brief Check a given DataHandler can handle data for the given data type.
		*
		* \param[in] type the std::type_info whose availability in the DataHandler is being tested.
		* \return true if the DataHandler can handle data for the given data type, and false otherwise.
		*/
		bool canHandle(const std::type_info& type) const;

		/**
		* \brief Retrieve the set of types provided by the DataHandler.
		*
		* \return a const reference to the data type set provided by the DataHandler.
		*/
		const std::vector<std::reference_wrapper<const std::type_info>>& getHandledTypes() const;
	};
}

#endif