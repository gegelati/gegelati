/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019)
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


#ifdef CODE_GENERATION

#ifndef GEGELATI_TPGGENERATIONENGINE_H
#define GEGELATI_TPGGENERATIONENGINE_H
#include "ProgramGenerationEngine.h"
#include <string>
#include "tpg/tpgEdge.h"
#include "tpg/tpgGraph.h"
#include "tpg/tpgTeam.h"

namespace TPG {
    class TpgGenerationEngine
    {
        /**
         * \brief Class in charge of generating the C code of a TPGGraph
         *
         * Each program of the TPGGraph is represented by a C function.
         * All the function are regrouped in a file. Another file holds
         * the main function to iterate threw the TPGGraph.
         *
         */
      protected:
        /// Filename of the file with the main function
        static const std::string filenameProg;

        /// Main function in charge of iterating threw the TPG
        std::ofstream fileMain;
        /// header file for the function that iterates threw the TPG
        std::ofstream fileMainH;

        /**
         * \brief Reference to the TPGGraph for which the code gen is generated.
         */
        const TPG::TPGGraph& tpg;

        /**
         * \brief ProgramGenerationEngine for generating Programs of edges.
         *
         * Keeping this ProgramGenerationEngine as an attribute avoids wasting
         * time rebuilding a new one for each edge.
         */
        Program::ProgramGenerationEngine progGenerationEngine;

        /**
         * \brief Map associating pointers to Program to an integer ID.
         *
         */
        std::map<const Program::Program*, uint64_t> programID;

        /**
         * \brief Integer number used during export to associate a unique
         * integer identifier to each new Program.
         *
         * Using the programID map, a Program that was already printed in
         * previous export will keep its ID.
         */
        uint64_t nbPrograms = 0;

        /**
         * \brief Map associating pointers to TPGVertex to an integer ID.
         *
         * In case the TPGGraphDotExporter is used to export multiple TPGGraph,
         * this map is used to ensure that a given TPGVertex will always be
         * associated to the same integer identifier in all exported dot files.
         */
        std::map<const TPG::TPGVertex*, uint64_t> vertexID;

        /**
         * \brief Integer number used during export to associate a unique
         * integer identifier to each new TPGTeam.
         *
         * Using the VertexID map, a TPGTeam that was already printed in
         * previous export will keep its ID.
         */
        uint64_t nbVertex = 0;

        /**
         * \brief Size of the stack
         *
         * Stack holding the visited edges during the inference
         */
        //todo la stack size
        unsigned int stackSize;

        /**
         * \brief function printing generic code in the main file
         *
         * This function print generic code to execute the TPG and manage the
         * stack of visited edges
         */
        void initTpgFile();

        /**
         * \brief function printing generic code declaration in the main file header
         *
         * This function print the the struct required to represent the TPG and
         * the prototypes of the function to execute the TPG and manage the
         * stack of visited edges
         */
        void initHeaderFile();

      public:
        /**
         * \brief Main constructor of the class.
         *
         * \param[in] filename : filename of the file holding the main function
         *                of the generated program.
         *
         * \param[in] env Environment in which the Program of the TPGGraph will
         *                be executed.
         */
        TpgGenerationEngine(std::string filename, const TPG::TPGGraph& tpg)
            :
            progGenerationEngine{filename+"_"+filenameProg,tpg.getEnvironment()}, tpg{tpg} {
            this->fileMain.open(filename+".c", std::ofstream::out);
            this->fileMainH.open(filename+".h", std::ofstream::out);
            fileMain << "#include \"" << filename << ".h\"" << std::endl;
            fileMain << "#include \"" << filename << "_" << filenameProg << ".h\"" << std::endl;
            initTpgFile();
            fileMainH << "#ifndef C_" << filename << "_H" << std::endl;
            fileMainH << "#define C_" << filename << "_H\n" << std::endl;
            initHeaderFile();
        };

        /**
         * \brief destructor of the class
         *
         * add #endif at the end of the header and close both file
         */

        ~TpgGenerationEngine(){
            fileMainH << "\n#endif" << std::endl;
            fileMain.close();
            fileMainH.close();
        }

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
         * \brief Method for generating an edge of the graph
         *
         * This function generate the code that represent an edge.
         * An edge of a team is represented by a struct with an integer, a function
         * pointer of type : double (*ptr_prog)() the following vertex is represented
         * by a function pointer of type : void* (*ptr_vertex)(int*);
         *
         * \param[in] edge that must be generated
         */
        void generateEdge(const TPG::TPGEdge&);

        /**
         * \brief Method for generating a team of the graph
         *
         * This method generates the C function that represents a team.
         * Each function representing a team contains a static array of Edge
         * and calls the function executeTeam(Edge*, int)
         *
         * \param[in] team that must be generated
         */
        void generateTeam(const TPG::TPGTeam&);

        /**
         * \brief Method for generating a team of the graph
         *
         * This method generates the C function that represents a team.
         * Each function representing a team contains a static array of Edge
         * and calls the function executeTeam(Edge*, int). The generated
         * function return a pointer to the next vertex to execute
         *
         * \param[in] team that must be generated
         */
        void generateAction(const TPG::TPGAction&);

        void setRoot(const TPG::TPGVertex&);

        //todo
        void generateTPGGraph();
    };
} // namespace TPG

#endif // GEGELATI_TPGGENERATIONENGINE_H

#endif // CODE_GENERATION