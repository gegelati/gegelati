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
