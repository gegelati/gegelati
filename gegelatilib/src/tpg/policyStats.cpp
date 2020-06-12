/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2020)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

#include <numeric>
#include <algorithm>

#include "tpg/policyStats.h"

void TPG::PolicyStats::clear() {
	this->maxPolicyDepth = 0;
	this->nbDistinctTeams = 0;
	this->nbTPGVertexPerDepthLevel.clear();
	this->nbLinesPerProgram.clear();
	this->nbIntronPerProgram.clear();
	this->nbOutgoingEdgesPerTeam.clear();
	this->nbUsagePerActionID.clear();
	this->nbUsagePerInstruction.clear();
	this->nbUsagePerDataLocation.clear();
	this->nbUsePerProgram.clear();
	this->nbUsePerTPGTeam.clear();
	this->nbUsePerTPGAction.clear();
}

void TPG::PolicyStats::setEnvironment(const Environment& env) {
	// Object needed to perform the analysis.. should be policyStats attributes
	this->environment = &env;
	this->dataSourcesAndRegisters.push_back(environment->getFakeRegisters());
	for (auto data : environment->getDataSources()) {
		dataSourcesAndRegisters.push_back(data.get());
	}
}

void TPG::PolicyStats::analyzeLine(const Program::Line* line) {
	// Count number of use of each instruction
	auto instructionIdx = line->getInstructionIndex();
	this->nbUsagePerInstruction[instructionIdx]++; // Create key if it does not exist yet.

	// Count number of access for each location
	const Instructions::Instruction& instruction = this->environment->getInstructionSet().getInstruction(instructionIdx);
	// Scan operands
	for (size_t operandIdx = 0; operandIdx < instruction.getNbOperands(); operandIdx++) {
		const std::pair<size_t, size_t>& rawOperand = line->getOperand(operandIdx);
		const std::type_info& operandType = instruction.getOperandTypes().at(operandIdx).get();
		const Data::DataHandler& dHandler = dataSourcesAndRegisters.at(rawOperand.first).get();
		size_t scaledLocation = dHandler.scaleLocation(rawOperand.second, operandType); // scaling.. should not be duplicate code.
		// Get list of accessed addresses
		std::vector<size_t> accessedLocations = dHandler.getAddressesAccessed(operandType, scaledLocation);
		// Fill attribute
		for (size_t accessedLocation : accessedLocations) {
			this->nbUsagePerDataLocation[{rawOperand.first, accessedLocation}]++; // create key if it does not exists.
		}
	}
}

void TPG::PolicyStats::analyzeProgram(const Program::Program* prog)
{
	// Check if the Program was already analyzed
	auto programIterator = this->nbUsePerProgram.find(prog);
	if (programIterator != this->nbUsePerProgram.end()) {
		// Increment the number of use of this Program.
		programIterator->second++;
		return;
	}

	// Else, this is a new program: analyze it
	this->nbUsePerProgram.emplace(prog, 1);

	// Count the number of lines
	this->nbLinesPerProgram.push_back(prog->getNbLines());

	// Count the number of intron lines
	size_t nbIntronLines = 0;
	for (auto lineIdx = 0; lineIdx < prog->getNbLines(); lineIdx++) {
		if (!prog->isIntron(lineIdx)) {
			const Program::Line& line = prog->getLine(lineIdx);
			this->analyzeLine(&line);
		}
		else {
			nbIntronLines++;
		}
	}
	this->nbIntronPerProgram.push_back(nbIntronLines);
}

void TPG::PolicyStats::analyzeTPGTeam(const TPG::TPGTeam* team)
{
	// Number of use per team
	size_t nbUse = ++this->nbUsePerTPGTeam[team];

	// Only update nbOutgoingEdgesPerTeam once per team.
	if (nbUse == 1) {
		this->nbDistinctTeams++;
		this->nbOutgoingEdgesPerTeam.push_back(team->getOutgoingEdges().size());
	}
}

void TPG::PolicyStats::analyzeTPGAction(const TPG::TPGAction* action)
{
	// Number of use per TPGAction
	this->nbUsePerTPGAction[action]++;

	// Nb use per action id
	this->nbUsagePerActionID[action->getActionID()]++;
}

void TPG::PolicyStats::analyzePolicy(const TPG::TPGVertex* root)
{
	size_t depth = 0;
	// Double bufferring to store vertices of the next stage.
	std::vector<const TPG::TPGVertex*> stage[2];

	// Do a breadth-first scan of the tree
	stage[0].push_back(root);
	while (stage[depth % 2].size() != 0) {
		// Fill nbTPGVertexPerDepthLevel attributes
		this->nbTPGVertexPerDepthLevel[depth] = stage[depth % 2].size();

		// Prepare next stage
		auto& nextStage = stage[(depth + 1) % 2];
		nextStage.clear();

		// scan current stage
		for (const TPG::TPGVertex* vertex : stage[depth % 2]) {
			if (typeid(*vertex) == typeid(TPG::TPGTeam)) {
				this->analyzeTPGTeam((const TPG::TPGTeam*)vertex);
				// Unless it was already analysed more than once,
				// add successors to the next stage and analyze their
				// Program.
				if (this->nbUsePerTPGTeam[(const TPG::TPGTeam*)vertex] == 1) {
					// Analyze outgoing edges
					for (const TPG::TPGEdge* edge : vertex->getOutgoingEdges()) {
						this->analyzeProgram(&edge->getProgram());
						nextStage.push_back(edge->getDestination());
					}
				}
			}

			if (typeid(*vertex) == typeid(TPG::TPGAction)) {
				this->analyzeTPGAction((const TPG::TPGAction*)vertex);
			}
		}
		depth++;
	}

	// Fill maxPolicyDepth
	this->maxPolicyDepth = depth - 1;
}

std::ostream& TPG::operator<<(std::ostream& os, const TPG::PolicyStats& policyStats)
{
	auto sumVec = [](const std::vector<size_t>& vec) {return std::accumulate(vec.cbegin(), vec.cend(), (size_t)0); };
	auto averageVec = [&sumVec](const std::vector<size_t>& vec) {return (double)sumVec(vec) / (double)vec.size(); };

	os << "# PolicyStats" << std::endl;
	os << "## Topology info" << std::endl;
	os << "Teams:\t\t" << policyStats.nbDistinctTeams << std::endl;
	os << "Edges:\t\t" << sumVec(policyStats.nbOutgoingEdgesPerTeam) << std::endl;
	os << "Actions:\t" << policyStats.nbUsagePerActionID.size() << std::endl;

	os << "Stages\t\t" << policyStats.maxPolicyDepth << std::endl;
	os << "Vertex/stage:\t";
	for (auto& nbVertexPerStage : policyStats.nbTPGVertexPerDepthLevel) {
		os << "{" << nbVertexPerStage.first << "," << nbVertexPerStage.second << "} ";
	}
	os << std::endl;

	os << "Use/action:\t" << (double)std::accumulate(policyStats.nbUsagePerActionID.begin(), policyStats.nbUsagePerActionID.end(), (size_t)0, [](size_t accu, std::pair<size_t, size_t> val) { return accu + val.second; }) / (double)policyStats.nbUsagePerActionID.size() << ": ";
	std::for_each(policyStats.nbUsagePerActionID.begin(), policyStats.nbUsagePerActionID.end(), [&os](auto& val) {os << "{" << val.first << "," << val.second << "} "; });
	os << std::endl;

	os << std::endl << "## Program info" << std::endl;
	os << "Programs:\t" << policyStats.nbUsePerProgram.size() << std::endl;
	os << "Line/prog:\t" << averageVec(policyStats.nbLinesPerProgram) << std::endl;
	os << "Intr/prog:\t" << averageVec(policyStats.nbIntronPerProgram) << std::endl;
	os << "Use/prog:\t" << (double)std::accumulate(policyStats.nbUsePerProgram.cbegin(), policyStats.nbUsePerProgram.cend(), size_t(0),
		[](size_t accu, const auto& val) {return accu + val.second; }) / (double)policyStats.nbUsePerProgram.size() << std::endl;

	os << "Use/instr:\t" << (double)std::accumulate(policyStats.nbUsagePerInstruction.cbegin(), policyStats.nbUsagePerInstruction.cend(), size_t(0), [](size_t accu, const std::pair<size_t, size_t>& val) {return accu + val.second; }) / (double)policyStats.nbUsagePerInstruction.size();
	os << ": ";
	std::for_each(policyStats.nbUsagePerInstruction.cbegin(), policyStats.nbUsagePerInstruction.cend(), [&os](const auto& val) {os << "{" << val.first << "," << val.second << "}"; });
	os << std::endl << std::endl;

	os << "## Data info";
	size_t currentDHandler = policyStats.nbUsagePerDataLocation.begin()->first.first - 1;

	std::for_each(policyStats.nbUsagePerDataLocation.cbegin(), policyStats.nbUsagePerDataLocation.cend(),
		[&os, &currentDHandler, &policyStats](const auto& entry) {
			if (entry.first.first != currentDHandler) {
				os << "\n\n### DataHandler " << entry.first.first << std::endl;
				currentDHandler = entry.first.first;
				size_t nbLocation = 0;
				auto nbAccess = std::accumulate(policyStats.nbUsagePerDataLocation.cbegin(), policyStats.nbUsagePerDataLocation.cend(), size_t(0), [&currentDHandler, &nbLocation](size_t accu, const auto& val) {
					if (val.first.first == currentDHandler) {
						nbLocation++;
						return accu + val.second;
					}
					else {
						return accu;
					}
					});
				os << "Accesses:\t" << nbAccess << std::endl;
				os << "Locations:\t" << nbLocation << std::endl;
			}
			os << "{" << entry.first.second << "," << entry.second << "} ";
		});

	return os;
}
