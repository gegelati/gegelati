/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
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

void File::GraphDotExporter::printTeam(const EvoGraph::Team& team)
{
    // Color is different for roots
    std::string color;
    if (team.getIncomingEdges().size() == 0) {
        color = "#1199bb";
    }
    else {
        color = "#66ddff";
    }

    fprintf(pFile, "%sT%" PRIu64 " [fillcolor=\"%s\"]\n", this->offset.c_str(),
            team.getVertexID(), color.c_str());
}
uint64_t File::GraphDotExporter::printAction(const EvoGraph::Action& action)
{

    uint64_t actionNumber = action.getVertexID();

    uint64_t actionID = action.getActionID();
    if (action.getOutgoingEdges().size() != 0) {
        actionID = actionNumber;
    }

    // Create a string stream to build the label
    std::ostringstream labelStream;

    auto outgoingEdges = action.getOutgoingEdges();
    if (action.getOutgoingEdges().size() != 0) {
        for (auto it = outgoingEdges.begin(); it != outgoingEdges.end(); ++it) {
            if (it != outgoingEdges.begin()) {
                labelStream << "-"; // Add separator between actionClasses
            }
            //auto actionClass =
            //    std::dynamic_pointer_cast<const EvoGraph::ActionEdge>(*it)->getActionClass();
            //labelStream << actionClass;
        }
    }
    else {
        labelStream << actionID;
    }

    // Get the complete label as a string
    std::string label = labelStream.str();

    // Write the label into the file
    fprintf(pFile,
            "%sA%" PRIu64 " [fillcolor=\"#ff3366\" shape=box margin=0.03 "
            "width=0 height=0 label=\"%s\"]\n",
            offset.c_str(), actionNumber, label.c_str());

    return actionNumber;
}

void File::GraphDotExporter::printEdge(const EvoGraph::Edge& edge)
{

    uint64_t srcID = edge.getSource()->getVertexID();

    std::shared_ptr<const Algorithm::Agent> agent;// = edge.getProgram();
    auto lgpAgent = std::dynamic_pointer_cast<const Algorithm::LGP::LGPAgent>(agent);
    if(lgpAgent == nullptr){
        throw std::runtime_error("File::GraphDotExporter::printEdge agentProgram is not an lgpAgent");
    }

    uint64_t progID = lgpAgent->getAgentID();
    if (this->programIDIsNew(progID)) {

        // First time thie Program is encountered
        fprintf(pFile,
                "%sP%" PRIu64
                " [fillcolor=\"#cccccc\" shape=point label=\"%d\"] //",
                this->offset.c_str(), progID, 0);
        // add next the content of the constant data handler in a comment (//)
        for (int i = 0; i < lgpAgent->getEnvironment()->getParams().nbProgramConstant;
             i++) {
            fprintf(pFile, "%f|", static_cast<double>(lgpAgent->getConstantAt(i)));
        }
        fprintf(pFile, "\n");
        // print the program content :
        printLGPAgent(lgpAgent);
        fprintf(pFile, "%sP%" PRIu64 " -> I%" PRIu64 "[style=invis]\n",
                this->offset.c_str(), progID, progID);
        if (true){//(dynamic_cast<const EvoGraph::ActionEdge*>(&edge) != nullptr) {

            fprintf(pFile, "%sA%" PRIu64 " -> P%" PRIu64 "\n",
                    this->offset.c_str(), srcID, progID);
        }
        else {

            auto dest = edge.getDestination();

            if (dest && std::dynamic_pointer_cast<const EvoGraph::Action>(dest) != nullptr) {
                uint64_t actionID = printAction(
                    *std::dynamic_pointer_cast<const EvoGraph::Action>(dest) );
                fprintf(pFile,
                        "%sT%" PRIu64 " -> P%" PRIu64 " -> A%" PRIu64 "\n",
                        this->offset.c_str(), srcID, progID, actionID);
            }
            else {
                uint64_t destID = edge.getDestination()->getVertexID();
                fprintf(pFile,
                        "%sT%" PRIu64 " -> P%" PRIu64 " -> T%" PRIu64 "\n",
                        this->offset.c_str(), srcID, progID, destID);
            }
        }
    }
    else {

        if (true){//(dynamic_cast<const EvoGraph::ActionEdge*>(&edge) != nullptr) {
            fprintf(pFile, "%sA%" PRIu64 " -> P%" PRIu64 "\n",
                    this->offset.c_str(), srcID, progID);
        }
        else {
            fprintf(pFile, "%sT%" PRIu64 " -> P%" PRIu64 "\n",
                    this->offset.c_str(), srcID, progID);
        }
    }
}

void File::GraphDotExporter::printLGPAgent(std::shared_ptr<const Algorithm::LGP::LGPAgent> lgpAgent)
{
    std::string programContent = "";
    for (int i = 0; i < lgpAgent->getNbLines(); i++) {
        const Algorithm::LGP::LGPLine& l = lgpAgent->getLine(i);
        // instruction index
        programContent += std::to_string(l.getInstructionIndex());
        programContent += "|";
        // instruction destination index
        programContent += std::to_string(l.getDestinationIndex());
        programContent += "&";
        // instruction operands
        for (int j = 0; j < l.getEnvironment()->getMaxNbOperands(); j++) {
            std::pair<uint64_t, uint64_t> p = l.getOperand(j);
            if (j != 0)
                programContent += "#";
            programContent += std::to_string(p.first);
            programContent += "|";
            programContent += std::to_string(p.second);
        }

        programContent += "&#92;n";
    }
    fprintf(pFile, "%sI%" PRIu64 " [shape=box style=invis label=\"%s\"] \n",
            this->offset.c_str(), lgpAgent->getAgentID(),
            programContent.c_str());
}

void File::GraphDotExporter::printGraphHeader()
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

void File::GraphDotExporter::printGraphFooter()
{

    // Print root actions (and keep the ids)
    auto rootVertices = tpg.getRootVertices();
    std::vector<uint64_t> rootActionIDs;
    for (const auto rootVertex : rootVertices) {
        if (auto actionRoot =  std::dynamic_pointer_cast<const EvoGraph::Action>(rootVertex)) {
            rootActionIDs.push_back(
                this->printAction(*actionRoot));
        }
    }

    // Print all action edges
    auto& edges = this->tpg.getEdges();
    for (const auto edge : edges) {
        //if (dynamic_cast<const EvoGraph::ActionEdge*>(edge.get()) != nullptr) {
        //    this->printEdge(*edge.get());
        //}
    }

    // Rank all the roots
    fprintf(pFile, "%s{ rank= same ", this->offset.c_str());
    // Team root ids
    for (const auto rootVertex : rootVertices) {
        if (dynamic_cast<const EvoGraph::Team*>(rootVertex.get()) != nullptr) {
            fprintf(pFile, "T%" PRIu64 " ", rootVertex->getVertexID());
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

void File::GraphDotExporter::print()
{

    // Print the graph header
    this->printGraphHeader();

    // Print all vertices
    auto vertices = this->tpg.getVertices();
    for (const auto vertex : vertices) {
        if (dynamic_cast<const EvoGraph::Team*>(vertex.get()) != nullptr) {
            this->printTeam(*(const EvoGraph::Team*)vertex.get());
        }
    }

    // Reset program ids
    // This is done to ensure that a program without an ID is properly printed
    // when first encountered. However, this ruins the original purpose of the
    // ID, which should remain constant through multiple exports and
    // generations.
    this->programID.erase(this->programID.begin(), this->programID.end());

    // Print all context edges
    auto& edges = this->tpg.getEdges();
    for (const auto edge : edges) {
        //if (dynamic_cast<const EvoGraph::ActionEdge*>(edge.get()) == nullptr) {
        //    this->printEdge(*edge.get());
        //}
    }

    // Print footer
    this->printGraphFooter();

    // flush file
    fflush(pFile);
}

void File::GraphDotExporter::printSubGraph(const EvoGraph::Vertex* root)
{
    // Print the graph header
    this->printGraphHeader();

    // Reset program ids
    // This is done to ensure that a program without an ID is properly printed
    // when first encountered. However, this ruins the original purpose of the
    // ID, which should remain constant through multiple exports and
    // generations.
    this->programID.erase(this->programID.begin(), this->programID.end());

    // Print edges stemming from the given root
    // Init a Breadth First scan
    std::deque<const EvoGraph::Vertex*> verticesToVisit;
    verticesToVisit.push_back(root);
    std::vector<const EvoGraph::Vertex*> visitedVertices;
    std::vector<const EvoGraph::Edge*> edgesToPrint;

    while (!verticesToVisit.empty()) {
        // Get first vertex
        const EvoGraph::Vertex* vertex = verticesToVisit.front();
        verticesToVisit.pop_front();
        visitedVertices.push_back(vertex);

        // Print it if it is a team (actions are printed with edges)
        if (dynamic_cast<const EvoGraph::Team*>(vertex) != nullptr) {
            this->printTeam(*(const EvoGraph::Team*)vertex);
        }
        else {
            this->printAction(*(const EvoGraph::Action*)vertex);
        }

        // Put its outgoing edge in the list for later print.
        // Edges must be printed after their destination team has been
        // written.
        for (auto edge : vertex->getOutgoingEdges()) {
            edgesToPrint.push_back(edge.get());

            // If the edge destination is a Team, put it in the list of
            // vertex to be visited.
            if (true){//(dynamic_cast<const EvoGraph::ActionEdge*>(edge.get()) == nullptr) {
                const EvoGraph::Vertex* dest = edge->getDestination().get();
                if (std::find(visitedVertices.begin(), visitedVertices.end(),
                              dest) == visitedVertices.end() &&
                    std::find(verticesToVisit.begin(), verticesToVisit.end(),
                              dest) == verticesToVisit.end()) {
                    verticesToVisit.push_back(dest);
                }
            }
        }
    }

    // Print edges
    for (const EvoGraph::Edge* edge : edgesToPrint) {
        this->printEdge(*edge);
    }

    // Print specific footer (no need for rank, since there is a single root)
    this->offset = "";
    fprintf(pFile, "%s}\n", this->offset.c_str());

    // flush file
    fflush(pFile);
}
