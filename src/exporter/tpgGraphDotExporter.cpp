#include "exporter/tpgGraphDotExporter.h"

uint64_t Exporter::TPGGraphDotExporter::findVertexID(const TPG::TPGVertex& vertex)
{
	static uint64_t nbVertex = 0;
	auto iter = this->vertexID.find(&vertex);
	if (iter == this->vertexID.end()) {
		// The vertex is not known yet
		this->vertexID.insert(std::pair<const TPG::TPGVertex*, uint64_t>(&vertex, nbVertex));
		nbVertex++;
		return nbVertex - 1;
	}
	else {
		return iter->second;
	}
}

bool Exporter::TPGGraphDotExporter::findProgramID(const Program::Program& prog, uint64_t& id)
{
	static uint64_t nbPrograms = 0;
	auto iter = this->programID.find(&prog);
	if (iter == this->programID.end()) {
		// The vertex is not known yet
		this->programID.insert(std::pair<const Program::Program*, uint64_t>(&prog, nbPrograms));
		nbPrograms++;
		id = nbPrograms - 1;
		return true;
	}
	else {
		id = iter->second;
		return false;
	}
}

void Exporter::TPGGraphDotExporter::printTPGVertex(const TPG::TPGVertex& vertex)
{
}

void Exporter::TPGGraphDotExporter::printTPGTeam(const TPG::TPGTeam& team)
{
	uint64_t name = this->findVertexID(team);
	// Color is different for roots
	std::string color;
	if (team.getIncomingEdges().size() == 0) {
		color = "#1199bb";
	}
	else {
		color = "#66ddff";
	}

	fprintf(pFile, "%sT%lld [fillcolor=\"%s\"]\n", this->offset.c_str(), name, color.c_str());
}

uint64_t Exporter::TPGGraphDotExporter::printTPGAction(const TPG::TPGAction& action)
{
	fprintf(pFile, "%sA%lld [fillcolor=\"#ff3366\" shape=box margin=0.03 width=0 height=0 label = \"%lld\"]\n", this->offset.c_str(), nbActions++, action.getActionID());
	return nbActions - 1;
}

void Exporter::TPGGraphDotExporter::printTPGEdge(const TPG::TPGEdge& edge)
{
	uint64_t srcID = this->findVertexID(*edge.getSource());

	uint64_t progID;
	if (this->findProgramID(edge.getProgram(), progID)) {
		// First time thie Program is encountered
		fprintf(pFile, "%sP%lld [fillcolor=\"#cccccc\" shape=point]\n", this->offset.c_str(), progID);

		if (typeid(*edge.getDestination()) == typeid(TPG::TPGAction)) {
			uint64_t actionID = printTPGAction(*(const TPG::TPGAction*)edge.getDestination());
			fprintf(pFile, "%sP%lld -> A%lld\n", this->offset.c_str(), progID, actionID);
		}
		else {
			uint64_t destID = findVertexID(*edge.getDestination());
			fprintf(pFile, "%sP%lld -> T%lld\n", this->offset.c_str(), progID, destID);
		}
	}

	fprintf(pFile, "%sT%lld -> P%lld\n", this->offset.c_str(), srcID, progID);
}

void Exporter::TPGGraphDotExporter::printTPGGraphHeader()
{
	/*
	graph{
	graph[pad = "0.212,0.055" bgcolor = lightgray]
	node[style = filled label = ""]
	*/
	fprintf(pFile, "%sdigraph{\n", this->offset.c_str());
	this->offset = "\t";
	fprintf(pFile, "%sgraph[pad = \"0.212, 0.055\" bgcolor = lightgray]\n", this->offset.c_str());
	fprintf(pFile, "%snode[shape=circle style = filled label = \"\"]\n", this->offset.c_str());
	this->offset = "\t\t";
}

void Exporter::TPGGraphDotExporter::printTPGGraphFooter()
{
	// Print root actions (and keep the ids)
	auto rootVertices = tpg.getRootVertices();
	std::vector<uint64_t> rootActionIDs;
	for (const TPG::TPGVertex* rootVertex : rootVertices) {
		if (typeid(*rootVertex) == typeid(TPG::TPGAction)) {
			rootActionIDs.push_back(this->printTPGAction(*(const TPG::TPGAction*)rootVertex));
		}
	}

	// Rank all the roots
	fprintf(pFile, "%s{ rank= same ", this->offset.c_str());
	// Team root ids
	for (const TPG::TPGVertex* rootVertex : rootVertices) {
		if (typeid(*rootVertex) == typeid(TPG::TPGTeam)) {
			fprintf(pFile, "T%lld ", this->findVertexID(*rootVertex));
		}
	}
	// Action root
	for (auto rootActionId : rootActionIDs) {
		fprintf(pFile, "A%lld ", rootActionId);
	}
	fprintf(pFile, "}\n");
	this->offset = "";
	fprintf(pFile, "%s}\n", this->offset.c_str());
}

void Exporter::TPGGraphDotExporter::print()
{
	// Print the graph header
	this->printTPGGraphHeader();

	// Print all teams
	auto vertices = this->tpg.getVertices();
	for (const TPG::TPGVertex* vertex : vertices) {
		if (typeid (*vertex) == typeid(TPG::TPGTeam)) {
			this->printTPGTeam(*(const TPG::TPGTeam*)vertex);
		}
	}

	// Reset program ids
	this->programID.erase(this->programID.begin(), this->programID.end());

	// Print all edges
	auto edges = this->tpg.getEdges();
	for (const TPG::TPGEdge& edge : edges) {
		this->printTPGEdge(edge);
	}

	// Print footer
	this->printTPGGraphFooter();

	// flush file
	fflush(pFile);
}
