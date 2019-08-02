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
		* Instruction added to the set are stored and indexed in the order in which they are added.
		*
		* \param[in] instruction the Instruction to be added to the Set.
		* \return true if the instruction was successfully added, false if it was already present in the set.
		*/
		bool add(const Instruction& instruction);

		/**
		* \brief Get the number of unique Instruction contained in the Set.
		*
		* \return the int corresponding to the number of unique Instruction in the Set.
		*/ 
		unsigned int getNbInstructions() const;

		/**
		* \brief Get the Instruction at the given index i in the Set.
		*
		* \param[in] i the unsigned integer index of the Instruction to retrieve.
		* \throws std::out_of_range if the given index exceeds the number of Instruction in the Set.
		* \return a reference to the Instruction at the given index.
		*/
		const Instruction& getInstruction(const unsigned int i) const;

		/**
		* \brief Get the maximum number of operands for Instruction currently contained in the Set.
		*
		* \return the unsigned int value corresponding to the largest number 
		* of operands needed to execute an Instruction in the Set.
		*/
		unsigned int getMaxNbOperands() const;
	};
}

#endif