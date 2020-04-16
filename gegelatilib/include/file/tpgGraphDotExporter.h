#ifndef TPG_GRAPH_DOT_EXPORTER_H
#define TPG_GRAPH_DOT_EXPORTER_H

#include <stdexcept>
#include <map>
#include <string>

#include "tpg/tpgVertex.h"
#include "tpg/tpgTeam.h"
#include "tpg/tpgAction.h"
#include "tpg/tpgEdge.h"
#include "tpg/tpgGraph.h"
#include "program/program.h"
#include "program/line.h"

namespace File {
	/**
	* \brief Class used to export a TPGGraph into a text file with the dot
	* format.
	*/
	class TPGGraphDotExporter {
	protected:
		/**
		* \brief File in which the dot content is written during export.
		*/
		FILE* pFile;

		/**
		* \brief Character chain used to control the indentation of the exported
		* file.
		*/
		std::string offset;

		/**
		* \brief Reference to the TPGGraph exported into dot.
		*/
		const TPG::TPGGraph& tpg;

		/**
		* \brief Map associating pointers to TPGVertex to an integer ID.
		*
		* In case the TPGGraphDotExporter is used to export multiple TPGGraph,
		* this map is used to ensure that a given TPGVertex will always be
		* associated to the same integer identifier in all exported dot files.
		*/
		std::map<const TPG::TPGVertex*, uint64_t> vertexID;

		/**
		* \brief Map associating pointers to Program to an integer ID.
		*
		* In case the TPGGraphDotExporter is used to export multiple TPGGraph,
		* this map is used to ensure that a given Program will always be
		* associated to the same integer identifier in all exported dot files.
		*/
		std::map<const Program::Program*, uint64_t> programID;

		/**
		* \brief Integer number used during export to associate a unique
		* integer identifier to each new TPGTeam.
		*
		* Using the VertexID map, a TPGTeam that was already printed in
		* previous export will keep its ID.
		*/
		uint64_t nbVertex = 0;

		/**
		* \brief Integer number used during export to associate a unique
		* integer identifier to each new Program.
		*
		* Using the programID map, a Program that was already printed in
		* previous export will keep its ID.
		*/
		uint64_t nbPrograms = 0;

		/**
		* \brief Integer number used during export to associate a unique
		* integer identifier to each TPGAction.
		*
		* Identifier associated to TPGAction are NOT preserved during multiple
		* printing of a TPGGraph.
		*/
		uint64_t nbActions;

		/**
		* \brief Method for finding the unique identifier associated to a given
		* TPGVertex.
		*
		* Using the vertexID map, this method returns the integer identifier
		* associated to the given TPGVertex. If not identifier exists for this
		* TPGVertex, a new one is created automatically and saved into the map.
		*
		* \param[in] vertex a const reference to the TPGVertex whose integer
		*                    identifier is retrieved.
		* \return the integer identifier for the given TPGVertex.
		*/
		uint64_t findVertexID(const TPG::TPGVertex& vertex);

		/**
		* \brief Method for finding the unique identifier associated to a given
		* Program.
		*
		* Using the programID map, this method retrieves the integer identifier
		* associated to the given Program. If not identifier exists for this
		* Program, a new one is created automatically and saved into the map.
		*
		* \param[in] prog a const reference to the Program whose integer
		*                    identifier is retrieved.
		* \param[out] id a pointer to an integer number, used to return the
		*                found identifier.
		* \return A boolean value indicating whether the returned ID is a new
		* one (true), or one found in the programID map (false).
		*/
		bool findProgramID(const Program::Program& prog, uint64_t& id);

		/**
		* \brief Print the dot content for the given TPGTeam.
		*
		* Content is printed directly into the file opened by the class
		* constructor, or by a call to setNewFilePath.
		*
		* \param[in] team the TPGTeam being printed.
		*/
		void printTPGTeam(const TPG::TPGTeam& team);


		/**
		* \brief Print the dot content for the given TPGAction.
		*
		* Content is printed directly into the file opened by the class
		* constructor, or by a call to setNewFilePath.
		* This method returns the identifier associated to the printed action
		* so that the print TPGEdge method can target this TPGAction. Indeed,
		* contrary to TPGTeam which have a unique ID, each action is printed on
		* the fly, with a unique ID, when a TPGEdge is targetting this action.
		*
		* \param[in] action the TPGTeam being printed.
		* \return the identifier associated to this action.
		*/
		uint64_t printTPGAction(const TPG::TPGAction& action);

		/**
		* \brief Prints the dot content for the given TPGAction.
		*
		* \param[in] edge the TPGEdge being printed.
		*/
		void printTPGEdge(const TPG::TPGEdge& edge);

		/**
		* \brief Prints the dot content for the given Program.
		*
		* \param[in] program the Program to be printed
		*
		* a program is stored in the .dot file with the format :
		* line_1&#92;nline2&#92;nline3&#92;n...&#92;nline_N&#92;n
		*
		* a line is stored in the .dot file with the following format
		* inst_idx|dest_idx&param_1|param_2|...|param_n$op1_param1|op1_param2#...#
		*
		* inst_idx = instruction index
		* dest_idx = destination index
		* param    = parameter
		* op       = operand
		*/
		void printProgram(const Program::Program& program);

		/**
		* \brief Prints header content in the dot file.
		*
		* This method prints preliminary content that must be printed into the
		* dot file before any vertex or edge.
		*/
		void printTPGGraphHeader();

		/**
		* \brief Prints footer content in the dot file.
		*
		* This method prints finalization content that must be printed into the
		* dot file after all vertices and edges.
		*/
		void printTPGGraphFooter();

	public:
		/**
		* \brief Constructor for the exporter.
		*
		* \param[in] filePath initial path to the file where the dot content
		* will be written.
		* \param[in] graph const reference to the graph whose content will
		* be exported in dot.
		* \throws std::runtime_error in case no file could be opened at the
		* given filePath.
		*/
		TPGGraphDotExporter(const char* filePath, const TPG::TPGGraph& graph) : pFile{ NULL }, tpg{ graph }, offset{ "" }, nbActions{ 0 } {
			if ((pFile = fopen(filePath, "w")) == NULL) {
				throw std::runtime_error("Could not open file " + std::string(filePath));
			}
		};

		/**
		* Disable copy construction.
		*
		* Until we see the need for it, there si no reason to enable copy-construction
		* of TPGGraphDotExporter.
		*/
		TPGGraphDotExporter(const TPGGraphDotExporter& other) = delete;

		/**
		* Disable TPGGraphDotExporter default assignment operator.
		*
		* Until we see the need for it, there si no reason to enable assignment
		* operator of TPGGraphDotExporter.
		*/
		TPGGraphDotExporter& operator=(const TPGGraphDotExporter& other) = delete;

		/**
		* Destructor for the exporter.
		*
		* Closes the file.
		*/
		~TPGGraphDotExporter() {
			if (pFile != NULL) {
				fclose(pFile);
			}
		}

		/**
		* \brief Set a new file for the exporter.
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
			if ((pFile = fopen(newFilePath, "w")) == NULL) {
				pFile = NULL;
				throw std::runtime_error("Could not open file " + std::string(newFilePath));
			}
		}

		/**
		* \brief Print the TPGGraph given when constructing the
		* TPGGraphDotExporter into a dot file.
		*/
		void print();
	};
};

#endif