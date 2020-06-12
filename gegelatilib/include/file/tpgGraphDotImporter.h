/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2020)
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

#ifndef TPG_GRAPH_DOT_IMPORTER_H
#define TPG_GRAPH_DOT_IMPORTER_H

#include <stdexcept>
#include <map>
#include <string>
#include <memory>
#include <inttypes.h>
#include <regex>
#include <fstream>
#include <cstdio>

#include "learn/learningEnvironment.h"
#include "tpg/tpgVertex.h"
#include "tpg/tpgTeam.h"
#include "tpg/tpgAction.h"
#include "tpg/tpgEdge.h"
#include "tpg/tpgGraph.h"

namespace File {
	/**
	* \brief Class used to import a TPG graph from a dot file.
	* It should be able to import a whole Learning agent object.
	*/
	class TPGGraphDotImporter {
	protected:
		/**
		* \brief File in which the dot content is read during import.
		*/
		std::ifstream pFile;

		/**
		* \brief last Line read from file 
		*
		* helps to pass a line that didn't match any regex to another function
		*/
		std::string lastLine;

		/**
		* \brief The environment in which the TPGGRAPH will be built
		*/
		Environment env;

		/**
		* \brief TPGGraph imported from dot file.
		*/
		TPG::TPGGraph & tpg;

		/**
		* \brief Map associating pointers to TPGVertex to an integer ID.
		*
		* This map is used to associate an unique id to a tpg vertex and to 
		* keep track of the pointers while restoring the TPGGraph described in a dot file
		*/
		std::map<uint64_t, const TPG::TPGVertex*> vertexID;

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
		* more than once.
		*/
		std::map<uint64_t, const TPG::TPGVertex*> actionID;

		/**
		* \brief Map associating actions to the corresponding action ID
		*
		* This map is here is used to access the correct TPGVertex while linking an action.
		*/
		std::map<uint64_t, uint64_t> actionLabel;

		/**
		* \brief string used to spot the end of a line in the program description.
		*/
		static const std::string lineSeparator;

		/**
		* \brief Contains the regex to identify a team declaration
		*
		* this regex values "T([0-9]+)\\x20\\x5B.*\\x5D"
		*
		* Explanation :
		*
		* T([0-9]+)     looks for a T followed by a number. the number will be stored in a group
		* \\x20\\x5B    looks for a succession of a whitespace and an opening bracket ('[')
		* .*\\x5D		the following can be any sequence of character terminated by ]
		*
		* Example:
		* T10 [fillcolor="#1199bb"]				Should pass
		* P0 [fillcolor="#cccccc" shape=point]	Should not pass
		*/
		static const std::string teamRegex;

		/**
		* \brief Contains the regex to identify a program declaration
		*
		* this regex values "P([0-9]+)\\x20\\x5B.*\\x5D"
		*
		* Explanation :
		*
		* P([0-9]+)     looks for a P followed by a number. the number will be stored in a group
		* \\x20\\x5B    looks for a succession of a whitespace and an opening bracket ('[')
		* .*\\x5D		the following can be any sequence of character terminated by ]
		*
		* Example:
		* P0 [fillcolor="#cccccc" shape=point]	Should pass
		* T10 [fillcolor="#1199bb"]				Should not pass
		*/
		static const std::string programRegex;

		/**
		* \brief contains the regex to identify an instruction declaration
		*
		* this regex values "I([0-9]+)\\x20\\x5B.*label=\"(.*)\"\\x5D"
		*
		* Explanation :
		*
		* I([0-9]+)     looks for a I followed by a number. the number will be stored in a group
		* \\x20\\x5B    looks for a succession of a whitespace and an opening bracket ('[')
		* .*			the following can be any sequence of character
		* label=\"	    looks for the label declaration sequence.
		* (.*)\"\\x5D	stores the content of the label in a group and  look for the ending sequence : "]
		*
		* Example:
		* I0 [shape=box style=invis]			Should pass
		* P0 [fillcolor="#cccccc" shape=point]	Should not pass
		*/
		static const std::string instructionRegex;

		/**
		* \brief contains the regex to identify an action declaration
		*
		* this regex values "A([0-9]+)\\x20\\x5B.*=\"([0-9]+)\"\\x5D"
		*
		* Explanation :
		*
		* A([0-9]+)     looks for a A followed by a number. the number will be stored in a group
		* \\x20\\x5B    looks for a succession of a whitespace and an opening bracket ('[')
		* .*=\"	        the following can be any sequence of character ending with ="		  
		* ([0-9]+)	    looks for a number and stores it into the next group
		* \"\\x5D 		the end of the sequence is made of "]
		*
		* Example:
		* A0 [shape=box style=invis]			Should pass
		* P0 [fillcolor="#cccccc" shape=point]	Should not pass
		*/
		static const std::string actionRegex;

		/**
		* \brief contains the regex to identify a Program -> Instruction Link
		*
		* this regex values "P([0-9]+)\\x20->\\x20I([0-9]+).*"
		*
		* Explanation :
		*
		* P[0-9]+       looks for a P followed by a number. the number will be stored in a group
		* \\x20	        looks for a whitespace 
		* ->			looxs for the sequence '->'
		* \\x20 	    looks for a whitespace 
		* I[0-9]+       looks for a I followed by a number. the number will be stored in a group
		* .*			the following can be any sequence of character
		*
		* Example:
		* P22 -> I22[style=invis]			Should pass
		* T0 -> P22 -> A11					Should not pass
		*/
		static const std::string linkProgramInstructionRegex;

		/**
		* \brief contains the regex to identify a Team -> Program -> Action Link
		*
		* this regex values "T([0-9]+)\\x20->\\x20P([0-9]+)\\x20->\\x20A([0-9]+).*"
		*
		* Explanation :
		*
		* T[0-9]+       looks for a T followed by a number. the number will be stored in a group
		* \\x20	        looks for a whitespace 
		* ->			looxs for the sequence '->'
		* \\x20	        looks for a whitespace 
		* P[0-9]+       looks for a P followed by a number. the number will be stored in a group
		* \\x20	        looks for a whitespace 
		* ->			looxs for the sequence '->'
		* \\x20	        looks for a whitespace 
		* A[0-9]+       looks for a A followed by a number. the number will be stored in a group
		* .*			the following can be any sequence of character
		*
		* Example:
		* P22 -> I22[style=invis]			Should not pass
		* T0 -> P22 -> A11					Should pass
		*/
		static const std::string linkProgramActionRegex;

		/**
		* \brief contains the regex to identify a Team -> Program -> Team Link
		*
		* this regex values "T([0-9]+)\\x20->\\x20P([0-9]+)\\x20->\\x20T([0-9]+).*"
		*
		* Explanation :
		*
		* T[0-9]+       looks for a T followed by a number. the number will be stored in a group
		* \\x20	        looks for a whitespace 
		* ->			looxs for the sequence '->'
		* \\x20	        looks for a whitespace 
		* P[0-9]+       looks for a P followed by a number. the number will be stored in a group
		* \\x20	        looks for a whitespace 
		* ->			looxs for the sequence '->'
		* \\x20	        looks for a whitespace 
		* T[0-9]+       looks for a T followed by a number. the number will be stored in a group
		* *			    the following can be any sequence of character
		*
		* Example:
		* P22 -> I22[style=invis]			Should not pass
		* T0 -> P22 -> T11					Should pass
		*/
		static const std::string linkProgramTeamRegex;

		/**
		* \brief contains the regex to identify a Team -> Program Link
		* the outgoing program vertex must already have been linked
		*
		* this regex values "T([0-9]+)\\x20->\\x20P([0-9]+)"
		*
		* Explanation :
		*
		* T[0-9]+       looks for a T followed by a number. the number will be stored in a group
		* \\x20	        looks for a whitespace 
		* ->			looxs for the sequence '->'
		* \\x20	        looks for a whitespace 
		* P[0-9]+       looks for a P followed by a number. the number will be stored in a group
		*
		* Example:
		* P22 -> I22[style=invis]			Should not pass
		* T0 -> P22 -> T11					Should pass
		*/
		static const std::string addLinkProgramRegex;
		
		/**
		* \brief Reads the content of the parameters in a string and puts it in the line
		* passed in parameter
		*
		* \param[in] str the string to parse
		* \param[in] line the line to fill with the parsed informations
		*/ 
		void readParameters(std::string & str, Program::Line & line);

		/**
		* \brief Reads the content of the operands and puts it in the line
		* passed in parameter
		*
		* \param[in] str the string to parse
		* \param[in] line the line to fill with the parsed informations
		*/ 
		void readOperands(std::string & str, Program::Line & line);

		/**
		* \brief Reads the content of a line and puts it in the line
		* passed in parameter
		*/ 
		void readLine(std::smatch & matches);

		/**
		* \brief Create a program from its dot content.
		*/
		void readProgram(std::smatch & matches);

		/**
		* \brief dumps the header of the dot file
		*
		* The header of the dot file contains no relevant informations to build the model
		*/
		void dumpTPGGraphHeader();

		/**
		* \brief reads and creates a TPGTeam.
		*/
		void readTeam(std::smatch & matches);

		/**
		* \brief reads and creates a TPGAction.
		*/
		void readAction(std::smatch & matches);

		/**
		* \brief reads a link declaration and creates a team to action edge
		*/
		void readLinkTeamProgramAction(std::smatch & matches);

		/**
		* \brief reads a link declaration and creates a team to team edge
		*/
		void readLinkTeamProgramTeam(std::smatch & matches);

		/**
		* \brief reads a link declaration and creates a team to program's destination edge.
		*/
		void readLinkTeamProgram(std::smatch & matches);


		/**
		*	\brief reads a single line of the file
		*   
		*	\return true if the line read matched any of the line characteristics specified as regexs.
		*/
		bool readLineFromFile();


	public:
		/**
		* \brief Constructor for the importer.
		*
		* \param[in] filePath initial path to the file where the dot content
		* will be written.
		* \param[in] environment the environment in which the tpg Graph should be built
		* \param[in] tpgref a Reference to the TPGGraph to buiuld from the .dot file
		* \throws std::runtime_error in case no file could be opened at the
		* given filePath.
		*/
		TPGGraphDotImporter(const char* filePath, Environment environment, TPG::TPGGraph & tpgref) : 
			env{environment}, 
			tpg{tpgref}
		{
			pFile.open(filePath);
			if (!pFile.is_open()) {
				throw std::runtime_error("Could not open file " + std::string(filePath));
			}
			importGraph();
		};

		/**
		* \brief Maximum number of characters that can be read in a single line.
		*/
		static const unsigned int MAX_READ_SIZE=1024;

		/**
		* Destructor for the importer.
		*
		* Closes the file.
		*/
		~TPGGraphDotImporter() {
			if (pFile.is_open()) {
				pFile.close();
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
		void setNewFilePath(const char* newFilePath);

		/**
		* \brief Creates a TPG Graph from its description in a .dot file
		*/
		void importGraph();
	};
};

#endif
