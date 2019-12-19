#include "importer/tpgGraphDotImporter.h"



void Importer::TPGGraphDotImporter::readParameters(std::string & str, Program::Line & l)
{
	std::string::size_type pos = 0;
	std::string::size_type pos2;

	uint64_t p_idx = 0;
	
	int16_t p_value;
	while(!str.empty())
	{
		pos2 = str.find("|");
		p_value = std::stoi(str.substr(pos, pos2));
		pos2++; 									// skip the '|'
		str = str.substr(pos2, str.size());			// store the rest of the string and iterate
		l.setParameter(p_idx, {p_value});
		p_idx++;
	}
}

void Importer::TPGGraphDotImporter::readOperands(std::string & str, Program::Line & l)
{
	std::string::size_type pos = 0;
	std::string::size_type pos2;
	std::string::size_type pos3;

	uint64_t o_idx = 0;
	uint64_t dataIndex = 0;
	uint64_t location = 0;

	for(int i = 0; i < this->tpg.getEnvironment().getMaxNbOperands(); ++i )
	{
		pos2 = str.find("|");
		dataIndex = std::stoi(str.substr(pos, pos2));
		pos2++; 												// skip the '|'
		pos3 = str.find("#");
		location = std::stoi(str.substr(pos2, pos3 - pos2));
		pos3;
		str = str.substr(pos3 + 1, str.size() - pos3);			// store the rest of the string and iterate

		l.setOperand(o_idx, dataIndex, location, true);
		o_idx++;
	}
}

void Importer::TPGGraphDotImporter::readLine()
{
	if(!this->lastLine.empty() && !matches.empty())
	{
		uint64_t prog_label= std::stoi(matches[1]);
		std::string label = matches[2];

		auto p_it = programID.find(prog_label);
		if(p_it != programID.end() && !label.empty())
		{
			std::shared_ptr<Program::Program> p = p_it->second;

			std::string instruction;

			std::string::size_type pos;
			std::string::size_type pos1;
			std::string::size_type pos2;

			uint64_t instructionIdx;
			uint64_t destinationIdx;
			std::string parameters;
			std::string operands;


			bool cont = true;
			while (cont)
			{
				Program::Line& l = p.get()->addNewLine();
				pos = label.find(this->lineSeparator);
				instruction = label.substr(0,pos);
				label = label.substr(pos+this->lineSeparator.size(), label.size());
				
				pos1 = instruction.find("|");
				instructionIdx = std::stoi(instruction.substr(0, pos1)); 			//extract everything before pos (ie |)

				//extract destination index;
				pos2 = instruction.find("&");
				pos1++; 															//skip the '|'
				destinationIdx = std::stoi(instruction.substr(pos1, pos2-pos1));	//extract and convert to int
				
				//extract parameters as a string
				pos1 = instruction.find("$");										//parameters are in between & and $
				pos2++;																// skip the '&'
				parameters = instruction.substr(pos2, pos1 - pos2);
				
				//extract operands as a string
				pos2 = instruction.size() - 1; 										//corresponds to the last index of the string.
				pos1++;																//slip the '$'
				operands = instruction.substr(pos1, instruction.size());

				//add indexes to line 
				l.setInstructionIndex(instructionIdx);
				l.setDestinationIndex(destinationIdx);

				//parse parameters
				readParameters(parameters,l);

				//parse operands
				readOperands(operands,l);
				
				if(label.size() <= 3)
				{
					cont = false;
				}
			}
		}
	}
}

void Importer::TPGGraphDotImporter::readProgram()
{
	if(!this->lastLine.empty() && !matches.empty())
	{
		// Program definition : 
		// P0 [fillcolor="#cccccc" shape=point]
		this->programID.insert(std::pair<uint64_t, std::shared_ptr<Program::Program>>(std::stoi(this->matches[1]), 
			new Program::Program(this->tpg.getEnvironment())));
	}	
}

void Importer::TPGGraphDotImporter::dumpTPGGraphHeader()
{
	//skips the three first lines of the file
	char buffer[MAX_READ_SIZE];
	pFile.getline(buffer, MAX_READ_SIZE);
	pFile.getline(buffer, MAX_READ_SIZE);
	pFile.getline(buffer, MAX_READ_SIZE);
}

void Importer::TPGGraphDotImporter::readTeam()
{
	if(!this->lastLine.empty() && !matches.empty())
	{
		this->vertexID.insert(std::pair<uint64_t, const TPG::TPGVertex*>(std::stoi(matches[1]), &this->tpg.addNewTeam()));
	}
}

void Importer::TPGGraphDotImporter::readAction()
{
	if(!this->lastLine.empty() && !matches.empty())
	{
		uint64_t action_label = std::stoi(matches[2]);
		uint64_t action_number = std::stoi(matches[1]);

		auto elmt = actionID.find(action_label);
		if (elmt == actionID.end()) 
		{
			this->actionID.insert(std::pair<uint64_t, const TPG::TPGVertex*>(action_label, &this->tpg.addNewAction(action_label)));
		}
		this->actionLabel.insert(std::pair<uint64_t, uint64_t>(action_number,action_label));
	}
}

void Importer::TPGGraphDotImporter::readLinkTeamProgramAction()
{
	if(!this->lastLine.empty() && !matches.empty())
	{
		uint64_t team_in = std::stoi(matches[1]);
		uint64_t program = std::stoi(matches[2]);
		uint64_t act_out = std::stoi(matches[3]);

		auto action_lab =  this->actionLabel.find(act_out);
		if (action_lab != this->actionLabel.end())
		{
			auto team_it = this->vertexID.find(team_in);
			auto action_it = this->actionID.find(action_lab->second);
			auto p_it = programID.find(program);
			if(team_it != vertexID.end() && action_it != this->actionID.end() && p_it != programID.end())
			{
				const TPG::TPGVertex * team = team_it->second;
				const TPG::TPGVertex * action = action_it->second;
				std::shared_ptr<Program::Program> p = p_it->second;
				this->tpg.addNewEdge(*team, *action, p);
			}
		}
	}
}

void Importer::TPGGraphDotImporter::readLinkTeamProgramTeam()
{
	if(!this->lastLine.empty() && !matches.empty())
	{
		uint64_t team_in  = std::stoi(matches[1]);
		uint64_t program  = std::stoi(matches[2]);
		uint64_t team_out = std::stoi(matches[3]);

		auto t1_it = this->vertexID.find(team_in);
		auto t2_it = this->vertexID.find(team_out);

		auto p_it = programID.find(program);
		if(p_it != programID.end())
		{
			std::shared_ptr<Program::Program> p = p_it->second;

			if(t1_it != this->vertexID.end() && t2_it != this->vertexID.end())
			{
				const TPG::TPGVertex * team_i = t1_it->second;
				const TPG::TPGVertex * team_o = t2_it->second;
				this->tpg.addNewEdge(*team_i, *team_o, p);
			}
		}
	}
}

void Importer::TPGGraphDotImporter::readLinkTeamProgram()
{
	if(!this->lastLine.empty() && !matches.empty())
	{
		uint64_t team_in  = std::stoi(matches[1]);
		uint64_t program  = std::stoi(matches[2]);

		//find edge
		const std::list<TPG::TPGEdge>& edges = this->tpg.getEdges();

		//find one of the selected program edges : 
		auto p_it = programID.find(program);
		if(p_it != programID.end())
		{
			std::shared_ptr<Program::Program> p = p_it->second;

			auto edge_it = std::find_if(edges.begin(), edges.end(), 
				[p](const TPG::TPGEdge& other) {
				if(&(other.getProgram()) == p.get())
					return true;
				return false;
			});
			if(edge_it != edges.end())			//we got the corresponding edge : 
			{
				//then get the team : 
				auto team_it = this->vertexID.find(team_in);
				if(team_it!= this->vertexID.end())
				{
					const TPG::TPGVertex * team = team_it->second;
					this->tpg.addNewEdge(*team,*edge_it->getDestination(), p);
				}
			}
		}
	}
}


TPG::TPGGraph& Importer::TPGGraphDotImporter::importGraph()
{
	//force seek at the beginning of file.
	pFile.seekg(0);

	//clear every storing objects
	this->tpg.clear();
	this->vertexID.clear();
	this->actionID.clear();
	this->actionLabel.clear();
	this->programID.clear();

	// skip header
	this->dumpTPGGraphHeader();
	bool read = true;
	while(read)
	{
		read = this->readLineFromFile();
	}
	return this->tpg;
}


bool Importer::TPGGraphDotImporter::readLineFromFile()
{
	char buffer[MAX_READ_SIZE];

	std::regex testActionDeclare(this->actionRegex);
	std::regex testTeamDeclare(this->teamRegex);
	std::regex testProgramDeclare(this->programRegex);
	std::regex testInstructionDeclare(this->instructionRegex);
	std::regex testLinkPI(this->linkProgramInstructionRegex);
	std::regex testLinkTPA(this->linkProgramActionRegex);
	std::regex testLinkTPT(this->linkProgramTeamRegex);
	std::regex testLinkTP(this->addLinkProgramRegex);

	if (!pFile.getline(buffer, MAX_READ_SIZE))
		throw std::ifstream::failure("Couldn't read in the given file");
	else
	{
		this->lastLine = buffer;
	}

	// check the line shape and parse it 
	if(std::regex_search(this->lastLine, this->matches, testTeamDeclare))
	{
		readTeam();
	}
	else if(std::regex_search(this->lastLine, this->matches, testActionDeclare))
	{
		readAction();
	}
	else if(std::regex_search(this->lastLine, this->matches, testProgramDeclare))
	{
		readProgram();
	}
	else if(std::regex_search(this->lastLine, this->matches, testInstructionDeclare))
	{
		readLine();
	}
	else if(std::regex_search(this->lastLine, this->matches, testLinkPI))
	{
		// by definition, a program is linked to its instruction from its declaration.
		// the link is used vor visualisation but doesn't require to be parsed
		return true;
	}
	else if(std::regex_search(this->lastLine, this->matches, testLinkTPA))
	{
		readLinkTeamProgramAction();
	}
	else if(std::regex_search(this->lastLine, this->matches, testLinkTPT))
	{
		readLinkTeamProgramTeam();
	}
	else if(std::regex_search(this->lastLine, this->matches, testLinkTP))
	{
		readLinkTeamProgram();
	}
	else
	{
		return false;
	}
	return true;
}