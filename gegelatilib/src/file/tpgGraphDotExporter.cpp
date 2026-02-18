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
#include "data/demangle.h"

void File::GraphDotExporter::printAgent(const Algorithm::Agent& agentProgram){

    // Find corresponding algorithm to the agent and print it
    for(const Algorithm::Algorithm& algorithm : this->potentialAlgorithms){
        if(algorithm.containsAgent(agentProgram)){
            std::vector<std::reference_wrapper<const EvoGraph::Element>> elements;
            algorithm.printAgent(agentProgram, this->pFile, this->offset, this->printedAgentID, elements);

            // Print the elements collected during the printAgent call
            for(auto element : elements){
                this->printElement(element);
            }
            return;
        }
    }
    throw std::runtime_error("File::GraphDotExporter::printAgent unknown agent program");
}

void File::GraphDotExporter::printElement(const EvoGraph::Element& element)
{
    // Check if the element has already been printed. If it is the case, do nothing, else print it.
    if(dynamic_cast<const EvoGraph::Vertex*>(&element) != nullptr){
        this->printVertex(*dynamic_cast<const EvoGraph::Vertex*>(&element));
    }
    else if(dynamic_cast<const EvoGraph::Edge*>(&element) != nullptr){
        this->printEdge(*dynamic_cast<const EvoGraph::Edge*>(&element));
    }
    else {
        throw std::runtime_error("File::GraphDotExporter::printElement unknown element type");
    }
}


void File::GraphDotExporter::printVertex(const EvoGraph::Vertex& vertex)
{
    if(this->printedVertexID.find(vertex.getVertexID()) == this->printedVertexID.end()){
        this->printedVertexID.insert(vertex.getVertexID());
        
        if (dynamic_cast<const EvoGraph::Team*>(&vertex) != nullptr) {
            printTeam(*dynamic_cast<const EvoGraph::Team*>(&vertex));
        }
        else if (dynamic_cast<const EvoGraph::Action*>(&vertex) != nullptr) {
            printAction(*dynamic_cast<const EvoGraph::Action*>(&vertex));
        }
        else {
            throw std::runtime_error("File::GraphDotExporter::printVertex unknown vertex type");
        }




        // Print the link between the vertex and its program if it has one
        if(vertex.hasProgram()){

            // Print the potential agent program associated to the vertex
            this->printAgent(vertex.getProgram());

            std::string srcLetter = (dynamic_cast<const EvoGraph::Team*>(&vertex) != nullptr) ? "T" : "A";
            fprintf(pFile, "%s%s%" PRIu64 " -> P%" PRIu64 " [style=dashed]\n",
                    this->offset.c_str(), srcLetter.c_str(), vertex.getVertexID(), vertex.getProgram().getAgentID());
        }

        // Print outgoing edges
        for(auto edge : vertex.getOutgoingEdges()){
            this->printEdge(*edge);
        }
    }

}

void File::GraphDotExporter::printTeam(const EvoGraph::Team& team)
{
    // Color is different for roots
    std::string color;
    if (team.getIncomingEdges().size() == 0) {
        color = "#2b7c91";
    }
    else {
        color = "#66ddff";
    }

    fprintf(pFile, "%sT%" PRIu64 " [fillcolor=\"%s\"]\n", this->offset.c_str(),
            team.getVertexID(), color.c_str());
}
void File::GraphDotExporter::printAction(const EvoGraph::Action& action)
{
    fprintf(pFile,
            "%sA%" PRIu64 " [fillcolor=\"#ff3366\" shape=box margin=0.03 "
            "width=0 height=0 label=\"%" PRIu64 "\"]\n",
            this->offset.c_str(), action.getVertexID(), action.getActionID());
}

void File::GraphDotExporter::printEdge(const EvoGraph::Edge& edge)
{
    // Check if the edge has already been printed. If it is the case, do nothing, else print it.
    if(this->printedEdgeID.find(edge.getEdgeID()) == this->printedEdgeID.end()){
        this->printedEdgeID.insert(edge.getEdgeID());


        // Print destination
        this->printVertex(*edge.getDestination());


        uint64_t srcID = edge.getSource()->getVertexID();
        uint64_t destID = edge.getDestination()->getVertexID();
        std::string srcLetter = (dynamic_cast<const EvoGraph::Team*>(edge.getSource().get()) != nullptr) ? "T" : "A";
        std::string destLetter = (dynamic_cast<const EvoGraph::Team*>(edge.getDestination().get()) != nullptr) ? "T" : "A";

        if(edge.hasProgram()){
            // Print the potential agent program associated to the edge
            this->printAgent(edge.getProgram());

            fprintf(pFile, "%s%s%" PRIu64 " -> P%" PRIu64 " -> %s%" PRIu64 "\n",
                    this->offset.c_str(), srcLetter.c_str(), srcID, edge.getProgram().getAgentID(), destLetter.c_str(), destID);
        }
        else {
            fprintf(pFile, "%s%s%" PRIu64 " -> %s%" PRIu64 "\n",
                    this->offset.c_str(), srcLetter.c_str(), srcID, destLetter.c_str(), destID);
        }
    }
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
    auto rootActions = tpg.getRootActions();
    std::vector<uint64_t> rootActionIDs;
    for (const auto rootVertex : rootActions) {
        this->printVertex(*rootVertex);
    }

    // Rank all the agents of main algoritms
    fprintf(pFile, "%s{ rank= same ", this->offset.c_str());
    // Main agents ids
    for(auto algorithm : this->algorithmsRef){
        for(const Algorithm::Agent& agent : algorithm->getManagerCst()->getAgents()){
            fprintf(pFile, "P%" PRIu64 " ", agent.getAgentID());
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

    this->printedVertexID.clear();
    this->printedEdgeID.clear();
    this->printedAgentID.clear();

    // Print each agent algorithms
    // If agent uses some vertices or edges, it will print them
    // Then if vertices and/or edges uses program agents it will print them, and so on...
    for(auto algorithm : this->algorithmsRef){
        for(const Algorithm::Agent& agent : algorithm->getManagerCst()->getAgents()){
            this->printAgent(agent);
        }
    }


    // Print footer
    this->printGraphFooter();

    // flush file
    fflush(pFile);
}

void File::GraphDotExporter::printSubGraph(const Algorithm::Agent& agent)
{
    // Print the graph header
    this->printGraphHeader();

    this->printedVertexID.clear();
    this->printedEdgeID.clear();
    this->printedAgentID.clear();

    // Print the agent given as parameter, its vertices and edges, and the potential agent programs associated to these vertices and edges.
    this->printAgent(agent);

    // Print specific footer (no need for rank, since there is a single root)
    this->offset = "";
    fprintf(pFile, "%s}\n", this->offset.c_str());

    // flush file
    fflush(pFile);
}