/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
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

#include <inttypes.h>

#include "file/tpgGraphDotExporter.h"

uint64_t File::TPGGraphDotExporter::findVertexID(const TPG::TPGVertex& vertex)
{
    auto iter = this->vertexID.find(&vertex);
    if (iter == this->vertexID.end()) {
        // The vertex is not known yet
        this->vertexID.insert(std::pair<const TPG::TPGVertex*, uint64_t>(
            &vertex, this->nbVertex));
        this->nbVertex++;
        return nbVertex - 1;
    }
    else {
        return iter->second;
    }
}

bool File::TPGGraphDotExporter::findProgramID(const Program::Program& prog,
                                              uint64_t& id)
{
    auto iter = this->programID.find(&prog);
    if (iter == this->programID.end()) {
        // The vertex is not known yet
        this->programID.insert(std::pair<const Program::Program*, uint64_t>(
            &prog, this->nbPrograms));
        this->nbPrograms++;
        id = this->nbPrograms - 1;
        return true;
    }
    else {
        id = iter->second;
        return false;
    }
}

void File::TPGGraphDotExporter::printTPGTeam(const TPG::TPGTeam& team)
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

    fprintf(pFile, "%sT%" PRIu64 " [fillcolor=\"%s\"]\n", this->offset.c_str(),
            name, color.c_str());
}

uint64_t File::TPGGraphDotExporter::printTPGAction(const TPG::TPGAction& action)
{
    fprintf(pFile,
            "%sA%" PRIu64 " [fillcolor=\"#ff3366\" shape=box margin=0.03 "
            "width=0 height=0 label=\"%" PRIu64 "\"]\n",
            this->offset.c_str(), nbActions++, action.getActionID());
    return nbActions - 1;
}

void File::TPGGraphDotExporter::printTPGEdge(const TPG::TPGEdge& edge)
{
    uint64_t srcID = this->findVertexID(*edge.getSource());
    uint64_t progID;

    Program::Program& p = edge.getProgram();
    if (this->findProgramID(edge.getProgram(), progID)) {
        // First time thie Program is encountered
        fprintf(pFile, "%sP%" PRIu64 " [fillcolor=\"#cccccc\" shape=point]\n",
                this->offset.c_str(), progID);
        // print the program content :
        printProgram(p);
        fprintf(pFile, "%sP%" PRIu64 " -> I%" PRIu64 "[style=invis]\n",
                this->offset.c_str(), progID, progID);
        auto* dest = edge.getDestination();
        if (dest && typeid(*dest) == typeid(TPG::TPGAction)) {
            uint64_t actionID =
                printTPGAction(*(const TPG::TPGAction*)edge.getDestination());
            fprintf(pFile, "%sT%" PRIu64 " -> P%" PRIu64 " -> A%" PRIu64 "\n",
                    this->offset.c_str(), srcID, progID, actionID);
        }
        else {
            uint64_t destID = findVertexID(*edge.getDestination());
            fprintf(pFile, "%sT%" PRIu64 " -> P%" PRIu64 " -> T%" PRIu64 "\n",
                    this->offset.c_str(), srcID, progID, destID);
        }
    }
    else {
        fprintf(pFile, "%sT%" PRIu64 " -> P%" PRIu64 "\n", this->offset.c_str(),
                srcID, progID);
    }
}

void File::TPGGraphDotExporter::printProgram(const Program::Program& program)
{
    uint64_t progID;
    this->findProgramID(program, progID);
    std::string programContent = "";
    for (int i = 0; i < program.getNbLines(); i++) {
        const Program::Line& l = program.getLine(i);
        // instruction index
        programContent += std::to_string(l.getInstructionIndex());
        programContent += "|";
        // instruction destination index
        programContent += std::to_string(l.getDestinationIndex());
        programContent += "&";
        // instruction parameters
        for (int j = 0; j < l.getEnvironment().getMaxNbParameters(); j++) {
            const Parameter& p = l.getParameter(j);
            programContent += std::to_string(p.i);
            programContent += "|";
        }
        programContent += "$";
        // instruction operands
        for (int j = 0; j < l.getEnvironment().getMaxNbOperands(); j++) {
            std::pair<uint64_t, uint64_t> p = l.getOperand(j);
            if (j != 0)
                programContent += "#";
            programContent += std::to_string(p.first);
            programContent += "|";
            programContent += std::to_string(p.second);
        }

        programContent += "&#92;n";
    }
    fprintf(pFile, "%sI%" PRIu64 " [shape=box style=invis label=\"%s\"]\n",
            this->offset.c_str(), progID, programContent.c_str());
}

void File::TPGGraphDotExporter::printTPGGraphHeader()
{
    /*
    graph{
    graph[pad = "0.212,0.055" bgcolor = lightgray]
    node[style = filled label = ""]
    */
    fprintf(pFile, "%sdigraph{\n", this->offset.c_str());
    this->offset = "\t";
    fprintf(pFile, "%sgraph[pad = \"0.212, 0.055\" bgcolor = lightgray]\n",
            this->offset.c_str());
    fprintf(pFile, "%snode[shape=circle style = filled label = \"\"]\n",
            this->offset.c_str());
    this->offset = "\t\t";
}

void File::TPGGraphDotExporter::printTPGGraphFooter()
{
    // Print root actions (and keep the ids)
    auto rootVertices = tpg.getRootVertices();
    std::vector<uint64_t> rootActionIDs;
    for (const TPG::TPGVertex* rootVertex : rootVertices) {
        if (typeid(*rootVertex) == typeid(TPG::TPGAction)) {
            rootActionIDs.push_back(
                this->printTPGAction(*(const TPG::TPGAction*)rootVertex));
        }
    }

    // Rank all the roots
    fprintf(pFile, "%s{ rank= same ", this->offset.c_str());
    // Team root ids
    for (const TPG::TPGVertex* rootVertex : rootVertices) {
        if (typeid(*rootVertex) == typeid(TPG::TPGTeam)) {
            fprintf(pFile, "T%" PRIu64 " ", this->findVertexID(*rootVertex));
        }
    }
    // Action root
    for (auto rootActionId : rootActionIDs) {
        fprintf(pFile, "A%" PRIu64 " ", rootActionId);
    }
    fprintf(pFile, "}\n");
    this->offset = "";
    fprintf(pFile, "%s}\n", this->offset.c_str());
}

void File::TPGGraphDotExporter::print()
{
    // Print the graph header
    this->printTPGGraphHeader();

    // Print all teams
    auto vertices = this->tpg.getVertices();
    for (const TPG::TPGVertex* vertex : vertices) {
        if (typeid(*vertex) == typeid(TPG::TPGTeam)) {
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
