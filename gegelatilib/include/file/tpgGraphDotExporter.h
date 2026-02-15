/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2022) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
 * Thomas Bourgoin <tbourgoi@insa-rennes.fr> (2021)
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

#ifndef TPG_GRAPH_DOT_EXPORTER_H
#define TPG_GRAPH_DOT_EXPORTER_H

#include <map>
#include <stdexcept>
#include <string>

#include "algorithm/algorithm.h"
#include "evoGraph/abstractEngine.h"
#include "evoGraph/action.h"
#include "evoGraph/edge.h"
#include "evoGraph/graph.h"
#include "evoGraph/team.h"
#include "evoGraph/vertex.h"

namespace File {
    /**
     * \brief Class used to export a Graph into a text file with the dot
     * format.
     */
    class GraphDotExporter : EvoGraph::AbstractEngine
    {
      protected:
        /**
         * \brief File in which the dot content is written during export.
         */
        FILE* pFile;

        /**
         * \brief Character chain used to control the indentation of the
         * exported file.
         */
        std::string offset;

        /// @brief vector of algorithms used. This is used to print the content of the programs when they are mutated by the algorithm.
        const std::vector<std::shared_ptr<Algorithm::Algorithm>>& algorithmsRef;

        /// @brief vector of algorithms used, including subAlgorithms. This is used to print the content of the programs when they are mutated by the algorithm.
        std::vector<std::reference_wrapper<const Algorithm::Algorithm>> potentialAlgorithms;

        /// @brief set of printed vertex ID. This is used to avoid printing twice the same vertex in case of multiple edges pointing toward it.
        std::set<uint64_t> printedVertexID;

        /// @brief set of printed edge ID. This is used to avoid printing twice the same edge in case of multiple edges pointing toward the same destination vertex.
        std::set<uint64_t> printedEdgeID;

        /// @brief set of printed agent ID. This is used to avoid printing twice the same agent in case of multiple vertices or edges using the same agent program.
        std::set<uint64_t> printedAgentID;


        /**
         * \brief Print the dot content for the given Program.
         * 
         * This method find the corresponding algorithm to the agent program given and print it.
         * 
         * \param[in] agentProgram the agent program to be printed
         */
        void printAgent(const Algorithm::Agent& agentProgram);

        /**
         * \brief Print the dot content for the given Element.
         * 
         * Content is printed directly into the file opened by the class
         * constructor, or by a call to setNewFilePath.
         * 
         * This method will either call printVertex or printEdge depending on the type of the element given.
         * It will also print the potential agent program associated to the element.
         */
        void printElement(const EvoGraph::Element& element);


        /**
         * \brief Print the dot content for the given Vertex.
         * 
         * Content is printed directly into the file opened by the class
         * constructor, or by a call to setNewFilePath.
         * 
         * This method will either call printTeam or printAction depending on the type of the vertex given.
         * It will also print the potential agent program associated to the vertex.
         */
        void printVertex(const EvoGraph::Vertex& vertex);

        /**
         * \brief Print the dot content for the given Team.
         *
         * Content is printed directly into the file opened by the class
         * constructor, or by a call to setNewFilePath.
         *
         * \param[in] team the Team being printed.
         */
        void printTeam(const EvoGraph::Team& team);

        /**
         * \brief Print the dot content for the given Action.
         *
         * Content is printed directly into the file opened by the class
         * constructor, or by a call to setNewFilePath.
         * This method returns the identifier associated to the printed action
         * so that the print Edge method can target this Action. Indeed,
         * contrary to Team which have a unique ID, each action is printed on
         * the fly, with a unique ID, when a Edge is targetting this action.
         *
         * \param[in] action the Team being printed.
         */
        void printAction(const EvoGraph::Action& action);

        /**
         * \brief Prints the dot content for the given Edge.
         * Prints the program index followed by its contants if the constants
         * are used.
         *
         * the program is printed under the format :
         * Pindex [fillcolor=\#cccccc shape = point] //constant1|...|constant_n
         *
         * \param[in] edge the Edge being printed.
         */
        void printEdge(const EvoGraph::Edge& edge);

        /**
         * \brief Prints the dot content for the given Program.
         *
         * \param[in] lgpAgent the lgpAgent to be printed
         *
         * a lgpAgent is stored in the .dot file with the format :
         * line_1&#92;nline2&#92;nline3&#92;n...&#92;nline_N&#92;n
         *
         * a line is stored in the .dot file with the following format
         * inst_idx|dest_idx&op1_param1|op1_param2#...#
         *
         * inst_idx = instruction index
         * dest_idx = destination index
         * op       = operand
         */
        //void printLGPAgent(std::shared_ptr<const Algorithm::LGP::LGPAgent> lgpAgent);

        /**
         * \brief Prints header content in the dot file.
         *
         * This method prints preliminary content that must be printed into the
         * dot file before any vertex or edge.
         */
        void printGraphHeader();

        /**
         * \brief Prints footer content in the dot file.
         *
         * This method prints finalization content that must be printed into the
         * dot file after all vertices and edges.
         */
        void printGraphFooter();

      public:
        /**
         * \brief Constructor for the exporter.
         *
         * \param[in] filePath initial path to the file where the dot content
         * will be written.
         * \param[in] graph const reference to the graph whose content will
         * be exported in dot.
         * \param[in] algorithms vector of algorithms used in the graph. This is
         * used to print the content of the programs when they are mutated by
         * \throws std::runtime_error in case no file could be opened at the
         * given filePath.
         */
        GraphDotExporter(const char* filePath, const EvoGraph::Graph& graph, const std::vector<std::shared_ptr<Algorithm::Algorithm>>& algorithms)
            : EvoGraph::AbstractEngine(graph), pFile{NULL}, offset{""}, algorithmsRef{algorithms}
        {
            if ((pFile = fopen(filePath, "w")) == NULL) {
                throw std::runtime_error("Could not open file " +
                                         std::string(filePath));
            }

            // Add all algorithms to the set, and recursively all their sub-algorithms, to be able to print the content of the programs when they are mutated by the algorithm.
            std::vector<std::reference_wrapper<const Algorithm::Algorithm>> algorithmsToAdd;
            for(const std::shared_ptr<Algorithm::Algorithm>& algorithm : algorithms){
                algorithmsToAdd.push_back(*algorithm);
            }
            while(!algorithmsToAdd.empty()){
                const Algorithm::Algorithm& algorithm = algorithmsToAdd.back();
                algorithmsToAdd.pop_back();

                if(this->potentialAlgorithms.end() == std::find(this->potentialAlgorithms.begin(), this->potentialAlgorithms.end(), algorithm)){
                    this->potentialAlgorithms.push_back(algorithm);
                    for(const Algorithm::Algorithm& subAlgorithm : algorithm.getSubAlgorithms()){
                        algorithmsToAdd.push_back(subAlgorithm);
                    }
                }
            }
            
        };

        /**
         * Disable copy construction.
         *
         * Until we see the need for it, there si no reason to enable
         * copy-construction of GraphDotExporter.
         */
        GraphDotExporter(const GraphDotExporter& other) = delete;

        /**
         * Disable GraphDotExporter default assignment operator.
         *
         * Until we see the need for it, there si no reason to enable assignment
         * operator of GraphDotExporter.
         */
        GraphDotExporter& operator=(const GraphDotExporter& other) =
            delete;

        /**
         * Destructor for the exporter.
         *
         * Closes the file.
         */
        ~GraphDotExporter()
        {
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
        void setNewFilePath(const char* newFilePath)
        {
            //  Close previous file
            fclose(pFile);

            // open new one;
            if ((pFile = fopen(newFilePath, "w")) == NULL) {
                pFile = NULL;
                throw std::runtime_error("Could not open file " +
                                         std::string(newFilePath));
            }
        }

        /**
         * \brief Print the Graph given when constructing the
         * GraphDotExporter into a dot file.
         */
        void print();

        /**
         * \brief Print a sub-tree of the Graph given when constructing the
         * GraphDotExporter into a dot file.
         *
         * Contrary to the print() method, which prints the whole TPG, this
         * method only prints the TPG stemming from the EvoGraph::Vertex passed as
         * a parameter. Hence, only vertices and programs connected to this
         * Vertex will be printed in the file, and all others will be
         * ignored.
         *
         * \param[in] agent The agent printed
         */
        void printSubGraph(const Algorithm::Agent& agent);
    };
}; // namespace File

#endif
