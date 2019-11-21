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
	
	int16_t p_value;

	for(int i = 0; i < env.getMaxNbOperands(); ++i )
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

void Importer::TPGGraphDotImporter::readLine(std::string & str, Program::Line & l)
{
	std::string::size_type pos;
	std::string::size_type pos2;
	uint64_t instructionIdx;
	uint64_t destinationIdx;
	std::string parameters;
	std::string operands;

	// str contains something like : 2|7&$1|729#0|199
	// str index is the first thing before '|'
	// destination index is in between '|' and '&'
	// parameters are in between '&'' and '$'. Parameters are separated with '|'
	// operands are after parameters and pairs are separated with '#'

	//extract instruction index.
	pos = str.find("|");
	instructionIdx = std::stoi(str.substr(0, pos)); 			//extract everything before pos (ie |)

	//extract destination index;
	pos2 = str.find("&");
	pos++; 																//skip the '|'
	destinationIdx = std::stoi(str.substr(pos, pos2-pos));		//extract and convert to int
	
	//extract parameters as a string
	pos = str.find("$");										//parameters are in between & and $
	pos2++;																// skip the '&'
	parameters = str.substr(pos2, pos - pos2);
	
	//extract operands as a string
	pos2 = str.size() - 1; 										//corresponds to the last index of the string.
	pos++;																//slip the '$'
	operands = str.substr(pos, str.size());

	//add indexes to line 
	l.setInstructionIndex(instructionIdx);
	l.setDestinationIndex(destinationIdx);

	//parse parameters
	readParameters(parameters,l);

	//parse operands
	readOperands(operands,l);
}

void Importer::TPGGraphDotImporter::fillWithLines(std::shared_ptr<Program::Program> & program)
{
	std::string instruction;
	std::string::size_type pos;
	std::string s;

	bool cont = true;
	
	readLineFromFile();

	//test a program declaration
	std::regex testInstructionDeclare(this->instructionRegex);

	if(std::regex_match(this->lastLine,testInstructionDeclare))
	{
		pos = this->lastLine.rfind("=\"")+2;
		s = this->lastLine.substr(pos,this->lastLine.size()-pos);
		//now s contains the label  and ends by &#92;n"]
		while (cont)
		{
			Program::Line& l = program.get()->addNewLine();
			pos = s.find(this->lineSeparator);
			instruction = s.substr(0,pos);
			s = s.substr(pos+this->lineSeparator.size(), s.size());

			readLine(instruction, l);
			if(s.size() <= 3)
			{
				cont = false;
			}
		}
	}
}

void Importer::TPGGraphDotImporter::readPrograms()
{
	//content of the dot file : 
	// P0 [fillcolor="#cccccc" shape=point]
	// I0 [shape=box style=invis label="4|0&$1|44#1|332&#92;n4|6&$1|225#0|454&#92;n1|1&$0|434#1|304&#92;n2|7&$1|729#0|199&#92;n"]
	// P0 -> I0[style=invis]
	// A0 [fillcolor="#ff3366" shape=box margin=0.03 width=0 height=0 label = "0"]
	// P0 -> A0
	// T0 -> P0

	// Program declaration
	// Instruction content : label : instruction_index|destination_index|{parameter1|parameter2|....}|{{op1_1|op1_2}|{op2_1|op2_2}|{otheroperands...}}&#92;n next line
 	// link program with list of instruction
	// Action declaration (optionnal)
	// link outgoing edge
	// link incomming edge


	// we dont read a line here because it has been read to trigger the stopping of the readNumberOfTeams function.
	bool cont = true;
	std::string s;
	std::string::size_type pos1;
	std::string::size_type pos2;

	std::regex testTeamLink(this->teamRegex);
	std::regex testProgramDeclare(this->programRegex);
	std::regex testActionDeclare(this->actionRegex);

	int team_idx;
	int out_team_idx;
	uint64_t action_label;
	uint64_t action_number;
	uint64_t prog_id;

	while (cont)
	{
		if(std::regex_match(this->lastLine,testProgramDeclare))				// we found a program declaration
		{
			auto p = std::make_shared<Program::Program>(env);
			//get the program id
			pos2 =  this->lastLine.rfind(" [");
			pos1 = this->lastLine.rfind("P") + 1;
			prog_id = std::stoi(this->lastLine.substr(pos1, pos2-pos1));
			this->programID.insert(std::pair<uint64_t, std::shared_ptr<Program::Program>>(prog_id, p));
			this->fillWithLines(p);											// fill the program with instructions
			readLineFromFile();												// skip the linking program-instructions

			//check if the programs links to an Action
			readLineFromFile();												//either action declaration or program->team
			if(std::regex_match(this->lastLine,testActionDeclare))			// we found an action declaration
			{
				//get the action label.
				pos2 =  this->lastLine.rfind("\"");
				pos1 = this->lastLine.rfind("=\"") + 2;
				action_label = std::stoi(this->lastLine.substr(pos1, pos2-pos1));
				//check action label and insert if not exixting

				auto elmt = actionID.find(action_label);
			    if (elmt == actionID.end()) {
        			this->actionID.insert(std::pair<uint64_t, const TPG::TPGVertex&>(action_label, this->tpg.addNewAction(action_label)));
				}
				//get the action number.
				pos2 =  this->lastLine.find(" [");
				pos1 = this->lastLine.find("A") + 1;
				action_number = std::stoi(this->lastLine.substr(pos1, pos2-pos1));
				std::cout << action_number << std::endl;
				this->actionLabel.insert(std::pair<uint64_t, uint64_t>(action_number,action_label));

				readLineFromFile();											// get the team/program links 
				//get the team index : 
				pos2 = this->lastLine.find(" ");
				pos1 = this->lastLine.find("T") + 1;
				team_idx = std::stoi(this->lastLine.substr(pos1, pos2-pos1));

				//actual linking of the team -> program -> action
				auto team_it = teamLabel.find(team_idx);
				const TPG::TPGVertex& team = vertexID.at(team_it->second);
				const TPG::TPGVertex& action  = actionID.at(action_label);
				this->tpg.addNewEdge(team, action, p);

			}
			else															// we found a program -> team link
			{
				// team from
				pos1 = this->lastLine.find("T")+1;
				pos2 = this->lastLine.find(" ");
				team_idx = std::stoi(this->lastLine.substr(pos1, pos2-pos1));

				//delete frst part of string
				pos1 = this->lastLine.find("> ")+2;
				this->lastLine = this->lastLine.substr(pos1, this->lastLine.size());

				// which program
				pos1 = this->lastLine.find("P")+1;
				pos2 = this->lastLine.find(" ");
				prog_id = std::stoi(this->lastLine.substr(pos1, pos2-pos1));

				//team to
				pos1 = this->lastLine.rfind("T")+1;
				pos2 = this->lastLine.rfind("\n");
				out_team_idx = std::stoi(this->lastLine.substr(pos1, pos2-pos1));

				auto team_it = teamLabel.find(team_idx);
				const TPG::TPGVertex& team_in = vertexID.at(team_it->second);				
				team_it = teamLabel.find(out_team_idx);
				const TPG::TPGVertex& team_out = vertexID.at(team_it->second);
				auto prog_it = programID.find(prog_id);
				const std::shared_ptr<Program::Program> prog = prog_it->second;
				this->tpg.addNewEdge(team_in, team_out, prog);
			}
			bool teamlink = true;
			while(teamlink)
			{
				readLineFromFile();
				bool isaction;
				if(std::regex_match(this->lastLine,testTeamLink))		// link a team to another program
				{
					//get team index 
					pos1 = this->lastLine.find("T")+1;
					pos2 = this->lastLine.find(" ");
					team_idx = std::stoi(this->lastLine.substr(pos1, pos2-pos1));

					//delete frst part of string
					pos1 = this->lastLine.find("> ")+2;
					this->lastLine = this->lastLine.substr(pos1, this->lastLine.size());

					//get prog index : 
					pos1 = this->lastLine.find("P")+1;
					pos2 = this->lastLine.find(" ");
					prog_id = std::stoi(this->lastLine.substr(pos1, pos2-pos1));

					//dest index
					pos1 = std::string::npos;
					pos1 = this->lastLine.find("T");
					if(pos1 != std::string::npos)
					{
						pos1++;
						isaction = false;
					}
					else
					{
						pos1 = this->lastLine.find("A")+1;
						isaction = true;
					}
					

					pos2 = this->lastLine.rfind("\n");
					out_team_idx = std::stoi(this->lastLine.substr(pos1, pos2-pos1));
					
					auto team_it = teamLabel.find(team_idx);
					const TPG::TPGVertex& team_in = vertexID.at(team_it->second);
					auto prog_it = programID.find(prog_id);
					const std::shared_ptr<Program::Program> prog = prog_it->second;
					if(isaction)
					{
						auto action_it = actionLabel.find(out_team_idx);
						const TPG::TPGVertex& action  = actionID.at(action_it->second);
						this->tpg.addNewEdge(team_in, action, prog);
					}
					else
					{
						team_it = teamLabel.find(out_team_idx);
						const TPG::TPGVertex& team_out = vertexID.at(team_it->second);
						this->tpg.addNewEdge(team_in, team_out, prog);
					}
				}
				else
				{
					teamlink = false;
				}
			}
		}
		else
		{
			cont = false;
		}
	}
}

void Importer::TPGGraphDotImporter::dumpTPGGraphHeader()
{
	//skips the three first lines of the file
	readLineFromFile();
	readLineFromFile();
	readLineFromFile();
}

void Importer::TPGGraphDotImporter::readNumberOfTeams()
{
	//team format is : T0 [fillcolor="#1199bb"]
	bool cont = true;
	std::regex testTeamDeclare(this->teamRegex);
	uint64_t index = 0;
	size_t pos1, pos2;
	uint64_t team_idx;

	while (cont)
	{		
		readLineFromFile();
		if(std::regex_match(this->lastLine,testTeamDeclare))
		{
			pos1 = this->lastLine.find("T")+1;
			pos2 = this->lastLine.find(" ");
			team_idx = std::stoi(this->lastLine.substr(pos1, pos2-pos1));
			this->vertexID.insert(std::pair<uint64_t, const TPG::TPGVertex&>(index, this->tpg.addNewTeam()));
			this->teamLabel.insert(std::pair<uint64_t, uint64_t>(team_idx, index++));
		}
		else
		{
			cont = false;
		}
	}
}

TPG::TPGGraph& Importer::TPGGraphDotImporter::importGraph()
{
	// skip header
	this->dumpTPGGraphHeader();
	//create a list of teams
	this->readNumberOfTeams();
	//read the programs and link them with their edges
	this->readPrograms();

	fflush(pFile);
	return this->tpg;
}


void Importer::TPGGraphDotImporter::readLineFromFile()
{
	ssize_t read_status = 0;
	size_t line_buf_size = 0;
	char * buffer;
	read_status = getline(&buffer,&line_buf_size,pFile);

	if (read_status <= 0)
		throw std::ifstream::failure("Couldn't read in the given file");
	else
	{
		this->lastLine = buffer;
	}
}