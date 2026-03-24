/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2020 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Agent Graphs (TPGs).
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
#include "file/graphDotImporter.h"

const std::string File::GraphDotImporter::algorithmRegex(
    "ALGO([0-9]+)\\x20\\x5B.*label=\"([^\\.]+)\\.([0-9]+)\".*");
const std::string File::GraphDotImporter::subAlgorithmLinkRegex(
    "ALGO([0-9]+)\\x20->\\x20ALGO([0-9]+)");
const std::string File::GraphDotImporter::endAlgorithmSubGraph(R"(^\s*\}\s*$)");


const std::string File::GraphDotImporter::teamRegex(
    "T([0-9]+)\\x20\\x5B.*fillcolor.*\\x5D");
const std::string File::GraphDotImporter::agentRegex(
    "P([0-9]+)\\x20\\x5B.*label=\"([^\\.]+)\\.([0-9]+)\".*");
const std::string File::GraphDotImporter::actionRegex(
    "A([0-9]+)\\x20\\x5B.*=\"(.*)\"\\x5D");
const std::string File::GraphDotImporter::linkTeamAgentActionRegex(
    "T([0-9]+)\\x20->\\x20P([0-9]+)\\x20->\\x20A([0-9]+).*");
const std::string File::GraphDotImporter::linkTeamAgentTeamRegex(
    "T([0-9]+)\\x20->\\x20P([0-9]+)\\x20->\\x20T([0-9]+).*");
const std::string File::GraphDotImporter::linkTeamAgentRegex(
    "T([0-9]+)\\x20->\\x20P([0-9]+)");
const std::string File::GraphDotImporter::linkAgentTeamRegex(
    "P([0-9]+)\\x20->\\x20T([0-9]+)");

Algorithm::Algorithm& File::GraphDotImporter::getAlgorithm(uint64_t algorithmID)
{
    auto it = this->mapAlgorithms.find(algorithmID);
    if(it == this->mapAlgorithms.end()){
        throw std::runtime_error("GraphDotImporter::getAlgorithm Algorithm not found"); 
    }
    return it->second;
}



bool File::GraphDotImporter::getExportedVersion(int& major, int& minor,
                                                   int& patch)
{
    char buffer[MAX_READ_SIZE];
    pFile.clear();
    pFile.seekg(0);
    if (!pFile.getline(buffer, MAX_READ_SIZE))
        return false;
    std::string line(buffer);
    std::regex versionRegex(
        R"(// File exported with GEGELATI v(\d+)\.(\d+)\.(\d+))");
    std::smatch matches;
    if (std::regex_search(line, matches, versionRegex)) {
        major = std::stoi(matches[1]);
        minor = std::stoi(matches[2]);
        patch = std::stoi(matches[3]);
        return true;
    }
    return false;
}

void File::GraphDotImporter::dumpGraphHeader()
{

    char buffer[MAX_READ_SIZE];

    // skips the comment lines of header (if any)
    do {
        pFile.getline(buffer, MAX_READ_SIZE);
    } while (buffer[0] == '/');

    // Skip the header (should be 3 lines, including one covered by previous
    // while loop)
    for (int i = 0; i < 2; i++) {
        pFile.getline(buffer, MAX_READ_SIZE);
    }
}

void File::GraphDotImporter::readAlgorithm(std::smatch matches)
{
    std::string name = matches[2];
    uint64_t id = std::stoi(matches[3]);
    
    auto it = this->mapAlgorithms.find(id);
    if(it == this->mapAlgorithms.end() || it->second.get().getAlgorithmName() != name){
        throw std::runtime_error("GraphDotImporter::readAlgorithm: Algorithm doesnt correspond"); 
    }
}

void File::GraphDotImporter::readSubAlgorithmLink(std::smatch matches)
{
    uint64_t id_src = std::stoi(matches[1]);
    uint64_t id_dest = std::stoi(matches[2]);

    auto it_src = this->mapAlgorithms.find(id_src);
    if(it_src == this->mapAlgorithms.end()){
        throw std::runtime_error("GraphDotImporter::readSubAlgorithmLink: source algorithm not found"); 
    }

    auto it_dest = this->mapAlgorithms.find(id_dest);
    if(it_dest == this->mapAlgorithms.end()){
        throw std::runtime_error("GraphDotImporter::readSubAlgorithmLink: destination algorithm not found"); 
    }

    // Will throw if nothing is found
    it_src->second.get().cGetSubAlgorithm(id_dest);
}

void File::GraphDotImporter::readAlgorithmGraphSubGraph()
{
    char buffer[MAX_READ_SIZE];
    // Skip subGraph header (should be 5 lines)
    for (int i = 0; i < 5; i++) {
        pFile.getline(buffer, MAX_READ_SIZE);
    }

    this->setMapAlgorithm();

    std::regex testAlgorithmRegex(this->algorithmRegex);
    std::regex testSubAlgorithmLinkRegex(this->subAlgorithmLinkRegex);
    std::regex testEndAlgorithmSubGraph(this->endAlgorithmSubGraph);


    std::smatch matches;
    
    bool read = true;
    while (read) {    
        if (!pFile.getline(buffer, MAX_READ_SIZE))
            throw std::ifstream::failure("Couldn't read in the given file");
        else {
            this->lastLine = buffer;
        }

        // check the line shape and parse it
        if (std::regex_search(this->lastLine, matches, testAlgorithmRegex)) {
            this->readAlgorithm(matches);
        } else if (std::regex_search(this->lastLine, matches, testSubAlgorithmLinkRegex)) {
            this->readSubAlgorithmLink(matches);
        } else if (std::regex_search(this->lastLine, matches, testEndAlgorithmSubGraph)) {
            read = false;
        } else {
        }
    }
}


void File::GraphDotImporter::setMapAlgorithm()
{
    this->mapAlgorithms.clear();
    // Add all algorithms to the set, and recursively all their sub-algorithms, to be able to print the content of the programs when they are mutated by the algorithm.
    std::vector<std::reference_wrapper<Algorithm::Algorithm>> algorithmsToAdd;
    algorithmsToAdd.push_back(algorithm);
    while(!algorithmsToAdd.empty()){
        Algorithm::Algorithm& algorithm = algorithmsToAdd.back();
        algorithmsToAdd.pop_back();

        if(this->mapAlgorithms.find(algorithm.getAlgorithmID()) == this->mapAlgorithms.end()){
            this->mapAlgorithms.insert({algorithm.getAlgorithmID(), algorithm});
            for(Algorithm::Algorithm& subAlgorithm : algorithm.getSubAlgorithms()){
                algorithmsToAdd.push_back(subAlgorithm);
            }
        }
    }
}

void File::GraphDotImporter::readTeam(std::smatch& matches)
{
    if (!this->lastLine.empty() && !matches.empty()) {
        const EvoGraph::Team& newTeam = this->graph.addNewTeam();

        this->readVertexID.insert({std::stoi(matches[1]), newTeam});

        this->graph.setNewVertexID(newTeam, std::stoi(matches[1]));
    }
}
void File::GraphDotImporter::readAction(std::smatch& matches)
{
    if (!this->lastLine.empty() && !matches.empty()) {
        const EvoGraph::Action& newAction = this->graph.addNewAction(std::stoi(matches[2]));

        this->readVertexID.insert({std::stoi(matches[1]), newAction});

        this->graph.setNewVertexID(newAction, std::stoi(matches[1]));
    }
}

void File::GraphDotImporter::readAgent(std::smatch& matches) {
    if (!this->lastLine.empty() && !matches.empty()) {
        uint64_t agentID = std::stoi(matches[1]);
        std::string algorithmName = matches[2];
        uint64_t algorithmID = std::stoi(matches[3]);
        
        Algorithm::Algorithm& algorithm = this->getAlgorithm(algorithmID);
        const Algorithm::Agent& agent = algorithm.readAgent(matches);

        this->readAgentID.insert({std::stoi(matches[1]), agent});

        algorithm.getManager().setNewAgentID(agent, agentID);
    }
}

void File::GraphDotImporter::readLinkTeamAgentAction(std::smatch& matches)
{
    // Creating a edge from a team to an action
    if (!this->lastLine.empty() && !matches.empty()) {
        uint64_t team_id = std::stoi(matches[1]);
        uint64_t agent_id = std::stoi(matches[2]);
        uint64_t action_id = std::stoi(matches[3]);

        // check object are found
        auto action_it = this->readVertexID.find(action_id);
        auto team_it = this->readVertexID.find(team_id);
        auto agent_it = this->readAgentID.find(agent_id);
        if (action_it == this->readVertexID.end()) {
            throw std::runtime_error("GraphDotImporter::readLinkTeamAgentAction action not found");
        }
        if (team_it == this->readVertexID.end()) {
            throw std::runtime_error("GraphDotImporter::readLinkTeamAgentAction team not found");
        }
        if (agent_it == this->readAgentID.end()) {
            throw std::runtime_error("GraphDotImporter::readLinkTeamAgentAction agent not found");
        }

        this->graph.addNewEdge(team_it->second, action_it->second, agent_it->second);
    }
}

void File::GraphDotImporter::readLinkTeamAgentTeam(std::smatch& matches)
{
    // Creating a edge from a team to a team
    if (!this->lastLine.empty() && !matches.empty()) {
        uint64_t team_in_id = std::stoi(matches[1]);
        uint64_t agent_id = std::stoi(matches[2]);
        uint64_t team_out_id = std::stoi(matches[3]);

        // check object are found
        auto team_in_it = this->readVertexID.find(team_in_id);
        auto team_out_it = this->readVertexID.find(team_out_id);
        auto agent_it = this->readAgentID.find(agent_id);
        if (team_in_it == this->readVertexID.end()) {
            throw std::runtime_error("GraphDotImporter::readLinkTeamAgentTeam team src not found");
        }
        if (team_out_it == this->readVertexID.end()) {
            throw std::runtime_error("GraphDotImporter::readLinkTeamAgentTeam team out not found");
        }
        if (agent_it == this->readAgentID.end()) {
            throw std::runtime_error("GraphDotImporter::readLinkTeamAgentTeam agent not found");
        }

        this->graph.addNewEdge(team_in_it->second, team_out_it->second, agent_it->second);
    }
}

void File::GraphDotImporter::readLinkTeamAgent(std::smatch& matches)
{
    // Add the agent to the team
    if (!this->lastLine.empty() && !matches.empty()) {
        uint64_t team_id = std::stoi(matches[1]);
        uint64_t agent_id = std::stoi(matches[2]);

        // check object are found
        auto team_it = this->readVertexID.find(team_id);
        auto agent_it = this->readAgentID.find(agent_id);
        if (team_it == this->readVertexID.end()) {
            throw std::runtime_error("GraphDotImporter::readLinkTeamAgent team src not found");
        }
        if (agent_it == this->readAgentID.end()) {
            throw std::runtime_error("GraphDotImporter::readLinkTeamAgent agent not found");
        }

        this->graph.setVertexProgram(team_it->second, agent_it->second);
    }
}

void File::GraphDotImporter::readLinkAgentTeam(std::smatch& matches)
{
    // Add the team to the agent
    if (!this->lastLine.empty() && !matches.empty()) {
        uint64_t agent_id = std::stoi(matches[1]);
        uint64_t team_id = std::stoi(matches[2]);

        // check object are found
        auto agent_it = this->readAgentID.find(agent_id);
        auto team_it = this->readVertexID.find(team_id);
        if (agent_it == this->readAgentID.end()) {
            throw std::runtime_error("GraphDotImporter::readLinkreadLinkAgentTeamTeamAgent agent not found");
        }
        if (team_it == this->readVertexID.end()) {
            throw std::runtime_error("GraphDotImporter::readLinkAgentTeam team src not found");
        }

        Algorithm::Algorithm& algorithm = this->getAlgorithm(agent_it->second.get().getAlgorithmID());
        algorithm.linkAgentVertex(agent_it->second, team_it->second);
    }
}

void File::GraphDotImporter::importGraph(const char* filePath)
{
    pFile.open(filePath);
    if (!pFile.is_open()) {
        throw std::runtime_error("Could not open file " +
                                    std::string(filePath));
    }

    // clear every storing objects
    algorithm.getManager().clearAgents(this->graph);
    this->readVertexID.clear();
    this->readEdgeID.clear();
    this->readAgentID.clear();
    // skip header
    int majorVersion = 0;
    int minorVersion = 0;
    int patchVersion = 0;
    if (this->getExportedVersion(majorVersion, minorVersion, patchVersion)) {
        if (majorVersion < supportedMajorVersion ||
            (majorVersion == supportedMajorVersion &&
             minorVersion < supportedMinorVersion) ||
            (majorVersion == supportedMajorVersion &&
             minorVersion == supportedMinorVersion &&
             patchVersion < supportedPatchVersion)) {
            std::cerr
                << "Deprecating: The file was exported with an older version "
                   "of GEGELATI (v"
                << majorVersion << "." << minorVersion << "." << patchVersion
                << "). Some features are no longer supported in the "
                   "current importer version (v"
                << supportedMajorVersion << "." << supportedMinorVersion << "."
                << supportedPatchVersion << ")." << std::endl;
            // throw std::runtime_error(
            //     "The file was exported with an unsupported GEGELATI
            //     version.");
        }
    }
    else {
        std::cerr
            << "Warning: The file does not contain a version header. "
               "Assuming it is compatible with the current importer version."
            << std::endl;
    }
    // force seek at the beginning of file.
    pFile.seekg(0);
    // Skip header
    this->dumpGraphHeader();
    // Read algorithm subGraph
    this->readAlgorithmGraphSubGraph();
    bool read = true;
    while (read) {
        read = this->readLineFromFile();
    }
}

bool File::GraphDotImporter::readLineFromFile()
{
    char buffer[MAX_READ_SIZE];

    std::regex testActionDeclare(this->actionRegex);
    std::regex testTeamDeclare(this->teamRegex);
    std::regex testAgentDeclare(this->agentRegex);
    std::regex testLinkTPA(this->linkTeamAgentActionRegex);
    std::regex testLinkTPT(this->linkTeamAgentTeamRegex);
    std::regex testLinkTP(this->linkTeamAgentRegex);
    std::regex testLinkPT(this->linkAgentTeamRegex);

    std::smatch matches;

    if (!pFile.getline(buffer, MAX_READ_SIZE))
        throw std::ifstream::failure("Couldn't read in the given file");
    else {
        this->lastLine = buffer;
    }


    // check the line shape and parse it
    if (std::regex_search(this->lastLine, matches, testTeamDeclare)) {
        readTeam(matches);
    }
    else if (std::regex_search(this->lastLine, matches, testActionDeclare)) {
        readAction(matches);
    }
    else if (std::regex_search(this->lastLine, matches, testAgentDeclare)) {
        readAgent(matches);
    }
    else if (std::regex_search(this->lastLine, matches, testLinkTPA)) {
        readLinkTeamAgentAction(matches);
    }
    else if (std::regex_search(this->lastLine, matches, testLinkTPT)) {
        readLinkTeamAgentTeam(matches);
    }
    else if (std::regex_search(this->lastLine, matches, testLinkTP)) {
        readLinkTeamAgent(matches);
    }
    else if (std::regex_search(this->lastLine, matches, testLinkPT)) {
        readLinkAgentTeam(matches);
    }
    else {
        return false;
    }
    return true;
}