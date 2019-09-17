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

void Exporter::TPGGraphDotExporter::printTPGVertex(const TPG::TPGVertex& vertex)
{
}

void Exporter::TPGGraphDotExporter::printTPGTeam(const TPG::TPGTeam& team)
{
	uint64_t name = this->findVertexID(team);
	// Color is different for roots
	std::string color;
	if (team.getIncomingEdges().size() == 0) {
		color = "#66ddff";
	}
	else {
		color = "#33bbdd";
	}

	fprintf(pFile, "%s%lld [fillcolor=\"%s\"]\n", this->offset.c_str(), name, color.c_str());
}

void Exporter::TPGGraphDotExporter::printTPGAction(const TPG::TPGAction& action)
{
	uint64_t name = this->findVertexID(action);
	fprintf(pFile, "%s%lld [fillcolor=\"#ff3366\" label = \"%lld\"]\n", this->offset.c_str(), name, action.getActionID());
}

void Exporter::TPGGraphDotExporter::printTPGEdge(const TPG::TPGEdge& edge)
{
	uint64_t srcID = this->findVertexID(*edge.getSource());
	uint64_t destID = this->findVertexID(*edge.getDestination());
	fprintf(pFile, "%s%lld -> %lld\n", this->offset.c_str(), srcID, destID);
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
	// Rank all the roots
	fprintf(pFile, "%s{ rank= same ", this->offset.c_str());
	auto rootVertices = tpg.getRootVertices();
	for (const TPG::TPGVertex* rootVertex : rootVertices) {
		fprintf(pFile, "%lld ", this->findVertexID(*rootVertex));
	}
	fprintf(pFile, "}\n");
	this->offset = "";
	fprintf(pFile, "%s}\n", this->offset.c_str());
}

void Exporter::TPGGraphDotExporter::print()
{
	// Print the graph header
	this->printTPGGraphHeader();

	// Print all vertices
	auto vertices = this->tpg.getVertices();
	for (const TPG::TPGVertex* vertex : vertices) {
		if (typeid (*vertex) == typeid(TPG::TPGAction)) {
			this->printTPGAction(*(const TPG::TPGAction*)vertex);
		}
		else if (typeid (*vertex) == typeid(TPG::TPGTeam)) {
			this->printTPGTeam(*(const TPG::TPGTeam*)vertex);
		}
		else {
			this->printTPGVertex(*vertex);
		}
	}

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
