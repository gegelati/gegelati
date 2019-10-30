#include <algorithm>

#include "instructions/set.h"

bool Instructions::Set::add(const Instruction& instruction)
{
	this->instructions.push_back(instruction);

	return true;
}

unsigned int Instructions::Set::getNbInstructions() const
{
	return (unsigned int)instructions.size();
}

const Instructions::Instruction& Instructions::Set::getInstruction(const uint64_t i) const
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

unsigned int Instructions::Set::getMaxNbParameters() const
{
	unsigned int res = 0;
	for (auto instruction : this->instructions) {
		unsigned int nb = instruction.get().getNbParameters();
		res = (nb > res) ? nb : res;
	}
	return res;
}
