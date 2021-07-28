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

#ifndef PROGRAMGENERATIONENGINE_H
#define PROGRAMGENERATIONENGINE_H
#include <fstream>

#include "data/dataHandlerPrinter.h"
#include "data/primitiveTypeArray.h"
#include "instructions/instruction.h"
#include "program/programEngine.h"

namespace CodeGen {
    /**
     *  \brief Class in charge of generating all the programs of a TPG.
     *
     *  This class generates the files(header and source code) that contains the
     * programs of the TPG. A program can be generated if all its instructions
     * are printable (inherits from PrintableInstruction).
     *
     *  The header externHeader.h allows to add the required header(like math.h)
     * to compile the generated code without modifying it.
     */
    class ProgramGenerationEngine : public Program::ProgramEngine
    {
      protected:
        /// regex used to identify operand in the printTemplate of an
        /// Instruction.
        static const std::regex operand_regex;

        /**
         * \brief name given to the global variable in generated files.
         *
         * "nameDataVariable"1 corresponds to the first variable
         * "nameDataVariable"2 is used if an other data is given
         * ...
         */
        static const std::string nameDataVariable;

        /// name of the registers in the TPG's programs.
        static const std::string nameRegVariable;

        /// name of the array of constants in the TPG's programs.
        static const std::string nameConstantVariable;

        /// name of the temporary operand used in the TPG's programs.
        static const std::string nameOperandVariable;

        /// The file in which programs will be added.
        std::ofstream fileC;
        /// The file in which prototypes of programs will be added.
        std::ofstream fileH;

        /**
         * \brief Map associating a DataHandlerPrinter to the ID of its
         * DataHandler
         *
         * This map is filled in the generateDataPrinterMap method.
         */
        Data::DataHandlerPrinter dataPrinter;

        /// inherited from Program::ProgramEngine
        virtual void processLine() override;

      public:
        /**
         * \brief Constructor of the class
         *
         * The constructor initialize the member of the parent class
         * (ProgramEngine) and the file "filename" is open with the flag
         * std::ofstream::app to generate the program in the file.
         *
         * \param[in] filename a const reference used to generate the filename
         * of the file that holds the programs of the TPG graph. The filename
         * holding the programs of the TPG is : 'filename'_program.c
         *
         * \param[in] env a const reference to the Environment of the TPGGraph
         *
         * \param[in] path a const reference to the path in which the file must
         * be generated. By default, the file is generated in the current
         * directory.
         */
        ProgramGenerationEngine(const std::string& filename,
                                const Environment& env,
                                const std::string& path = "./")
            : ProgramEngine(env), dataPrinter()
        {
            openFile(filename, path, env.getNbConstant());
        }

        /**
         * \brief Constructor of the class
         *
         * The constructor initialize the member of the parent class
         * (ProgramEngine) and the file "filename" is open with the flag
         * std::ofstream::out to generate the program in the file.
         *
         * This constructor is useful to create a ProgramGeneration engine
         *
         * \param[in] filename a const reference used to generate the filename
         * of the file that holds the programs of the TPG graph. The filename
         * holding the programs of the TPG is : 'filename'_program.c
         *
         * \param[in] p const reference to a Program
         *
         * \param[in] path const reference to the path in which the file is
         * generated
         */
        ProgramGenerationEngine(const std::string& filename,
                                const Program::Program& p,
                                const std::string& path = "./")
            : ProgramEngine(p), dataPrinter()
        {
            openFile(filename, path, p.getEnvironment().getNbConstant());
            setProgram(p);
        }

        /**
         * \brief destructor of the class
         *
         * close both files and add endif at the end of the header
         */
        ~ProgramGenerationEngine()
        {
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
         * \brief generate the C code that corresponds to the member program of
         * the class
         *
         * Create a function in the file "filename"_program.c that regroups all
         * the instruction of the program and return a double. The name of the
         * function is based on the identifier of the program. The declaration
         * of function of the program with ID=1 is double P1(int* action)
         *
         * \param[in] progID : unique identifier of the program used to generate
         *            the name of the function in the C file.
         * \param[in] ignoreException When true, all exceptions thrown when
         *            fetching current instructions, operands are
         *            caught and the current program Line is simply ignored.
         *            When false, all lines of the Program are assumed to be
         *            correct by construction, and any exception is re-thrown
         *            for higher-level handling, thus stopping the program.
         *            Exception thrown by getCurrentLine are never ignored.
         */
        void generateProgram(uint64_t progID,
                             const bool ignoreException = false);

      protected:
        /**
         * \brief Set global variables in the file holding the programs.
         *
         * Set type of the global variable accordingly to the type of the data
         * sources of the environment.
         *
         * \param[in] nbConstant size_t of the number of Data::Constant
         * available for a Program.
         */
        void initGlobalVar(size_t nbConstant);

        /**
         * \brief create the line of C code that equals to instruction in
         * parameter
         *
         * Replace each operand of the printTemplate of the printable
         * instruction by a pointer to the data of the environment.
         *
         * \param[in] instruction that as to be converted into a line of code
         *
         * @return a copy of the printTemplate with the variables changed
         * according to the operand of the instruction.
         */
        std::string completeFormat(
            const Instructions::Instruction& instruction) const;

        // private:
        /**
         * \brief Function used to open the file that as to be generated.
         *
         * This is function is called in both constructor of the class. It open
         * the source file and the header file. It also add the include guards
         * in the header.
         *
         * \param[in] filename const reference to the name of the file.
         * \param[in] path const reference to the path of the file.
         * \param[in] nbConstant number of constant used in the program of the
         * TPG.
         */
        void openFile(const std::string& filename, const std::string& path,
                      size_t nbConstant);

        /**
         * \brief Function called to generate the initialization of all operand
         * of an instruction.
         *
         * This function prints in the C source file the declaration and the
         * initialization for each operand of the current line.
         */
        void initOperandCurrentLine();

        /**
         * \brief Method returning the name of the data source in the file
         * generated.
         *
         * \param[in] idx const uint64_t reference to the index of the data
         * source in Environment of the Program.
         * \return the name of the variable to use to access the data source in
         * the generated program
         */
        std::string getNameSourceData(const uint64_t& idx);
    };

} // namespace CodeGen

#endif // PROGRAMGENERATIONENGINE_H

#endif // CODE_GENERATION