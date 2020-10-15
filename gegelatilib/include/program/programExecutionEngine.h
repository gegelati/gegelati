/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019)
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

#ifndef PROGRAM_EXECUTION_ENGINE_H
#define PROGRAM_EXECUTION_ENGINE_H

#include <type_traits>

#include "data/primitiveTypeArray.h"
#include "data/untypedSharedPtr.h"
#include "program/program.h"

namespace Program {
    /**
     * \brief Class in charge of executing a Program within its Environment.
     */
    class ProgramExecutionEngine
    {
      protected:
        /// The program currently executed by the ProgramExecutionEngine
        /// instance.
        const Program* program;

        /// Default constructor is deleted.
        ProgramExecutionEngine() = delete;

        /// Registers used for the Program execution.
        Data::PrimitiveTypeArray<double>
            registers; // If the type of registers attribute is changed one day
                       // make sure to update the Program::identifyIntrons()
                       // method as it create its own
                       // Data::PrimitiveTypeArray<double> to keep track of
                       // accessed addresses.

        /// Data sources from the environment used for archiving a program.
        std::vector<std::reference_wrapper<const Data::DataHandler>>
            dataSources;

        /// Data sources (including registers) used in the Program.
        std::vector<std::reference_wrapper<const Data::DataHandler>>
            dataScsConstsAndRegs;

        /// Program counter of the execution engine.
        uint64_t programCounter;

      public:
        /**
         * \brief Constructor of the class.
         *
         * The constructor initialize the number of registers accordingly
         * with the Environment given as a parameter.
         *
         * \param[in] env The Environment in which the Program will be executed.
         */
        ProgramExecutionEngine(const Environment& env)
            : programCounter{0}, registers{env.getNbRegisters()}, program{NULL},
              dataSources{env.getDataSources()}
        {
            // Setup the data sources
            dataScsConstsAndRegs.push_back(this->registers);

            if (env.getNbConstant() > 0) {
                dataScsConstsAndRegs.push_back(env.getFakeDataSources().at(1));
            }

            // Cannot use insert here because it dataSourcesAndRegisters
            // requires constnessand dataSrc data are not const...
            for (auto data : env.getDataSources()) {
                dataScsConstsAndRegs.push_back(data.get());
            }
        }

        /**
         * \brief Constructor of the class.
         *
         * The constructor initialize the number of registers accordingly
         * with the Environment given as a parameter instead of that of the
         * Program or its Environment.
         *
         * This constructor is useful for testing a Program on a different
         * Environment than its own.
         *
         * \param[in] prog the const Program that will be executed by the
         * ProgramExecutionEngine. 
         * \param[in] dataSrc The DataHandler with which
         * the Program will be executed.
         */
        template <class T>
        ProgramExecutionEngine(
            const Program& prog,
            const std::vector<std::reference_wrapper<T>>& dataSrc)
            : programCounter{0},
              registers{prog.getEnvironment().getNbRegisters()}, program{NULL}
        {
            // Check that T is either convertible to a const DataHandler
            static_assert(
                std::is_convertible<T&, const Data::DataHandler&>::value);
            // Setup the data sources
            this->dataScsConstsAndRegs.push_back(this->registers);

            if (prog.getEnvironment().getNbConstant() > 0) {
                this->dataScsConstsAndRegs.push_back(
                    prog.cGetConstantHandler());
            }

            // Cannot use insert here because it dataSourcesAndRegisters
            // requires constnessand dataSrc data are not const...
            for (std::reference_wrapper<T> data : dataSrc) {
                this->dataScsConstsAndRegs.push_back(data.get());
                this->dataSources.push_back(data.get());
            }

            // Set the Program
            this->setProgram(prog);
        };

        /**
         * \brief Constructor of the class.
         *
         * The constructor initialize the number of registers accordingly
         * with the Environment of the given Program.
         *
         * \param[in] prog the const Program that will be executed by the
         * ProgramExecutionEngine.
         */
        ProgramExecutionEngine(const Program& prog)
            : ProgramExecutionEngine(prog,
                                     prog.getEnvironment().getDataSources()){};

        /**
         * \brief Method for changing the Program executed by a
         * ProgramExecutionEngin.
         *
         * \param[in] prog the const Program that will be executed by the
         * ProgramExecutionEngine. \throws std::runtime_error if the Environment
         * references by the Program is incompatible with the dataSources of the
         * ProgramExecutionEngine.
         */
        void setProgram(const Program& prog);

        /**
         * \brief Method for changing the dataSources on which the Program will
         * be executed.
         *
         * \param[in] dataSrc The vector of DataHandler references with which
         * the Program will be executed.
         * \throws std::runtime_error if the Environment references by the
         * Program is incompatible with the given dataSources.
         */
        template <class T>
        void setDataSources(
            const std::vector<std::reference_wrapper<T>>& dataSrc);

        /**
         * \brief Get the DataHandler of the ProgramExecutionEngine.
         *
         * \return a vector containing references to the dataHandlers of the
         * dataSourses attribute (i.e. without the registers)
         */
        const std::vector<std::reference_wrapper<const Data::DataHandler>>&
        getDataSources() const;

        /**
         * \brief Increments the programCounter and checks for the end of the
         * Program.
         *
         * This method will automatically skip intron lines of the Program when
         * searching for the next Line to execute.
         *
         * \return true if the Program of the ProgramExecutionEngine has a Line
         * for the new programCounter value, and false otherwise.
         */
        const bool next();

        /**
         * \brief Get the Program Line corresponding to the current
         * programCounter.
         *
         * \return a const ref to the Line from the Program indexed by the
         * current programCounter.
         * \throw std::out_of_range if the programCounter exceeds the number of
         * lines of the program.
         */
        const Line& getCurrentLine() const;

        /**
         * \brief Get the Instruction corresponding to the current
         * programCounter.
         *
         * \return the Instruction from the Environment Instruction::Set for
         * the Line of the Program indexed by the current programCounter.
         * \throw std::out_of_range if the programCounter exceeds the number of
         * lines of the program or if the instruction index contained in the
         * current Line exceeds the number of Instruction in the Environment
         * Instructions::Set.
         */
        const Instructions::Instruction& getCurrentInstruction() const;

        /**
         * \brief Get the operands for the current Instruction.
         *
         * This method fetches from the dataSourcesAndRegisters the operands
         * indexed in the current Line of the Program. To get the correct data,
         * the method Uses the data types of the current Instruction of the
         * program.
         *
         * \param[in,out] operands std::vector where the fetched operands will
         * be inserted. \throws std::invalid_argument if the data type of the
         * current Instruction is not provided by the indexed DataHandler.
         * \throws std::out_of_range if the given address is invalid for the
         * indexed DataHandler, with the given data type, or if the indexed
         *         DataHandler does not exist.
         */
        const void fetchCurrentOperands(
            std::vector<Data::UntypedSharedPtr>& operands) const;

        /**
         * \brief Execute the current line of the program.
         *
         * \throws see fetchCurrentOperands, getCurrentInstruction.
         */
        void executeCurrentLine();

        /**
         * \brief Execute the program completely and returns the content of
         * register 0.
         *
         * \param[in] ignoreException When true, all exceptions thrown when
         *            fetching current instructions, operands are
         *            caught and the current program Line is simply ignored.
         *            When true, all lines of the Program are assumed to be
         *            correct by construction, and any exception is re-thrown
         *            for higher-level handling, thus stopping the program.
         *            Exception thrown by getCurrentLine are never ignored.
         * \return the double value contained in the 0-indexed register at the
         *         end of the program execution.
         */
        double executeProgram(const bool ignoreException = false);
    };
    template <class T>
    inline void ProgramExecutionEngine::setDataSources(
        const std::vector<std::reference_wrapper<T>>& dataSrc)
    {
        // Check that T is either convertible to a const DataHandler
        static_assert(std::is_convertible<T&, const Data::DataHandler&>::value);

        // Replace the references in attributes
        this->dataSources = dataSrc;
        // we need this offset to push the constant at the firs
        size_t offset =
            this->program->getEnvironment().getNbConstant() > 0 ? 2 : 1;
        if (this->program && offset == 2) {
            this->dataScsConstsAndRegs.at(1) =
                this->program->cGetConstantHandler();
        }
        for (size_t idx = 0; idx < this->dataSources.size(); idx++) {
            this->dataScsConstsAndRegs.at(idx + offset) = dataSrc.at(idx);
        }

        // Set program to check compatibility with new data source
        this->setProgram(*this->program);
    }
}; // namespace Program
#endif
