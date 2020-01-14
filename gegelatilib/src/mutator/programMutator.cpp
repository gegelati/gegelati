#include "mutator/mutationParameters.h"
#include "mutator/rng.h"
#include "mutator/programMutator.h"

void Mutator::ProgramMutator::initRandomProgram(Program::Program& p, const MutationParameters& params, Mutator::RNG& rng)
{
	// Empty the program
	while (p.getNbLines() > 0) {
		p.removeLine(0);
	}

	// Select the number of line randomly
	const uint64_t nbLine = rng.getUnsignedInt64(1, params.prog.maxProgramSize);
	// Insert them
	while (p.getNbLines() < nbLine) {
		insertRandomLine(p, rng);
	}

	// Identify Introns
	p.identifyIntrons();
}

bool Mutator::ProgramMutator::deleteRandomLine(Program::Program& p, Mutator::RNG& rng)
{
	// Line cannot be removed from a program with a single line.
	if (p.getNbLines() <= 1) {
		return false;
	}

	uint64_t lineIndex = rng.getUnsignedInt64(0, p.getNbLines() - 1);
	p.removeLine(lineIndex);
	return true;
}

void Mutator::ProgramMutator::insertRandomLine(Program::Program& p, Mutator::RNG& rng)
{
	uint64_t lineIndex = rng.getUnsignedInt64(0, p.getNbLines());
	Program::Line& line = p.addNewLine(lineIndex);
	Mutator::LineMutator::initRandomCorrectLine(line, rng);
}

bool Mutator::ProgramMutator::swapRandomLines(Program::Program& p, Mutator::RNG& rng)
{
	if (p.getNbLines() < 2) {
		return false;
	}
	// Select two distinct random index.
	const uint64_t lineIndex0 = rng.getUnsignedInt64(0, p.getNbLines() - 1);
	uint64_t lineIndex1 = rng.getUnsignedInt64(0, p.getNbLines() - 2);
	lineIndex1 += (lineIndex1 >= lineIndex0) ? 1 : 0;

	p.swapLines(lineIndex0, lineIndex1);

	return true;
}

bool Mutator::ProgramMutator::alterRandomLine(Program::Program& p, Mutator::RNG& rng)
{
	if (p.getNbLines() < 1) {
		return false;
	}
	// Select a random index.
	const uint64_t lineIndex = rng.getUnsignedInt64(0, p.getNbLines() - 1);
	Mutator::LineMutator::alterCorrectLine(p.getLine(lineIndex), rng);
	return true;
}

bool Mutator::ProgramMutator::mutateProgram(Program::Program& p, const MutationParameters& params, Mutator::RNG& rng)
{
	bool anyMutation = false;
	if (p.getNbLines() > 1 && rng.getDouble(0.0, 1.0) < params.prog.pDelete) {
		anyMutation = true;
		deleteRandomLine(p, rng);
	}

	if (p.getNbLines() < params.prog.maxProgramSize && rng.getDouble(0.0, 1.0) < params.prog.pAdd) {
		anyMutation = true;
		insertRandomLine(p, rng);
	}

	if (rng.getDouble(0.0, 1.0) < params.prog.pMutate) {
		anyMutation = true;
		alterRandomLine(p, rng);
	}

	if (rng.getDouble(0.0, 1.0) < params.prog.pSwap) {
		anyMutation = true;
		swapRandomLines(p, rng);
	}

	// Identify introns
	if (anyMutation) {
		p.identifyIntrons();
	}

	return anyMutation;
}
