#ifndef GEGELATI_PROGRAMENGINE_H
#define GEGELATI_PROGRAMENGINE_H

#include "data/primitiveTypeArray.h"
#include "data/untypedSharedPtr.h"
#include "program/program.h"

namespace Program {
    /**
     * \brief This abstract class is the base class for any program engine
     * (generation and execution)
     *
     * This class holds the common algortithms and members required to generate
     * or execute a program for a given Environment.
     */
    class ProgramEngine
    {
      protected:
        /// The program currently executed by the ProgramExecutionEngine
        /// instance.
        const Program* program;

        /// Default constructor is deleted.
        ProgramEngine() = delete;

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

      protected:
        /**
         * \brief Constructor of the class.
         *
         * The constructor initialize the number of registers accordingly
         * with the Environment given as a parameter.
         *
         * \param[in] env The Environment in which the Program will be executed.
         */
        ProgramEngine(const Environment& env)
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
        ProgramEngine(const Program& prog,
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
        ProgramEngine(const Program& prog)
            : ProgramEngine(prog,
                                     prog.getEnvironment().getDataSources()){};
      public:
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
    };

    template <class T>
    inline void ProgramEngine::setDataSources(
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
} // namespace Program

#endif // GEGELATI_PROGRAMENGINE_H
