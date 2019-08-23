#include "mutator/rng.h"
#include "mutator/programMutator.h"

void Mutator::ProgramMutator::deleteRandomLine(Program::Program& p)
{
	// Line cannot be removed from a program with a single line.
	if (p.getNbLines() <= 1) {
		return;
	}

	uint64_t lineIndex = Mutator::RNG::getUnsignedInt64(0, p.getNbLines() - 1);
	p.removeLine(lineIndex);
}
