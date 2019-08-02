#include <algorithm>

#include "instructions/set.h"

bool Instructions::Set::add(const Instruction& instruction)
{
	// Check for presence
	for (auto inst : this->instructions) {
		// Instruction are compared by their type.
		if (typeid(inst.get()) == typeid(instruction)) {
			return false;
		}
	}

	// If the instruction was not yet present in the set
	this->instructions.push_back(instruction);

	return true;
}

unsigned int Instructions::Set::getNbInstructions() const
{
	return (unsigned int)instructions.size();
}

const Instructions::Instruction& Instructions::Set::getInstruction(const unsigned int i) const
{
	return instructions.at(i).get();
}

unsigned int Instructions::Set::getMaxNbOperands() const
{
	unsigned int res = 0;
	for (auto instruction : this->instructions) {
		unsigned int nb = instruction.get().getNbOperands();
		res = (nb > res)? nb : res;
	}
	return res;
}
