/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2022) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2020 - 2022)
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

#include "file/tpgGraphDotImporter.h"

const std::string File::TPGGraphDotImporter::lineSeparator("&#92;n");
const std::string File::TPGGraphDotImporter::teamRegex(
    "T([0-9]+)\\x20\\x5B.*\\x5D");
const std::string File::TPGGraphDotImporter::programRegex(
    "P([0-9]+)\\x20\\x5B.*label=\"(.*)\"*\\x5D");
const std::string File::TPGGraphDotImporter::instructionRegex(
    "I([0-9]+)\\x20\\x5B.*label=\"(.*)\"\\x5D");
const std::string File::TPGGraphDotImporter::actionRegex(
    "A([0-9]+)\\x20\\x5B.*=\"(.*)\"\\x5D");
const std::string File::TPGGraphDotImporter::linkProgramInstructionRegex(
    "P([0-9]+)\\x20->\\x20I([0-9]+).*");
const std::string File::TPGGraphDotImporter::linkProgramActionRegex(
    "T([0-9]+)\\x20->\\x20P([0-9]+)\\x20->\\x20A([0-9]+).*");
const std::string File::TPGGraphDotImporter::linkProgramTeamRegex(
    "T([0-9]+)\\x20->\\x20P([0-9]+)\\x20->\\x20T([0-9]+).*");
const std::string File::TPGGraphDotImporter::linkActionProgramRegex(
    "A([0-9]+)\\x20->\\x20P([0-9]+).*");
const std::string File::TPGGraphDotImporter::addLinkProgramRegex(
    "T([0-9]+)\\x20->\\x20P([0-9]+)");

void File::TPGGraphDotImporter::readOperands(std::string& str, Program::Line& l)
{
    std::string::size_type pos = 0;
    std::string::size_type pos2;
    std::string::size_type pos3;

    uint64_t o_idx = 0;
    uint64_t dataIndex = 0;
    uint64_t location = 0;

    // operands are stored in str with the following format :
    // op1_param1|op1_param2#...|param_N

    for (int i = 0; i < this->tpg.getEnvironment().getMaxNbOperands(); ++i) {
        pos2 = str.find("|");
        dataIndex = std::stoi(str.substr(pos, pos2));
        pos2++; // skip the '|'
        pos3 = str.find("#");
        location = std::stoi(str.substr(pos2, pos3 - pos2));
        str = str.substr(pos3 + 1,
                         str.size() -
                             pos3); // store the rest of the string and iterate

        l.setOperand(o_idx, dataIndex, location, true);
        o_idx++;
    }
}

void File::TPGGraphDotImporter::readLine(std::smatch& matches)
{
    // a line is stored in the .dot file with the following format
    // inst_idx|dest_idx&op1_param1|op1_param2#...#opN_param1|opN_param2
    if (!this->lastLine.empty() && !matches.empty()) {
        uint64_t prog_label = std::stoi(matches[1]);
        std::string label = matches[2];

        auto p_it = programID.find(prog_label);
        if (p_it != programID.end() && !label.empty()) {

            std::shared_ptr<Program::Program> p = p_it->second;
            // stores the whole program
            std::string instruction;

            // used to seek delimiters in the variable "instruction"
            std::string::size_type pos;
            std::string::size_type pos1;
            std::string::size_type pos2;

            // instruction index of a line
            uint64_t instructionIdx;
            // destination index of a line
            uint64_t destinationIdx;
            // store operands in a new string
            std::string operands;

            // as long as there are lines in the program, parse those lines
            bool cont = true;
            while (cont) {
                Program::Line& l = p.get()->addNewLine();
                pos = label.find(this->lineSeparator);
                instruction = label.substr(0, pos);
                label = label.substr(pos + this->lineSeparator.size(),
                                     label.size());

                // extract everything before pos (ie |)
                // corresponds to instruction index
                pos1 = instruction.find("|");
                instructionIdx = std::stoi(instruction.substr(0, pos1));

                // extract destination index;
                pos2 = instruction.find("&");
                pos1++; // skip the '|'
                destinationIdx = std::stoi(instruction.substr(
                    pos1, pos2 - pos1)); // extract and convert to int

                // extract operands as a string
                pos2++; // skip the '$'
                operands = instruction.substr(pos2, instruction.size());

                // add indexes to line
                l.setInstructionIndex(instructionIdx);
                l.setDestinationIndex(destinationIdx);

                // parse operands
                readOperands(operands, l);

                if (label.size() <= 3) {
                    cont = false;
                }
            }
            p->identifyIntrons();

            std::string::size_type pos3;
            std::string::size_type pos4;
            // read constants
            std::vector<double> v_constant;
            pos3 = this->lastLine.find("//") + 2;
            pos4 = this->lastLine.find("|", pos3);
            for (;;) {
                if (pos4 != std::string::npos) {
                    v_constant.push_back(std::stod(this->lastLine.substr(
                        pos3, pos4 - pos3))); // Convertit la chaîne en double
                }
                else {
                    break;
                }
                pos3 = pos4 + 1;
                pos4 = this->lastLine.find("|", pos3);
            }
            // set the previously read constants
            p->setLineConstants(v_constant);
            this->programID.insert(
                std::pair<uint64_t, std::shared_ptr<Program::Program>>(
                    std::stoi(matches[1]), p));
        }
    }
}

void File::TPGGraphDotImporter::readProgram(std::smatch& matches)
{
    if (!this->lastLine.empty() && !matches.empty()) {
        // Program definition :
        // P0 [fillcolor="#cccccc" shape=point] //const0|const1|...|constn|
        std::string::size_type pos;
        std::string::size_type pos1;
        // read constants
        std::vector<Data::Constant> v_constant;
        pos = this->lastLine.find("//") + 2;
        pos1 = this->lastLine.find("|", pos);
        for (;;) {
            if (pos1 != std::string::npos) {
                v_constant.push_back(
                    {std::stod(this->lastLine.substr(pos, pos1 - pos))});
            }
            else {
                break;
            }
            pos = pos1 + 1;
            pos1 = this->lastLine.find("|", pos);
        }

        // Get if the program is an action or context program.
        bool isActionProgram = std::stoi(matches[2]);

        // create new program with the correct amount of constants
        Program::Program* p =
            new Program::Program(this->tpg.getEnvironment(), isActionProgram);
        // set the previously read constants
        for (int i = 0; i < v_constant.size(); i++) {
            p->getConstantHandler().setDataAt(typeid(Data::Constant), i,
                                              v_constant.at(i));
        }
        this->programID.insert(
            std::pair<uint64_t, std::shared_ptr<Program::Program>>(
                std::stoi(matches[1]), p));
    }
}

void File::TPGGraphDotImporter::dumpTPGGraphHeader()
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

void File::TPGGraphDotImporter::readTeam(std::smatch& matches)
{
    if (!this->lastLine.empty() && !matches.empty()) {
        this->vertexID.insert(std::pair<uint64_t, const TPG::TPGVertex*>(
            std::stoi(matches[1]), &this->tpg.addNewTeam()));
    }
}
void File::TPGGraphDotImporter::readAction(std::smatch& matches)
{
    if (!this->lastLine.empty() && !matches.empty()) {
        // the regex matches two groups (action number and action label)
        std::string action_label = matches[2];
        uint64_t action_number = std::stoi(matches[1]);

        // Vector to store the extracted numbers
        std::vector<uint64_t> numbers;

        // Use a stringstream to parse the input
        std::stringstream ss(action_label);
        std::string token;

        // Split the string by the delimiter '-'
        while (std::getline(ss, token, '-')) {
            numbers.push_back(std::stoull(
                token)); // Convert the token to uint64_t and store it
        }
        // elmt points to the action with the same label as the action we are
        // parsing
        auto elmt = actionID.find(action_number);
        if (elmt == actionID.end()) {

            uint64_t currActionID = action_number;
            if (tpg.getEnvironment().getNbContinuousActions() == 0) {
                currActionID = std::stoi(action_label);
            }

            // create a new action and insert it if none was previously found
            this->actionID.insert(std::pair<uint64_t, const TPG::TPGAction*>(
                action_number, dynamic_cast<const TPG::TPGAction*>(&this->tpg.addNewAction(currActionID))));
        }
        this->actionClasses.insert(
            std::pair<const TPG::TPGAction*, std::vector<uint64_t>>(
                dynamic_cast<const TPG::TPGAction*>(this->tpg.getVertices().back()), numbers));
    }
}

void File::TPGGraphDotImporter::readLinkActionProgram(std::smatch& matches)
{
    // Creating a edge from a team to an action
    if (!this->lastLine.empty() && !matches.empty()) {
        uint64_t act_in = std::stoi(matches[1]);
        uint64_t program = std::stoi(matches[2]);

        // get the action depending on its label
        auto action_it = this->actionID.find(act_in);
        if (action_it != this->actionID.end()) {
            // find the program to add to the edge
            auto p_it = programID.find(program);
            if (action_it != this->actionID.end() && p_it != programID.end()) {
                const TPG::TPGAction* action = action_it->second;

                uint64_t actionClass = this->actionClasses.at(action).at(
                    action->getOutgoingEdges().size());
                std::shared_ptr<Program::Program> p = p_it->second;
                this->tpg.addNewActionEdge(*action, p, actionClass);
            }
        }
    }
}

void File::TPGGraphDotImporter::readLinkTeamProgramAction(std::smatch& matches)
{
    // Creating a edge from a team to an action
    if (!this->lastLine.empty() && !matches.empty()) {
        uint64_t team_in = std::stoi(matches[1]);
        uint64_t program = std::stoi(matches[2]);
        uint64_t act_out = std::stoi(matches[3]);

        // get the action depending on its label
        auto action_it = this->actionID.find(act_out);
        if (action_it != this->actionID.end()) {
            auto team_it = this->vertexID.find(team_in);
            // find the program to add to the edge
            auto p_it = programID.find(program);
            if (team_it != vertexID.end() &&
                action_it != this->actionID.end() && p_it != programID.end()) {
                const TPG::TPGVertex* team = team_it->second;
                const TPG::TPGVertex* action = action_it->second;
                std::shared_ptr<Program::Program> p = p_it->second;
                this->tpg.addNewEdge(*team, *action, p);
            }
        }
    }
}

void File::TPGGraphDotImporter::readLinkTeamProgramTeam(std::smatch& matches)
{
    // creating a edge between two teams
    if (!this->lastLine.empty() && !matches.empty()) {
        uint64_t team_in = std::stoi(matches[1]);
        uint64_t program = std::stoi(matches[2]);
        uint64_t team_out = std::stoi(matches[3]);

        // get the source and destination teams
        auto t1_it = this->vertexID.find(team_in);
        auto t2_it = this->vertexID.find(team_out);

        // find the program
        auto p_it = programID.find(program);
        if (p_it != programID.end()) {
            std::shared_ptr<Program::Program> p = p_it->second;
            if (t1_it != this->vertexID.end() &&
                t2_it != this->vertexID.end()) {
                const TPG::TPGVertex* team_i = t1_it->second;
                const TPG::TPGVertex* team_o = t2_it->second;
                this->tpg.addNewEdge(*team_i, *team_o, p);
            }
        }
    }
}

void File::TPGGraphDotImporter::readLinkTeamProgram(std::smatch& matches)
{
    if (!this->lastLine.empty() && !matches.empty()) {
        uint64_t team_in = std::stoi(matches[1]);
        uint64_t program = std::stoi(matches[2]);

        // find edge
        const std::list<std::unique_ptr<TPG::TPGEdge>>& edges =
            this->tpg.getEdges();

        // find one of the selected program edges :
        auto p_it = programID.find(program);
        if (p_it != programID.end()) {
            std::shared_ptr<Program::Program> p = p_it->second;

            auto edge_it =
                std::find_if(edges.begin(), edges.end(),
                             [p](const std::unique_ptr<TPG::TPGEdge>& other) {
                                 return (&(other->getProgram()) == p.get());
                             });
            if (edge_it != edges.end()) // we got the corresponding edge :
            {
                // then get the team :
                auto team_it = this->vertexID.find(team_in);
                if (team_it != this->vertexID.end()) {
                    const TPG::TPGVertex* team = team_it->second;
                    this->tpg.addNewEdge(
                        *team, *(edge_it->get()->getDestination()), p);
                }
            }
        }
    }
}

void File::TPGGraphDotImporter::importGraph()
{
    // force seek at the beginning of file.
    pFile.seekg(1);

    // clear every storing objects
    this->tpg.clear();
    this->vertexID.clear();
    this->actionID.clear();
    this->actionClasses.clear();
    this->programID.clear();

    // skip header
    this->dumpTPGGraphHeader();
    bool read = true;
    while (read) {
        read = this->readLineFromFile();
    }
}

bool File::TPGGraphDotImporter::readLineFromFile()
{
    char buffer[MAX_READ_SIZE];

    std::regex testActionDeclare(this->actionRegex);
    std::regex testTeamDeclare(this->teamRegex);
    std::regex testProgramDeclare(this->programRegex);
    std::regex testInstructionDeclare(this->instructionRegex);
    std::regex testLinkPI(this->linkProgramInstructionRegex);
    std::regex testLinkTPA(this->linkProgramActionRegex);
    std::regex testLinkAP(this->linkActionProgramRegex);
    std::regex testLinkTPT(this->linkProgramTeamRegex);
    std::regex testLinkTP(this->addLinkProgramRegex);

    std::smatch matches;

    if (!pFile.getline(buffer, MAX_READ_SIZE))
        throw std::ifstream::failure("Couldn't read in the given file");
    else {
        this->lastLine = buffer;
    }

    // Trouver la position de "//"
    size_t pos = this->lastLine.find("//");
    std::string usedLine;
    if (pos != std::string::npos) {
        // Garder uniquement la partie avant "//"
        usedLine = this->lastLine.substr(0, pos);
    }

    // check the line shape and parse it
    if (std::regex_search(this->lastLine, matches, testTeamDeclare)) {
        readTeam(matches);
    }
    else if (std::regex_search(this->lastLine, matches, testActionDeclare)) {
        readAction(matches);
    }
    else if (std::regex_search(this->lastLine, matches, testProgramDeclare)) {
        readProgram(matches);
    }
    else if (std::regex_search(usedLine, matches, testInstructionDeclare)) {
        readLine(matches);
    }
    else if (std::regex_search(this->lastLine, matches, testLinkPI)) {
        // by definition, a program is linked to its instruction from its
        // declaration. the link is used vor visualisation but doesn't require
        // to be parsed
        return true;
    }
    else if (std::regex_search(this->lastLine, matches, testLinkTPA)) {
        readLinkTeamProgramAction(matches);
    }
    else if (std::regex_search(this->lastLine, matches, testLinkTPT)) {
        readLinkTeamProgramTeam(matches);
    }
    else if (std::regex_search(this->lastLine, matches, testLinkTP)) {
        readLinkTeamProgram(matches);
    }
    else if (std::regex_search(this->lastLine, matches, testLinkAP)) {
        readLinkActionProgram(matches);
    }
    else {
        return false;
    }
    return true;
}

void File::TPGGraphDotImporter::setNewFilePath(const char* newFilePath)
{
    //  Close previous file
    pFile.close();
    // open new one;
    pFile.open(newFilePath);
    if (!pFile.is_open()) {
        throw std::runtime_error("Could not open file " +
                                 std::string(newFilePath));
    }
}
