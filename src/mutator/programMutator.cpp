#include "mutator/rng.h"
#include "mutator/programMutator.h"

void Mutator::ProgramMutator::initRandomProgram(Program::Program& p, size_t maxProgSize)
{
	// Empty the program
	while (p.getNbLines() > 0) {
		p.removeLine(0);
	}

	// Select the number of line randomly
	const uint64_t nbLine = Mutator::RNG::getUnsignedInt64(1, maxProgSize);
	// Insert them
	while (p.getNbLines() < nbLine) {
		insertRandomLine(p);
	}
}

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
	lineIndex1 += (lineIndex1 >= lineIndex0) ? 1 : 0;

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

bool Mutator::ProgramMutator::mutateProgram(Program::Program& p, double pDelete, double pAdd, size_t maxProgramSize, double pMutate, double pSwap)
{
	bool anyMutation = false;
	if (p.getNbLines() > 1 && Mutator::RNG::getDouble(0.0, 1.0) < pDelete) {
		anyMutation = true;
		deleteRandomLine(p);
	}

	if (p.getNbLines() < maxProgramSize && Mutator::RNG::getDouble(0.0, 1.0) < pAdd) {
		anyMutation = true;
		insertRandomLine(p);
	}

	if (Mutator::RNG::getDouble(0.0, 1.0) < pMutate) {
		anyMutation = true;
		alterRandomLine(p);
	}

	if (Mutator::RNG::getDouble(0.0, 1.0) < pSwap) {
		anyMutation = true;
		swapRandomLines(p);
	}

	return anyMutation;
}
