/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2020 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
 * Pierre-Yves Le Rolland-Raumer <plerolla@insa-rennes.fr> (2020)
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

#ifndef GRAPH_DOT_IMPORTER_H
#define GRAPH_DOT_IMPORTER_H

#include <cstdio>
#include <fstream>
#include <inttypes.h>
#include <map>
#include <memory>
#include <regex>
#include <stdexcept>
#include <string>

#include "algorithm/lgp/lgpAgent.h"
#include "learn/learningEnvironment.h"
#include "evoGraph/action.h"
#include "evoGraph/edge.h"
#include "evoGraph/graph.h"
#include "evoGraph/team.h"
#include "evoGraph/vertex.h"
#include "util/counterReset.h"

namespace File {
    /**
     * \brief Class used to import a graph from a dot file.
     * It should be able to import a whole Learning agent object.
     */
    class GraphDotImporter
    {
      protected:

        /**
         * Get an algorithm from the algorithm ID
         */
        Algorithm::Algorithm& getAlgorithm(uint64_t algorithmID);
        /**
         * \brief set the map algorithms used in the attribute vector
         */
        void setMapAlgorithm();

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
         * \brief Graph imported from dot file.
         */
        EvoGraph::Graph& graph;

        /// @brief algorithm containing the agents.
        std::vector<std::reference_wrapper<Algorithm::Algorithm>> algorithms;

        /// @brief vector of algorithms used, including subAlgorithms. This is used to read the content of the programs of sub algorithms.
        std::map<uint64_t, std::reference_wrapper<Algorithm::Algorithm>> mapAlgorithms;

        
        /// @brief map of printed vertex. This is used to avoid printing twice the same vertex in case of multiple edges pointing toward it.
        std::map<uint64_t, std::reference_wrapper<const EvoGraph::Vertex>> readVertexID;

        /// @brief map of printed edge. This is used to avoid printing twice the same edge in case of multiple edges pointing toward the same destination vertex.
        std::map<uint64_t, std::reference_wrapper<const EvoGraph::Edge>> readEdgeID;

        /// @brief map of printed agent. This is used to avoid printing twice the same agent in case of multiple vertices or edges using the same agent agent.
        std::map<uint64_t, std::reference_wrapper<const Algorithm::Agent>> readAgentID;


        
        /**
         * \brief Contains the regex to identify an algorithm declaration
         * 
         * Should work with ALGO10 [.... label="Name.ID"]
         */
        static const std::string algorithmRegex;

        /**
         * \brief Contains the regex to identify an aggregated algorithm link declaration
         * 
         * Should work with ALGO10 -> ALGO12 [style=dashed]
         */
        static const std::string aggregatedAlgorithmLinkRegex;

        /**
         * \brief Contains the regex to identify a sub algorithm link declaration
         * 
         * Should work with ALGO10 -> ALGO12
         */
        static const std::string subAlgorithmLinkRegex;


        /**
         * \brief Contains the regex to identify the end of the sub graph of algorithms
         * 
         * should only work with "}"
         */
        static const std::string endAlgorithmSubGraph;



        /**
         * \brief Contains the regex to identify a team declaration
         *
         * this regex values "T([0-9]+)\\x20\\x5B.*\\x5D"
         *
         * Explanation :
         *
         * T([0-9]+)     looks for a T followed by a number. the number will be
         * stored in a group
         * \\x20\\x5B    looks for a succession of a whitespace and an opening
         * bracket ('[')
         * .*\\x5D		the following can be any sequence of character
         * terminated by
         * ]
         *
         * Example:
         * T10 [fillcolor="#1199bb"]				Should pass
         * P0 [fillcolor="#22cccc" shape=point]	Should not pass
         */
        static const std::string teamRegex;

        /**
         * \brief Contains the regex to identify a agent declaration
         *
         * this regex values "P([0-9]+)\\x20\\x5B.*\\x5D"
         *
         * Explanation :
         *
         * P([0-9]+)     looks for a P followed by a number. the number will be
         * stored in a group
         * \\x20\\x5B    looks for a succession of a whitespace and an opening
         * bracket ('[')
         * .*\\x5D		the following can be any sequence of character
         * terminated by
         * ]
         *
         * Example:
         * P0 [fillcolor="#22cccc" shape=point]	Should pass
         * T10 [fillcolor="#1199bb"]				Should not pass
         */
        static const std::string agentRegex;

        /**
         * \brief contains the regex to identify an action declaration
         *
         * this regex values "A([0-9]+)\\x20\\x5B.*=\"([0-9]+)\"\\x5D"
         *
         * Explanation :
         *
         * A([0-9]+)     looks for a A followed by a number. the number will be
         * stored in a group
         * \\x20\\x5B    looks for a succession of a whitespace and an opening
         * bracket ('[')
         * .*=\"	        the following can be any sequence of character
         * ending with ="
         * ([0-9]+)	    looks for a number and stores it into the next group
         * \"\\x5D 		the end of the sequence is made of "]
         *
         * Example:
         * A0 [shape=box style=invis]			Should pass
         * P0 [fillcolor="#22cccc" shape=point]	Should not pass
         */
        static const std::string actionRegex;

        /**
         * \brief contains the regex to identify a Team -> Agent -> Action
         * Link
         *
         * this regex values
         * "T([0-9]+)\\x20->\\x20P([0-9]+)\\x20->\\x20A([0-9]+).*"
         *
         * Explanation :
         *
         * T[0-9]+       looks for a T followed by a number. the number will be
         * stored in a group
         * \\x20	        looks for a whitespace
         * ->			looxs for the sequence '->'
         * \\x20	        looks for a whitespace
         * P[0-9]+       looks for a P followed by a number. the number will be
         * stored in a group
         * \\x20	        looks for a whitespace
         * ->			looxs for the sequence '->'
         * \\x20	        looks for a whitespace
         * A[0-9]+       looks for a A followed by a number. the number will be
         * stored in a group
         * .*			the following can be any sequence of character
         *
         * Example:
         * P22 -> I22[style=invis]			Should not pass
         * T0 -> P22 -> A11					Should pass
         */
        static const std::string linkTeamAgentActionRegex;

        /**
         * \brief contains the regex to identify a Team -> Agent -> Team Link
         *
         * this regex values
         * "T([0-9]+)\\x20->\\x20P([0-9]+)\\x20->\\x20T([0-9]+).*"
         *
         * Explanation :
         *
         * T[0-9]+       looks for a T followed by a number. the number will be
         * stored in a group
         * \\x20	        looks for a whitespace
         * ->			looxs for the sequence '->'
         * \\x20	        looks for a whitespace
         * P[0-9]+       looks for a P followed by a number. the number will be
         * stored in a group
         * \\x20	        looks for a whitespace
         * ->			looxs for the sequence '->'
         * \\x20	        looks for a whitespace
         * T[0-9]+       looks for a T followed by a number. the number will be
         * stored in a group *			    the following can be any sequence of
         * character
         *
         * Example:
         * P22 -> I22[style=invis]			Should not pass
         * T0 -> P22 -> T11					Should pass
         */
        static const std::string linkTeamAgentTeamRegex;

        /**
         * \brief contains the regex to identify a Team -> Agent
         * Link
         */
        static const std::string linkTeamAgentRegex;

        /**
         * \brief contains the regex to identify a Team -> Agent Link
         */
        static const std::string linkAgentTeamRegex;


        /**
         * \brief reads the version of the dot file and returns it in the
         * parameters.
         *
         * \param[out] major the major version of the dot file
         * \param[out] minor the minor version of the dot file
         * \param[out] patch the patch version of the dot file
         * \return true if the version was found, false otherwise.
         */
        bool getExportedVersion(int& major, int& minor, int& patch);

        /**
         * \brief dumps the header of the dot file
         *
         * The header of the dot file contains no relevant informations to build
         * the model
         */
        void dumpGraphHeader();

        /**
         * \brief reads an algorithm and control its validity
         */
        void readAlgorithm(std::smatch matches);

        /**
         * \brief control that aggregated algorithm link exist
         */
        void readAggregatedAlgorithmLink(std::smatch matches);

        /**
         * \brief control that sub algorithm link exist
         */
        void readSubAlgorithmLink(std::smatch matches);

        /**
         * \brief read the algorithm subGraph
         */
        void readAlgorithmGraphSubGraph();

        /**
         * \brief reads and creates a Team.
         */
        void readTeam(std::smatch& matches);

        /**
         * \brief reads and creates a Action.
         */
        void readAction(std::smatch& matches);


        /**
         * \brief read and create an agent.
         */
        void readAgent(std::smatch& matches);

        /**
         * \brief reads a link declaration and creates a team to action edge
         */
        void readLinkTeamAgentAction(std::smatch& matches);

        /**
         * \brief reads a link declaration and creates a team to team edge
         */
        void readLinkTeamAgentTeam(std::smatch& matches);

        /**
         * \brief reads a link declaration and add the agent to the team's agent
         */
        void readLinkTeamAgent(std::smatch& matches);
        /**
         * \brief reads a link declaration and add the team to the agent's team. 
         * 
         * Will throw if the algorithm doesn't handle it.
         */
        void readLinkAgentTeam(std::smatch& matches);


        /**
         *	\brief reads a single line of the file
         *
         *	\return true if the line read matched any of the line
         *characteristics specified as regexs.
         */
        bool readLineFromFile();


      public:
        /// \brief The major version supported by the importer.
        static const int supportedMajorVersion = 2;
        /// \brief The minor version supported by the importer.
        static const int supportedMinorVersion = 0;
        /// \brief The patch version supported by the importer.
        static const int supportedPatchVersion = 0;

        /**
         * \brief Constructor for the importer.
         *
         * \param[in] filePath initial path to the file where the dot content
         * will be written.
         * \param[in] graphRef a Reference to the Graph to build from
         * the .dot file
         * \param[in] algorithms algorithm containing the agents.
         * \throws std::runtime_error in case no file could be
         * opened at the given filePath.
         */
        GraphDotImporter(const char* filePath, EvoGraph::Graph& graphRef, std::vector<std::reference_wrapper<Algorithm::Algorithm>> algorithms)
            : graph{graphRef}, algorithms{algorithms}
        {
            std::cout<<filePath<<std::endl;
            pFile.open(filePath);
            if (!pFile.is_open()) {
                throw std::runtime_error("Could not open file " +
                                         std::string(filePath));
            }
            importGraph();
        };

        /**
         * \brief Maximum number of characters that can be read in a single
         * line.
         */
        static const unsigned int MAX_READ_SIZE = 131072;

        /**
         * Destructor for the importer.
         *
         * Closes the file.
         */
        ~GraphDotImporter()
        {
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
         * \brief Creates a Graph from its description in a .dot file
         */
        void importGraph();
    };
}; // namespace File


#endif
