/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2022) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
 * Thomas Bourgoin <tbourgoi@insa-rennes.fr> (2021)
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

#include "data/constant.h"
#include "file/tpgGraphDotExporter.h"
#include "util/timestamp.h"

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
            "width=0 height=0 label=\"%" PRIu64 "-" PRIu64 "\"]\n",
            this->offset.c_str(), nbActions++, action.getActionID(), action.getActionValue());
    return nbActions - 1;
}

void File::TPGGraphDotExporter::printTPGEdge(const TPG::TPGEdge& edge)
{
    uint64_t srcID = this->findVertexID(*edge.getSource());
    uint64_t progID;

    Program::Program& p = edge.getProgram();
    if (this->findProgramID(edge.getProgram(), progID)) {
        // First time thie Program is encountered
        fprintf(pFile, "%sP%" PRIu64 " [fillcolor=\"#cccccc\" shape=point] //",
                this->offset.c_str(), progID);
        // add next the content of the constant data handler in a comment (//)
        for (int i = 0; i < p.getEnvironment().getNbConstant(); i++) {
            fprintf(pFile, "%d|", static_cast<int>(p.getConstantAt(i)));
        }
        fprintf(pFile, "\n");
        // print the program content :
        printProgram(p);
        fprintf(pFile, "%sP%" PRIu64 " -> I%" PRIu64 "[style=invis]\n",
                this->offset.c_str(), progID, progID);
        auto* dest = edge.getDestination();
        if (dest && dynamic_cast<const TPG::TPGAction*>(dest) != nullptr) {
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
    // File exported with GEGELATI vX.Y.Z
    // On the YYYY-MM-DD HH:MM:SS
    // With the <Printer>
    graph{
    graph[pad = "0.212,0.055" bgcolor = lightgray]
    node[style = filled label = ""]
    */
    fprintf(pFile, "// File exported with GEGELATI v" GEGELATI_VERSION "\n");
    fprintf(pFile, "// On the %s\n", Util::getCurrentDate().c_str());
    fprintf(pFile, "// With the %s\n",
            DEMANGLE_TYPEID_NAME(typeid(*this).name()));
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
        if (dynamic_cast<const TPG::TPGAction*>(rootVertex) != nullptr) {
            rootActionIDs.push_back(
                this->printTPGAction(*(const TPG::TPGAction*)rootVertex));
        }
    }

    // Rank all the roots
    fprintf(pFile, "%s{ rank= same ", this->offset.c_str());
    // Team root ids
    for (const TPG::TPGVertex* rootVertex : rootVertices) {
        if (dynamic_cast<const TPG::TPGTeam*>(rootVertex) != nullptr) {
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
        if (dynamic_cast<const TPG::TPGTeam*>(vertex) != nullptr) {
            this->printTPGTeam(*(const TPG::TPGTeam*)vertex);
        }
    }

    // Reset program ids
    // This is done to ensure that a program without an ID is properly printed
    // when first encountered. However, this ruins the original purpose of the
    // ID, which should remain constant through multiple exports and
    // generations.
    this->programID.erase(this->programID.begin(), this->programID.end());

    // Print all edges
    auto& edges = this->tpg.getEdges();
    for (const std::unique_ptr<TPG::TPGEdge>& edge : edges) {
        this->printTPGEdge(*edge.get());
    }

    // Print footer
    this->printTPGGraphFooter();

    // flush file
    fflush(pFile);
}

void File::TPGGraphDotExporter::printSubGraph(const TPG::TPGVertex* root)
{
    // Print the graph header
    this->printTPGGraphHeader();

    // Reset program ids
    // This is done to ensure that a program without an ID is properly printed
    // when first encountered. However, this ruins the original purpose of the
    // ID, which should remain constant through multiple exports and
    // generations.
    this->programID.erase(this->programID.begin(), this->programID.end());

    // Print edges stemming from the given root
    // Init a Breadth First scan
    std::deque<const TPG::TPGVertex*> verticesToVisit;
    verticesToVisit.push_back(root);
    std::vector<const TPG::TPGVertex*> visitedVertices;
    std::vector<const TPG::TPGEdge*> edgesToPrint;

    while (!verticesToVisit.empty()) {
        // Get first vertex
        const TPG::TPGVertex* vertex = verticesToVisit.front();
        verticesToVisit.pop_front();
        visitedVertices.push_back(vertex);

        // Print it if it is a team (actions are printed with edges)
        const TPG::TPGTeam* team = nullptr;
        if ((team = dynamic_cast<const TPG::TPGTeam*>(vertex)) != nullptr) {
            this->printTPGTeam(*(const TPG::TPGTeam*)vertex);

            // Put its outgoing edge in the list for later print.
            // Edges must be printed after their destination team has been
            // written.
            for (auto edge : team->getOutgoingEdges()) {
                edgesToPrint.push_back(edge);

                // If the edge destination is a Team, put it in the list of
                // vertex to be visited.
                const TPG::TPGVertex* dest = edge->getDestination();
                if (dynamic_cast<const TPG::TPGTeam*>(dest) != nullptr &&
                    std::find(visitedVertices.begin(), visitedVertices.end(),
                              dest) == visitedVertices.end() &&
                    std::find(verticesToVisit.begin(), verticesToVisit.end(),
                              dest) == verticesToVisit.end()) {
                    verticesToVisit.push_back(dest);
                }
            }
        }
    }

    // Print edges
    for (const TPG::TPGEdge* edge : edgesToPrint) {
        this->printTPGEdge(*edge);
    }

    // Print specific footer (no need for rank, since there is a single root)
    this->offset = "";
    fprintf(pFile, "%s}\n", this->offset.c_str());

    // flush file
    fflush(pFile);
}
