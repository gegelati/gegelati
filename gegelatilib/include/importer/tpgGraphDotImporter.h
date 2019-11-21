#ifndef TPG_GRAPH_DOT_IMPORTER_H
#define TPG_GRAPH_DOT_IMPORTER_H

#include <stdexcept>
#include <map>
#include <string>
#include <memory>
#include <inttypes.h>
#include <regex>
#include <fstream>

#include "learn/learningEnvironment.h"
#include "tpg/tpgVertex.h"
#include "tpg/tpgTeam.h"
#include "tpg/tpgAction.h"
#include "tpg/tpgEdge.h"
#include "tpg/tpgGraph.h"

namespace Importer {
	/**
	* \brief Class used to import a TPG graph from a dot file.
	* It should be able to import a whole Learning agent object.
	*/
	class TPGGraphDotImporter {
	friend TPG::TPGGraph;
	protected:
		/**
		* \brief File in which the dot content is read during import.
		*/
		FILE* pFile;

		/**
		* \brief last Line read from file 
		*
		*	helps to pass a line that didn't match any regex to another function
		*/
		std::string lastLine;;

		/**
		* \brief The environment in which the TPGGRAPH will be built
		*/
		Environment env;

		/**
		* \brief TPGGraph imported from dot file.
		*/
		TPG::TPGGraph tpg;

		/**
		* \brief Map associating pointers to TPGVertex to an integer ID.
		*
		* This map is used to associate an unique id to a tpg vertex and to 
		* keep track of the pointers while restoring the TPGGraph described in a dot file
		*/
		std::map<uint64_t, const TPG::TPGVertex&> vertexID;

		/**
		* \brief Map associating pointers to Program to an integer ID.
		*
		* This map is used to associate an unique id to a tpg program and to 
		* keep track of the pointers while restoring the TPGGraph described in a dot file
		*/
		std::map<uint64_t, std::shared_ptr<Program::Program>> programID;
		
		/**
		* \brief Map associating pointers to TPGVertex representing actions
		* to the corresponding action 
		*
		* This map is used to ensure that identical actions are not created
		* moore than once.
		*/
		std::map<uint64_t, const TPG::TPGVertex&> actionID;

		/**
		* \brief Map associating actions to the corresponding action ID
		*
		* This map is here is used to access the correct TPGVertex while linking an action.
		*/
		std::map<uint64_t, uint64_t> actionLabel;

		/**
		* \brief Map associating actions to the corresponding team ID
		*
		* This map is here is used to access the correct TPGVertex while linking a team.
		*/
		std::map<uint64_t, uint64_t> teamLabel;

		/**
		* \brief Integer number used during import to associate a unique
		* integer identifier to each TPGAction.
		*
		* Identifier associated to TPGAction are NOT preserved during multiple
		* reading of a TPGGraph.
		*/
		uint64_t nbActions;

		/**
		*	\brief string used to spot the end of a line in the program description.

		*/
		const std::string lineSeparator;

		/**
		*	\brief contains the regex to identify a team declaration
		*
		*	this regex values "\t\tT[0-9]+.*\n"
		*
		*	Explanation :
		*
		*	\t\t     looks for two consecutives tabulations
		*	T[0-9]+  looks for the Letter T followed by 1 or more digits
		*	.*\n	 indicates that there will be several characters before the end of the line
		*
		*   Example:
		*	T10 [fillcolor="#1199bb"]				Should pass
		*	P0 [fillcolor="#cccccc" shape=point]	Should not pass
		*/
		const std::string teamRegex;

		/**
		*	\brief contains the regex to identify a program declaration
		*
		*	this regex values "\t\tP[0-9]+.*\n"
		*
		*	Explanation :
		*
		*	\t\t     looks for two consecutives tabulations
		*	P[0-9]+  looks for the Letter P followed by 1 or more digits
		*	.*\n	 indicates that there will be several characters before the end of the line
		*
		*   Example:
		*	P0 [fillcolor="#cccccc" shape=point]	Should pass
		*	T10 [fillcolor="#1199bb"]				Should not pass
		*/
		const std::string programRegex;

		/**
		*	\brief contains the regex to identify an instruction declaration
		*
		*	this regex values "\t\tI[0-9]+.*\n"
		*
		*	Explanation :
		*
		*	\t\t     looks for two consecutives tabulations
		*	I[0-9]+  looks for the Letter I followed by 1 or more digits
		*	.*\n	 indicates that there will be several characters before the end of the line
		*
		*   Example:
		*	I0 [shape=box style=invis]				Should pass
		*	P0 [fillcolor="#cccccc" shape=point]	Should not pass
		*/
		const std::string instructionRegex;

		/**
		*	\brief contains the regex to identify an action declaration
		*
		*	this regex values "\t\tA[0-9]+.*\n"
		*
		*	Explanation :
		*
		*	\t\t     looks for two consecutives tabulations
		*	A[0-9]+  looks for the Letter A followed by 1 or more digits
		*	.*\n	 indicates that there will be several characters before the end of the line
		*
		*   Example:
		*	A0 [shape=box style=invis]				Should pass
		*	P0 [fillcolor="#cccccc" shape=point]	Should not pass
		*/
		const std::string actionRegex;


		/**
		* \brief Reads the content of the parameters in a string and puts it in the line
		* passed in parameter
		*
		* \param[in] str the string to parse
		*
		* \param[in] line the line to fill with the parsed informations
		*/ 
		void readParameters(std::string & str, Program::Line & line);

		/**
		* \brief Reads the content of the operands and puts it in the line
		* passed in parameter
		*
		* \param[in] str the string to parse
		*
		* \param[in] line the line to fill with the parsed informations
		*/ 
		void readOperands(std::string & str, Program::Line & line);

		/**
		* \brief Reads the content of a line and puts it in the line
		* passed in parameter
		*
		* \param[in] str the string to parse
		*
		* \param[in] line the line to fill with the parsed informations
		*/ 
		void readLine(std::string & str, Program::Line & line);


		/**
		* \brief Reads the content of a program and puts it in the program
		* passed in parameter
		*
		* \param[in] program the program to fill with the read instructions
		*/
		void fillWithLines(std::shared_ptr<Program::Program> & program);


		/**
		* \brief Create a program from its dot content.
		*/
		void readPrograms();

		/**
		* \brief dumps the header of the dot file
		*
		* The header of the dot file contains no relevant informations to build the model
		*/
		void dumpTPGGraphHeader();

		/**
		* \brief create as much teams as there are in the file
		*/
		void readNumberOfTeams();

		/**
		*	\brief reads a single line of the file
		*/
		void readLineFromFile();


	public:
		/**
		* \brief Constructor for the importer.
		*
		* \param[in] filePath initial path to the file where the dot content
		* will be written.
		* \param[in] graph const reference to the graph whose content will
		* be imported in dot.
		* \throws std::runtime_error in case no file could be opened at the
		* given filePath.
		*/
		TPGGraphDotImporter(const char* filePath, Learn::LearningEnvironment& le, const Instructions::Set& iSet, const unsigned int nbRegs = 8) : 
			pFile{ NULL }, 
			env{iSet, le.getDataSources(), nbRegs}, 
			tpg(env), 
			nbActions{ 0 }, 
			lineSeparator("&#92;n"),
			teamRegex("\t\tT[0-9]+.*\n"), 
			programRegex("\t\tP[0-9]+.*\n"), 
			instructionRegex("\t\tI[0-9]+.*\n"),
			actionRegex("\t\tA[0-9]+.*\n")
		{
			if ((pFile = fopen(filePath, "r")) == NULL) {
				throw std::runtime_error("Could not open file " + std::string(filePath));
			}
		};

		/**
		* Destructor for the importer.
		*
		* Closes the file.
		*/
		~TPGGraphDotImporter() {
			if (pFile != NULL) {
				fclose(pFile);
			}
		}

		/**
		* \brief Set a new file for the importer.
		*
		* \param[in] newFilePath new path to the file where the dot content
		* will be written.
		* \throws std::runtime_error in case no file could be opened at the
		* given newFilePath.
		*/
		void setNewFilePath(const char* newFilePath) {
			//  Close previous file
			fclose(pFile);

			// open new one;
			if ((pFile = fopen(newFilePath, "r")) == NULL) {
				pFile = NULL;
				throw std::runtime_error("Could not open file " + std::string(newFilePath));
			}
		}

		/**
		* \brief Print the TPGGraph given when constructing the
		* TPGGraphImporter into a dot file.
		*/
		TPG::TPGGraph& importGraph();
	};
};

#endif