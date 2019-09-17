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

namespace Exporter {
	class TPGGraphDotExporter {
	protected:
		FILE* pFile;

		std::string offset;

		const TPG::TPGGraph& tpg;

		std::map<const TPG::TPGVertex*, uint64_t> vertexID;

		uint64_t findVertexID(const TPG::TPGVertex&);

		void printTPGVertex(const TPG::TPGVertex& vertex);

		void printTPGTeam(const TPG::TPGTeam& team);

		void printTPGAction(const TPG::TPGAction& action);

		void printTPGEdge(const TPG::TPGEdge& edge);

		void printTPGGraphHeader();

		void printTPGGraphFooter();

	public:
		/**
		* \brief Constructor for the exporter.
		*/
		TPGGraphDotExporter(const char* filePath, const TPG::TPGGraph& graph) : pFile{ NULL }, tpg{ graph }, offset{ "" } {
			if ((pFile = fopen(filePath, "w")) == NULL) {
				throw std::runtime_error("Could not open file " + std::string(filePath));
			}
		};

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

		void setNewFilePath(const char* newFilePath) {
			//  Close previous file
			fclose(pFile);

			// open new one;
			if ((pFile = fopen(newFilePath, "w")) == NULL) {
				pFile = NULL;
				throw std::runtime_error("Could not open file " + std::string(newFilePath));
			}
		}

		void print();
	};
};

#endif