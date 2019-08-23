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

bool Mutator::ProgramMutator::swapRandomLines(Program::Program& p)
{
	if (p.getNbLines() < 2) {
		return false;
	}
	// Select two distinct random index.
	const uint64_t lineIndex0 = Mutator::RNG::getUnsignedInt64(0, p.getNbLines() - 1);
	uint64_t lineIndex1 = Mutator::RNG::getUnsignedInt64(0, p.getNbLines() - 2);
	lineIndex1 += (lineIndex1 >= lineIndex0)? 1 : 0;

	p.swapLines(lineIndex0, lineIndex1);

	return true;
}

bool Mutator::ProgramMutator::alterRandomLine(Program::Program& p)
{
	if (p.getNbLines() < 1) {
		return false;
	}
	// Select a random index.
	const uint64_t lineIndex = Mutator::RNG::getUnsignedInt64(0, p.getNbLines() - 1);
	Mutator::LineMutator::alterCorrectLine(p.getLine(lineIndex));
	return true;
}
