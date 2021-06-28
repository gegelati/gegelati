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

#ifndef GEGELATI_PROGRAMGENERATIONENGINE_H
#define GEGELATI_PROGRAMGENERATIONENGINE_H
#include "PrintableInstruction.h"
#include "program/programEngine.h"
//#include <bits/fcntl-linux.h>
#include <fstream>

namespace Program {
    /**
     *  \brief Class in charge of generating a Program.
     */
    class ProgramGenerationEngine : public ProgramEngine
    {
      protected:
        /// regex used to identify operand in the format of a printableInstruction
        static const std::regex operand_regex;

        /**
         * \brief name given to the global variable in generated files.
         *
         * "nameDataVariable"1 corresponds to the first variable
         * "nameDataVariable"2 is used if an other data is given
         * ...
         */
        static const std::string nameDataVariable;

        /// names of the registers in the TPG's programs
        static const std::string nameRegVariable;

        /// The file in which programs will be added
        std::ofstream fileC;
        /// The file in which prototypes of programs will be added
        std::ofstream fileH;

        /**
         * \brief Set global variables in the file holding the programs
         * //todo
         * Set type of the global variable accordingly to the type of the data sources of the environnement
         * Pay attention that the function typeid::name() is dependant on the compiler choosen.
         * Here the function must return a human readable type. If it's not the case the output of the function has to
         * be unmangle
         *
         */

        void initGlobalVar();

      public:
        /**
         * \brief Constructor of the class
         *
         * The constructor initialize the member of the parent class (ProgramEngine)
         * and the file "filename" is open with the flag std::ofstream::app
         * to generate the program in the file.
         *
         * \param[in] filename
         * \param[in] env
         */

        ProgramGenerationEngine(const std::string& filename,const Environment& env)
            : ProgramEngine(env){
            this->fileC.open(filename+".c", std::ofstream::out /*| std::ofstream::app*/);
            this->fileH.open(filename+".h", std::ofstream::out /*| std::ofstream::app*/);
            fileC << "#include \"" << filename <<".h\"" << std::endl;
            initGlobalVar();
            fileH << "#ifndef C_" << filename << "_H" << std::endl;
            fileH << "#define C_" << filename << "_H\n" << std::endl;
            fileC << "#include \"externHeader.h\"" << std::endl;
#ifdef DEBUG
            fileC << "#include <stdio.h>" << std::endl;
#endif // DEBUG
        }

        /**
         * \brief destructor of the class
         *
         * close both files and add endif at the end of the header
         */

         ~ProgramGenerationEngine(){
            fileH << "#endif " << std::endl;
            fileC.close();
            fileH.close();
        }

        /**
         * \brief generate the current line of the program
         *
         * Generate the line of code that corresponds to the current line in the
         * program of the TPG.
         */
        void generateCurrentLine();

        /**
         * \brief generate the C code that corresponds to the member program of the class
         *
         * Create a function in the file "filename"_program.c that regroup all
         * the instruction of the program and return a double. The name of the
         * function is based on the identifier of the program. The declaration of
         * function of the program with ID=1 is double P1(int* action)
         *
         * \param[in] progID unique identifier of the program used to generate the name
         *            of the function in the C file.
         * \param[in] ignoreException When true, all exceptions thrown when
         *            fetching current instructions, operands are
         *            caught and the current program Line is simply ignored.
         *            When true, all lines of the Program are assumed to be
         *            correct by construction, and any exception is re-thrown
         *            for higher-level handling, thus stopping the program.
         *            Exception thrown by getCurrentLine are never ignored.
         */
        void generateProgram(uint64_t progID, const bool ignoreException = false);

      protected:
        /**
         * \brief create the line of C code that equals to instruction in parameter
         *
         * Replace each operand of the format of the printable instruction by a
         * pointer to the data of the environment.
         *
         * \param[in] instruction that as to be converted into a line of code
         *
         * @return a line of code that can be printed in the program file
         */
        std::string completeFormat(const Instructions::PrintableInstruction& instruction) const;
    };

} // namespace Program

#endif // GEGELATI_PROGRAMGENERATIONENGINE_H

#endif // CODE_GENERATION