/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2020 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
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
#include "file/graphDotImporter.h"

const std::string File::GraphDotImporter::representationRegex(
    "ALGO([0-9]+)\\x20\\x5B.*label=\"([^\\.]+)\\.([0-9]+)\".*");
const std::string File::GraphDotImporter::subRepresentationLinkRegex(
    "ALGO([0-9]+)\\x20->\\x20ALGO([0-9]+)");
const std::string File::GraphDotImporter::endRepresentationSubGraph(R"(^\s*\}\s*$)");


const std::string File::GraphDotImporter::teamRegex(
    "T([0-9]+)\\x20\\x5B.*fillcolor.*\\x5D");
const std::string File::GraphDotImporter::individualRegex(
    "P([0-9]+)\\x20\\x5B.*label=\"([^\\.]+)\\.([0-9]+)\".*");
const std::string File::GraphDotImporter::actionRegex(
    "A([0-9]+)\\x20\\x5B.*=\"(.*)\"\\x5D");
const std::string File::GraphDotImporter::linkTeamIndividualActionRegex(
    "T([0-9]+)\\x20->\\x20P([0-9]+)\\x20->\\x20A([0-9]+).*");
const std::string File::GraphDotImporter::linkTeamIndividualTeamRegex(
    "T([0-9]+)\\x20->\\x20P([0-9]+)\\x20->\\x20T([0-9]+).*");
const std::string File::GraphDotImporter::linkTeamIndividualRegex(
    "T([0-9]+)\\x20->\\x20P([0-9]+)");
const std::string File::GraphDotImporter::linkIndividualTeamRegex(
    "P([0-9]+)\\x20->\\x20T([0-9]+)");

Representation::Representation& File::GraphDotImporter::getRepresentation(uint64_t representationID)
{
    auto it = this->mapRepresentations.find(representationID);
    if(it == this->mapRepresentations.end()){
        throw std::runtime_error("GraphDotImporter::getRepresentation Representation not found"); 
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

void File::GraphDotImporter::readRepresentation(std::smatch matches)
{
    std::string name = matches[2];
    uint64_t id = std::stoi(matches[3]);
    
    auto it = this->mapRepresentations.find(id);
    if(it == this->mapRepresentations.end() || it->second.get().getRepresentationName() != name){
        throw std::runtime_error("GraphDotImporter::readRepresentation: Representation doesnt correspond"); 
    }
}

void File::GraphDotImporter::readSubRepresentationLink(std::smatch matches)
{
    uint64_t id_src = std::stoi(matches[1]);
    uint64_t id_dest = std::stoi(matches[2]);

    auto it_src = this->mapRepresentations.find(id_src);
    if(it_src == this->mapRepresentations.end()){
        throw std::runtime_error("GraphDotImporter::readSubRepresentationLink: source representation not found"); 
    }

    auto it_dest = this->mapRepresentations.find(id_dest);
    if(it_dest == this->mapRepresentations.end()){
        throw std::runtime_error("GraphDotImporter::readSubRepresentationLink: destination representation not found"); 
    }

    // Will throw if nothing is found
    it_src->second.get().cGetSubRepresentation(id_dest);
}

void File::GraphDotImporter::readRepresentationGraphSubGraph()
{
    char buffer[MAX_READ_SIZE];
    // Skip subGraph header (should be 5 lines)
    for (int i = 0; i < 5; i++) {
        pFile.getline(buffer, MAX_READ_SIZE);
    }

    this->setMapRepresentation();

    std::regex testRepresentationRegex(this->representationRegex);
    std::regex testSubRepresentationLinkRegex(this->subRepresentationLinkRegex);
    std::regex testEndRepresentationSubGraph(this->endRepresentationSubGraph);


    std::smatch matches;
    
    bool read = true;
    while (read) {    
        if (!pFile.getline(buffer, MAX_READ_SIZE))
            throw std::ifstream::failure("Couldn't read in the given file");
        else {
            this->lastLine = buffer;
        }

        // check the line shape and parse it
        if (std::regex_search(this->lastLine, matches, testRepresentationRegex)) {
            this->readRepresentation(matches);
        } else if (std::regex_search(this->lastLine, matches, testSubRepresentationLinkRegex)) {
            this->readSubRepresentationLink(matches);
        } else if (std::regex_search(this->lastLine, matches, testEndRepresentationSubGraph)) {
            read = false;
        } else {
        }
    }
}


void File::GraphDotImporter::setMapRepresentation()
{
    this->mapRepresentations.clear();
    // Add all representations to the set, and recursively all their sub-representations, to be able to print the content of the programs when they are mutated by the representation.
    std::vector<std::reference_wrapper<Representation::Representation>> representationsToAdd;
    representationsToAdd.push_back(representation);
    while(!representationsToAdd.empty()){
        Representation::Representation& representation = representationsToAdd.back();
        representationsToAdd.pop_back();

        if(this->mapRepresentations.find(representation.getRepresentationID()) == this->mapRepresentations.end()){
            this->mapRepresentations.insert({representation.getRepresentationID(), representation});
            for(Representation::Representation& subRepresentation : representation.getSubRepresentations()){
                representationsToAdd.push_back(subRepresentation);
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

void File::GraphDotImporter::readIndividual(std::smatch& matches) {
    if (!this->lastLine.empty() && !matches.empty()) {
        uint64_t individualID = std::stoi(matches[1]);
        std::string representationName = matches[2];
        uint64_t representationID = std::stoi(matches[3]);
        
        Representation::Representation& representation = this->getRepresentation(representationID);
        const Representation::Individual& individual = representation.readIndividual(matches);

        this->readIndividualID.insert({std::stoi(matches[1]), individual});

        representation.getPopulation().setNewIndividualID(individual, individualID);
    }
}

void File::GraphDotImporter::readLinkTeamIndividualAction(std::smatch& matches)
{
    // Creating a edge from a team to an action
    if (!this->lastLine.empty() && !matches.empty()) {
        uint64_t team_id = std::stoi(matches[1]);
        uint64_t individual_id = std::stoi(matches[2]);
        uint64_t action_id = std::stoi(matches[3]);

        // check object are found
        auto action_it = this->readVertexID.find(action_id);
        auto team_it = this->readVertexID.find(team_id);
        auto individual_it = this->readIndividualID.find(individual_id);
        if (action_it == this->readVertexID.end()) {
            throw std::runtime_error("GraphDotImporter::readLinkTeamIndividualAction action not found");
        }
        if (team_it == this->readVertexID.end()) {
            throw std::runtime_error("GraphDotImporter::readLinkTeamIndividualAction team not found");
        }
        if (individual_it == this->readIndividualID.end()) {
            throw std::runtime_error("GraphDotImporter::readLinkTeamIndividualAction individual not found");
        }

        this->graph.addNewEdge(team_it->second, action_it->second, individual_it->second);
    }
}

void File::GraphDotImporter::readLinkTeamIndividualTeam(std::smatch& matches)
{
    // Creating a edge from a team to a team
    if (!this->lastLine.empty() && !matches.empty()) {
        uint64_t team_in_id = std::stoi(matches[1]);
        uint64_t individual_id = std::stoi(matches[2]);
        uint64_t team_out_id = std::stoi(matches[3]);

        // check object are found
        auto team_in_it = this->readVertexID.find(team_in_id);
        auto team_out_it = this->readVertexID.find(team_out_id);
        auto individual_it = this->readIndividualID.find(individual_id);
        if (team_in_it == this->readVertexID.end()) {
            throw std::runtime_error("GraphDotImporter::readLinkTeamIndividualTeam team src not found");
        }
        if (team_out_it == this->readVertexID.end()) {
            throw std::runtime_error("GraphDotImporter::readLinkTeamIndividualTeam team out not found");
        }
        if (individual_it == this->readIndividualID.end()) {
            throw std::runtime_error("GraphDotImporter::readLinkTeamIndividualTeam individual not found");
        }

        this->graph.addNewEdge(team_in_it->second, team_out_it->second, individual_it->second);
    }
}

void File::GraphDotImporter::readLinkTeamIndividual(std::smatch& matches)
{
    // Add the individual to the team
    if (!this->lastLine.empty() && !matches.empty()) {
        uint64_t team_id = std::stoi(matches[1]);
        uint64_t individual_id = std::stoi(matches[2]);

        // check object are found
        auto team_it = this->readVertexID.find(team_id);
        auto individual_it = this->readIndividualID.find(individual_id);
        if (team_it == this->readVertexID.end()) {
            throw std::runtime_error("GraphDotImporter::readLinkTeamIndividual team src not found");
        }
        if (individual_it == this->readIndividualID.end()) {
            throw std::runtime_error("GraphDotImporter::readLinkTeamIndividual individual not found");
        }

        this->graph.setVertexProgram(team_it->second, individual_it->second);
    }
}

void File::GraphDotImporter::readLinkIndividualTeam(std::smatch& matches)
{
    // Add the team to the individual
    if (!this->lastLine.empty() && !matches.empty()) {
        uint64_t individual_id = std::stoi(matches[1]);
        uint64_t team_id = std::stoi(matches[2]);

        // check object are found
        auto individual_it = this->readIndividualID.find(individual_id);
        auto team_it = this->readVertexID.find(team_id);
        if (individual_it == this->readIndividualID.end()) {
            throw std::runtime_error("GraphDotImporter::readLinkreadLinkIndividualTeamTeamIndividual individual not found");
        }
        if (team_it == this->readVertexID.end()) {
            throw std::runtime_error("GraphDotImporter::readLinkIndividualTeam team src not found");
        }

        Representation::Representation& representation = this->getRepresentation(individual_it->second.get().getRepresentationID());
        representation.linkIndividualVertex(individual_it->second, team_it->second);
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
    representation.getPopulation().clearIndividuals(this->graph);
    this->readVertexID.clear();
    this->readEdgeID.clear();
    this->readIndividualID.clear();
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
    // Read representation subGraph
    this->readRepresentationGraphSubGraph();
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
    std::regex testIndividualDeclare(this->individualRegex);
    std::regex testLinkTPA(this->linkTeamIndividualActionRegex);
    std::regex testLinkTPT(this->linkTeamIndividualTeamRegex);
    std::regex testLinkTP(this->linkTeamIndividualRegex);
    std::regex testLinkPT(this->linkIndividualTeamRegex);

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
    else if (std::regex_search(this->lastLine, matches, testIndividualDeclare)) {
        readIndividual(matches);
    }
    else if (std::regex_search(this->lastLine, matches, testLinkTPA)) {
        readLinkTeamIndividualAction(matches);
    }
    else if (std::regex_search(this->lastLine, matches, testLinkTPT)) {
        readLinkTeamIndividualTeam(matches);
    }
    else if (std::regex_search(this->lastLine, matches, testLinkTP)) {
        readLinkTeamIndividual(matches);
    }
    else if (std::regex_search(this->lastLine, matches, testLinkPT)) {
        readLinkIndividualTeam(matches);
    }
    else {
        return false;
    }
    return true;
}