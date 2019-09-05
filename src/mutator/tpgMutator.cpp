#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <vector>

#include "tpg/tpgEdge.h"
#include "tpg/tpgTeam.h"
#include "tpg/tpgAction.h"
#include "tpg/tpgGraph.h"

#include "mutator/rng.h"
#include "mutator/programMutator.h"
#include "mutator/tpgMutator.h"

void Mutator::TPGMutator::initRandomTPG(TPG::TPGGraph& graph, size_t nbAction, size_t maxInitOutgoingEdges, size_t maxProgramSize)
{
	if (maxInitOutgoingEdges > nbAction) {
		throw std::runtime_error("Maximum initial number of outgoing edges cannot exceed the number of action");
	}
	if (nbAction < 2) {
		throw std::runtime_error("A TPG with a single action makes no sense.");
	}
	// Empty graph
	graph.clear();

	// Create teams, programs and Actions
	std::vector<const TPG::TPGAction*> actions;
	std::vector<const TPG::TPGTeam*> teams;
	std::vector<std::shared_ptr<Program::Program>> programs;
	for (size_t i = 0; i < nbAction; i++) {
		actions.push_back(&(graph.addNewAction(i)));
		teams.push_back(&(graph.addNewTeam()));
	}
	for (size_t i = 0; i < 2 * nbAction; i++) {
		programs.emplace_back(new Program::Program(graph.getEnvironment()));
		// RandomInit the Programs
		Mutator::ProgramMutator::initRandomProgram(*programs.back(), maxProgramSize);
	}

	// Connect each team with two distinct actions, through two distinct programs
	// Association here are determinists since randomness would uselessly 
	// complicate the code while bringing no real value since anyway, Programs
	// have been initialized randomly.
	for (size_t i = 0; i < 2 * nbAction; i++) {
		graph.addNewEdge(*teams.at(i / 2), *actions.at(((i / 2) + (i % 2)) % nbAction), programs.at(i));
	}

	//Add additional connections to TPG 
	// Team-by-Team
	for (const TPG::TPGTeam* team : teams) {
		// Pick a number of additional outedge
		size_t nbAdditionalEdges = RNG::getUnsignedInt64(0, maxInitOutgoingEdges - 2);

		// For each additional edge to add
		for (uint64_t i = 0; i < nbAdditionalEdges; i++) {
			// Pick 2 random programs not already used by the Team
			int64_t randomProgIndex[2] = { -1,-1 };
			int pickedProgram = 0;
			{
				// Copy the list of programs
				std::vector<int> availableChoices(programs.size());
				std::iota(availableChoices.begin(), availableChoices.end(), 0);
				// Remove already connected ones
				auto iter = availableChoices.begin();
				while (iter < availableChoices.end()) {
					if (std::count_if(team->getOutgoingEdges().begin(), team->getOutgoingEdges().end(),
						[&iter, &programs](const TPG::TPGEdge* edge) {
							return &edge->getProgram() == programs.at(*iter).get();
						}) > 0) {
						iter = availableChoices.erase(iter);
					}
					else {
						iter++;
					}
				}

				// Pick two programs (if possible, maybe only one is available)
				for (int i = 0; i < 2 && availableChoices.size() > 0; i++) {
					uint64_t progNr = RNG::getUnsignedInt64(0, availableChoices.size() - 1);
					randomProgIndex[i] = availableChoices.at(progNr);
					availableChoices.erase(availableChoices.begin() + progNr);
					pickedProgram++;
				}
			}
			// Select the least used program for the connection
			uint64_t selectedProgramIndex =
				(pickedProgram > 1 && programs.at(randomProgIndex[1]).use_count() < programs.at(randomProgIndex[0]).use_count()) ?
				randomProgIndex[1] : randomProgIndex[0];

			// Add the connection
			graph.addNewEdge(*team,
				*actions.at(((selectedProgramIndex / 2) + (selectedProgramIndex % 2)) % nbAction),
				programs.at(selectedProgramIndex));
		}
	}
}
