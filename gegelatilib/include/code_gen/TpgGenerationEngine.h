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
#include "tpg/tpgAbstractEngine.h"
#include "tpg/tpgEdge.h"
#include "tpg/tpgGraph.h"
#include "tpg/tpgTeam.h"
#include <ios>
#include <iostream>
#include <string>

namespace CodeGen {
    /**
     * \brief Class in charge of generating the C code of a TPGGraph
     *
     * Each program of the TPGGraph is represented by a C function.
     * All the function are regrouped in a file. Another file holds
     * the main function to iterate threw the TPGGraph.
     *
     */
    class TPGGenerationEngine : public TPG::TPGAbstractEngine
    {
      protected:
        /// Filename of the file with the main function
        static const std::string filenameProg;

        /// Main function in charge of iterating threw the TPG
        std::ofstream fileMain;
        /// header file for the function that iterates threw the TPG
        std::ofstream fileMainH;

        /**
         * \brief ProgramGenerationEngine for generating Programs of edges.
         *
         * Keeping this ProgramGenerationEngine as an attribute avoids wasting
         * time rebuilding a new one for each edge.
         */
        CodeGen::ProgramGenerationEngine progGenerationEngine;

        /**
         * \brief Size of the stack
         *
         * Stack holding the visited edges during the inference
         */
        // todo la stack size
        uint64_t stackSize;

        /**
         * \brief function printing generic code in the main file
         *
         * This function print generic code to execute the TPG and manage the
         * stack of visited edges
         */
        // todo des .c/.h qui sont dupliqué pour éviter le recopiage ?
        void initTpgFile();

        /**
         * \brief function printing generic code declaration in the main file
         * header
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
         * \param[in] tpg Environment in which the Program of the TPGGraph will
         *                be executed.
         *
         * \param[in] path to the folder in which the file are generated. If the folder does not exist
         *
         * \param[in] stackSize size of call stack for the execution of the TPG
         * graph
         */
        TPGGenerationEngine(const std::string& filename,
                            const TPG::TPGGraph& tpg,
                            const std::string& path = "./",
                            const uint64_t& stackSize = 8)
            : TPGAbstractEngine(tpg),
              progGenerationEngine{filename + "_" + filenameProg,
                                   tpg.getEnvironment(), path},
              stackSize{stackSize}
        {
            try {
                this->fileMain.open(path+filename + ".c", std::ofstream::out);
                this->fileMainH.open(path+filename + ".h", std::ofstream::out); 
            }
            catch (std::ios_base::failure e) {
                throw std::runtime_error("Could not open file " +
                                         std::string(path+filename));
            }

            fileMain << "#include \"" << filename << ".h\"" << std::endl;
            fileMain << "#include \"" << filename << "_" << filenameProg
                     << ".h\"" << std::endl;
            initTpgFile();
            fileMainH << "#ifndef C_" << filename << "_H" << std::endl;
            fileMainH << "#define C_" << filename << "_H\n" << std::endl;
            initHeaderFile();
        };

        /**
         * \brief destructor of the class
         *
         * add endif at the end of the header and close both file
         */

        ~TPGGenerationEngine()
        {
            fileMainH << "\n#endif" << std::endl;
            fileMain.close();
            fileMainH.close();
        }

        /**
         * \brief Method for generating an edge of the graph
         *
         * This function generates the code that represents an edge.
         * An edge of a team is represented by a struct with an integer, a
         * function pointer of type : double (*ptr_prog)() for the program of
         * the edge and the following vertex is represented by a function
         * pointer of type : void* (*ptr_vertex)(int*);
         *
         * \param[in] edge that must be generated
         */
        void generateEdge(const TPG::TPGEdge& edge);

        /**
         * \brief Method for generating a team of the graph
         *
         * This method generates the C function that represents a team.
         * Each function representing a team contains a static array of Edge
         * and calls the function executeTeam(Edge*, int)
         *
         * \param[in] team that must be generated
         */
        void generateTeam(const TPG::TPGTeam& team);

        /**
         * \brief Method for generating a action of the graph
         *
         * This method generates the C function that represents an action.
         * The generated function return a NULL pointer and write the action in
         * the pointer given as parameter
         *
         * \param[in] action that must be generated
         */
        void generateAction(const TPG::TPGAction& action);

        /**
         * \brief define the function pointer root to the vertex passed in
         * argument
         *
         * \param[in] root of the TPG graph
         */

        void setRoot(const TPG::TPGVertex& root);

        /**
         * \brief function that creates the C files required to execute the TPG
         * without gegelati
         *  /todo complete
         */
        void generateTPGGraph();
    };
} // namespace CodeGen

#endif // GEGELATI_TPGGENERATIONENGINE_H

#endif // CODE_GENERATION