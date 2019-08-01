#ifndef SET_H
#define SET_H

#include "instruction.h"

namespace Instructions {
	/**
	* \brief Class for storing a set of Instruction.
	*
	* Together with storage capabilities, this class provide methods to compute
	* useful information on the ordered set of Instruction it contains.
	*/
	class Set {
	protected:
		/**
		* \brief Ordered container of the Instruction of the set.
		*/
		std::vector<std::reference_wrapper<const Instruction>> instructions;

	public:
		/**
		* \brief Default constructor.
		*
		* This constructor simply creates an empty list of instructions.
		*/
		Set() : instructions{} {};

		/**
		* \brief Add an instruction to the set.
		*
		* \return true if the instruction was successfully added, false if it was already present in the set.
		*/
		bool add(const Instruction& instruction);
	};
}

#endif