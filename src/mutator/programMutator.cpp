#include "mutator/rng.h"
#include "mutator/programMutator.h"

bool Mutator::ProgramMutator::deleteRandomLine(Program::Program& p)
{
	// Line cannot be removed from a program with a single line.
	if (p.getNbLines() <= 1) {
		return false;
	}

	uint64_t lineIndex = Mutator::RNG::getUnsignedInt64(0, p.getNbLines() - 1);
	p.removeLine(lineIndex);
	return true;
}

void Mutator::ProgramMutator::insertRandomLine(Program::Program& p)
{
	uint64_t lineIndex = Mutator::RNG::getUnsignedInt64(0, p.getNbLines());
	Program::Line& line = p.addNewLine(lineIndex);
	Mutator::LineMutator::initRandomCorrectLine(line);
}
