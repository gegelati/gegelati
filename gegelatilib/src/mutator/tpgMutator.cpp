#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <vector>

#include "archive.h"

#include "tpg/tpgEdge.h"
#include "tpg/tpgTeam.h"
#include "tpg/tpgAction.h"
#include "tpg/tpgGraph.h"
#include "program/programExecutionEngine.h"

#include "mutator/mutationParameters.h"
#include "mutator/rng.h"
#include "mutator/programMutator.h"
#include "mutator/tpgMutator.h"

void Mutator::TPGMutator::initRandomTPG(TPG::TPGGraph& graph, const Mutator::MutationParameters& params)
{
	if (params.tpg.maxInitOutgoingEdges > params.tpg.nbActions) {
		throw std::runtime_error("Maximum initial number of outgoing edges cannot exceed the number of action");
	}
	if (params.tpg.nbActions < 2) {
		throw std::runtime_error("A TPG with a single action makes no sense.");
	}
	// Empty graph
	graph.clear();

	// Create teams, programs and Actions
	std::vector<const TPG::TPGAction*> actions;
	std::vector<const TPG::TPGTeam*> teams;
	std::vector<std::shared_ptr<Program::Program>> programs;
	for (size_t i = 0; i < params.tpg.nbActions; i++) {
		actions.push_back(&(graph.addNewAction(i)));
		teams.push_back(&(graph.addNewTeam()));
	}
	for (size_t i = 0; i < 2 * params.tpg.nbActions; i++) {
		programs.emplace_back(new Program::Program(graph.getEnvironment()));
		// RandomInit the Programs
		Mutator::ProgramMutator::initRandomProgram(*programs.back(), params);
	}

	// Connect each team with two distinct actions, through two distinct programs
	// Association here are determinists since randomness would uselessly 
	// complicate the code while bringing no real value since anyway, Programs
	// have been initialized randomly.
	for (size_t i = 0; i < 2 * params.tpg.nbActions; i++) {
		graph.addNewEdge(*teams.at(i / 2), *actions.at(((i / 2) + (i % 2)) % params.tpg.nbActions), programs.at(i));
	}

	//Add additional connections to TPG 
	// Team-by-Team
	for (const TPG::TPGTeam* team : teams) {
		// Pick a number of additional outedge
		size_t nbAdditionalEdges = RNG::getUnsignedInt64(0, params.tpg.maxInitOutgoingEdges - 2);

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
				*actions.at(((selectedProgramIndex / 2) + (selectedProgramIndex % 2)) % params.tpg.nbActions),
				programs.at(selectedProgramIndex));
		}
	}
}

void Mutator::TPGMutator::removeRandomEdge(TPG::TPGGraph& graph, const TPG::TPGTeam& team) {
	// Pick an outgoing edge randomly, 
	// Copy the set 
	std::list<TPG::TPGEdge*> pickableEdges = team.getOutgoingEdges();
	auto isTPGAction = [](const TPG::TPGEdge* edge)->bool {
		return typeid(*edge->getDestination()) == typeid(TPG::TPGAction);
	};

	// if there is a unique TPGAction among the edges, exclude it from the pickable edges
	if (std::count_if(pickableEdges.begin(), pickableEdges.end(), isTPGAction) == 1) {
		for (auto iter = pickableEdges.begin(); iter != pickableEdges.end();) {
			if (isTPGAction(*iter)) {
				iter = pickableEdges.erase(iter);
			}
			else {
				iter++;
			}
		}
	}

	// Pick a random edge
	auto iterSet = pickableEdges.begin();
	std::advance(iterSet, Mutator::RNG::getUnsignedInt64(0, pickableEdges.size() - 1));
	const TPG::TPGEdge* removedEdge = *iterSet;
	graph.removeEdge(*removedEdge);
}

void Mutator::TPGMutator::addRandomEdge(TPG::TPGGraph& graph, const TPG::TPGTeam& team,
	const std::list<const TPG::TPGEdge*>& preExistingEdges) {
	// Pick an edge (excluding ones from the team and edges with the team as a destination)
	auto pickableEdges(preExistingEdges);
	// cf erase-remove idiom
	pickableEdges.erase(std::remove_if(pickableEdges.begin(), pickableEdges.end(),
		[&team](const TPG::TPGEdge* edge)-> bool {
			return edge->getSource() == &team || edge->getDestination() == &team;
		}), pickableEdges.end());

	// Pick a pickable Edge 
	// (This code assumes that the set of pickable edge is never empty.. 
	// otherwise it will throw an exception. Possible solution if needed
	// initialize an entirely new program and pick a random target.) 
	std::list<const TPG::TPGEdge*>::iterator iter = pickableEdges.begin();
	std::advance(iter, Mutator::RNG::getUnsignedInt64(0, pickableEdges.size() - 1));
	const TPG::TPGEdge* pickedEdge = *iter;

	// Create new edge from team and with the same ProgramSharedPointer
	// But with the team as its source
	// throw std::runtime_error if the edge is not from the graph;
	const TPG::TPGEdge& newEdge = graph.cloneEdge(*pickedEdge);
	graph.setEdgeSource(newEdge, team);
}

void Mutator::TPGMutator::mutateEdgeDestination(TPG::TPGGraph& graph,
	const TPG::TPGTeam& team,
	const TPG::TPGEdge* edge,
	const std::vector<const TPG::TPGTeam*>& preExistingTeams,
	const std::vector<const TPG::TPGAction*>& preExistingActions,
	const Mutator::MutationParameters& params)
{
	// Pick an edge among preexisting vertices
	const TPG::TPGVertex* target = NULL;
	// Should the new target be an action or a team
	bool targetAction = Mutator::RNG::getDouble(0, 1) < params.tpg.pEdgeDestinationIsAction;

	// Check if the edge is the only of the team connected to an action.
	// in which case, selecting an action is mandatory.
	if (targetAction || (typeid(*edge->getDestination()) == typeid(TPG::TPGAction)
		&& std::count_if(team.getOutgoingEdges().begin(), team.getOutgoingEdges().end(),
			[](const TPG::TPGEdge* other) {
				return typeid(*other->getDestination()) == typeid(TPG::TPGAction);
			}) == 1)) {
		// Pick an Action target
		target = preExistingActions.at(Mutator::RNG::getUnsignedInt64(0, preExistingActions.size() - 1));
	}
	else {
		// Pick any target
		target = preExistingTeams.at(Mutator::RNG::getUnsignedInt64(0, preExistingTeams.size() - 1));
	}
	// Change the target
	// Changing the target should not fail.
	graph.setEdgeDestination(*edge, *target);
}

void Mutator::TPGMutator::mutateOutgoingEdge(TPG::TPGGraph& graph,
	const Archive& archive,
	const TPG::TPGTeam& team,
	const TPG::TPGEdge* edge,
	const std::vector<const TPG::TPGTeam*>& preExistingTeams,
	const std::vector<const TPG::TPGAction*>& preExistingActions,
	const Mutator::MutationParameters& params) {
	// copy program
	std::shared_ptr<Program::Program> newProg(new Program::Program(edge->getProgram()));
	// Mutate its behavior until it changes (against the archive).
	bool allUnique;
	do {
		// Mutate until something is mutated (i.e. the function returns true)
		while (!Mutator::ProgramMutator::mutateProgram(*newProg, params));
		// Check for uniqueness in archive
		auto archivedDataHandlers = archive.getDataHandlers();
		std::map<size_t, double> hashesAndResults;
		Program::ProgramExecutionEngine pee(*newProg);
		for (std::pair<size_t, std::vector<std::reference_wrapper<const DataHandlers::DataHandler>>> archiveDatahandler : archivedDataHandlers) {
			// Execute the mutated program on the archive data handlers
			pee.setDataSources(archiveDatahandler.second);
			double result = pee.executeProgram();
			hashesAndResults.insert({ archiveDatahandler.first, result });
		}

		// If the result is not unique, do another mutation.
		allUnique = archive.areProgramResultsUnique(hashesAndResults);
	} while (!allUnique);

	// Set the mutated program to the edge
	edge->setProgram(newProg);

	// Edge target modification
	// As it Stephen kelly's work, Edge target modification is conditionned
	// to the modification of the prealable Edge.Program behavior.
	if (Mutator::RNG::getDouble(0.0, 1.0) < params.tpg.pEdgeDestinationChange) {
		mutateEdgeDestination(graph, team, edge, preExistingTeams, preExistingActions, params);
	}
}

void Mutator::TPGMutator::mutateTPGTeam(TPG::TPGGraph& graph,
	const Archive& archive,
	const TPG::TPGTeam& team,
	const std::vector<const TPG::TPGTeam*>& preExistingTeams,
	const std::vector<const TPG::TPGAction*>& preExistingActions,
	const std::list<const TPG::TPGEdge*>& preExistingEdges,
	const Mutator::MutationParameters& params)
{
	// 1. Remove randomly selected edges
	{
		double proba = 1.0;
		while (team.getOutgoingEdges().size() > 2 && proba > Mutator::RNG::getDouble(0.0, 1.0)) {
			removeRandomEdge(graph, team);

			// Decrement the proba of removing another edge
			proba *= params.tpg.pEdgeDeletion;
		}
	}

	// 2. Add random duplicated edge with the team as its source
	{
		double proba = 1.0;
		while (team.getOutgoingEdges().size() < params.tpg.maxOutgoingEdges
			&& proba > Mutator::RNG::getDouble(0.0, 1.0)) {
			// Add an edge (by duplication of an existing one)
			addRandomEdge(graph, team, preExistingEdges);

			// Decrement the proba of adding another edge
			proba *= params.tpg.pEdgeAddition;
		}
	}

	// 3. Mutate program of the team
	{
		bool anyMutationDone = false;
		do {
			// Process edge-by-edge
			// And possibly modify their behavior and their target
			for (TPG::TPGEdge* edge : team.getOutgoingEdges()) {
				// Edge->Program bid modification
				if (Mutator::RNG::getDouble(0.0, 1.0) < params.tpg.pProgramMutation) {
					// Mutate the edge
					mutateOutgoingEdge(graph, archive, team, edge, preExistingTeams, preExistingActions, params);
					anyMutationDone = true;
				}
			}
		} while (!anyMutationDone);
	}
}


void Mutator::TPGMutator::populateTPG(TPG::TPGGraph& graph, const Archive& archive, const Mutator::MutationParameters& params)
{
	// Get current vertex set (copy)
	auto vertices(graph.getVertices());
	// Get current root teams (copy)
	auto rootVertices(graph.getRootVertices());
	// Get root Teams
	std::vector<const TPG::TPGTeam*> rootTeams;
	std::for_each(rootVertices.begin(), rootVertices.end(),
		[&rootTeams](const TPG::TPGVertex* vertex) {
			if (typeid(*vertex) == typeid(TPG::TPGTeam)) {
				rootTeams.push_back((const TPG::TPGTeam*)vertex);
			}
		});

	// If the graph doesn't contain any root teams, call the init procedure.
	// (note that execution of this code is not a very good sign.. maybe an 
	// exception would be more appropriate?)
	if (rootTeams.size() == 0) {
		initRandomTPG(graph, params);
		vertices = graph.getVertices();
		rootVertices = graph.getRootVertices();
		rootTeams.clear();
		std::for_each(rootVertices.begin(), rootVertices.end(),
			[&rootTeams](const TPG::TPGVertex* vertex) {
				rootTeams.push_back((const TPG::TPGTeam*)vertex);
			});
	}

	// Pre compute liste of available TPGTeam and TPGActions
	std::vector<const TPG::TPGTeam*> preExistingTeams;
	std::vector<const TPG::TPGAction*> preExistingActions;

	std::for_each(vertices.begin(), vertices.end(),
		[&preExistingActions, &preExistingTeams](const TPG::TPGVertex* target) {
			if (typeid(*target) == typeid(TPG::TPGAction)) {
				preExistingActions.push_back((const TPG::TPGAction*)target);
			}
			else {
				preExistingTeams.push_back((const TPG::TPGTeam*)target);
			}
		});

	// Get a list of pre existing edges before mutations (copy)
	std::list<const TPG::TPGEdge*> preExistingEdges;
	std::for_each(graph.getEdges().begin(), graph.getEdges().end(),
		[&preExistingEdges](const TPG::TPGEdge& edge) {
			preExistingEdges.push_back(&edge);
		});

	// While the target is not reached, add new teams
	uint64_t currentNumberOfRoot = rootVertices.size();
	while (params.tpg.nbRoots > currentNumberOfRoot) {
		// Select a random existing root
		uint64_t clonedRootIndex = RNG::getUnsignedInt64(0, rootTeams.size() - 1);
		// clone it (the vertex and all its outgoing edges)
		const TPG::TPGTeam& newRoot = (const TPG::TPGTeam&)graph.cloneVertex(*rootTeams.at(clonedRootIndex));
		// Apply mutations to the root
		mutateTPGTeam(graph, archive, newRoot, preExistingTeams, preExistingActions, preExistingEdges, params);
		// Check the new number of roots
		// Needed since preExisting root may be subsumed by new ones.
		currentNumberOfRoot = graph.getNbRootVertices();
	}
}
