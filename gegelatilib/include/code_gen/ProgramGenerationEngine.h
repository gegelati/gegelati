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

#define CODE_GENERATION
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
        static const std::regex operand_regex;
        static const std::string nameRegVariable;
        static const std::string nameDataVariable;

        /// The file in which programs will be added
        std::ofstream fileC;
        /// The file in which prototypes of programs will be added
        std::ofstream fileH;

        /**
         * \brief Set global variables in the file holding the programs
         *
         * Set type of the global variable accordingly to the type of the data sources of the environnement
         * Pay attention that the function typeid::name() is dependant on the compiler choosen.
         * Here the function must return a human readable type. If it's not the case the output of the function has to
         * be unmangle
         *
         */

        void initGlobalVar();

        /**
         * \brief function use to unmangle the char* obtain by the function typeid::name()
         *
         *  // todo
         * for now the function only takes into account the gcc compiler
         *  // todo manage MSVC as compiler (with #ifdef, MSVC le code de retour est lisible pas besoin de unmangle)
         * @return unamngle type of the variable
         */

        char* unmangle(char*);
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
        }

        /**
         * \brief destructor
         *
         * close both files
         */

         ~ProgramGenerationEngine(){
            fileH << "#endif " << std::endl;
            fileC.close();
            fileH.close();
        }

        void generateCurrentLine();

        void generateProgram(uint64_t progID, const bool ignoreException = false);
      protected:
        std::string completeFormat(const Instructions::PrintableInstruction&, const std::vector<Data::UntypedSharedPtr>&) const;
    };

} // namespace Program

#endif // GEGELATI_PROGRAMGENERATIONENGINE_H

#endif // CODE_GENERATION