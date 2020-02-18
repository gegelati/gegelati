#ifndef PROGRAM_EXECUTION_ENGINE_H
#define PROGRAM_EXECUTION_ENGINE_H

#include <type_traits>

#include "data/primitiveTypeArray.h"
#include "program/program.h"

namespace Program {
	/**
	* \brief Class in charge of executing a Program within its Environment.
	*/
	class ProgramExecutionEngine {
	protected:
		/// The program currently executed by the ProgramExecutionEngine instance.
		const Program* program;

		/// Default constructor is deleted.
		ProgramExecutionEngine() = delete;

		/// Registers used for the Program execution.
		Data::PrimitiveTypeArray<double> registers;

		/// Data sources used in the Program.
		std::vector < std::reference_wrapper<const Data::DataHandler >> dataSources;

		/// Data sources (including registers) used in the Program.
		std::vector < std::reference_wrapper<const Data::DataHandler >> dataSourcesAndRegisters;

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
		ProgramExecutionEngine(Environment& env) : programCounter{ 0 }, registers{ env.getNbRegisters() }, program{ NULL }, dataSources{ env.getDataSources() } {
			// Setup the data sources
			dataSourcesAndRegisters.push_back(this->registers);

			// Cannot use insert here because it dataSourcesAndRegisters requires 
			// constnessand dataSrc data are not const...
			for (auto data : env.getDataSources()) {
				dataSourcesAndRegisters.push_back(data.get());
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
		* \param[in] prog the const Program that will be executed by the ProgramExecutionEngine.
		* \param[in] dataSrc The DataHandler with which the Program will be executed.
		*/
		template <class T> ProgramExecutionEngine(const Program& prog, const std::vector<std::reference_wrapper<T>>& dataSrc) : programCounter{ 0 }, registers{ prog.getEnvironment().getNbRegisters() }, program{ NULL } {
			// Check that T is either convertible to a const DataHandler
			static_assert(std::is_convertible<T&, const Data::DataHandler&>::value);
			// Setup the data sources
			this->dataSourcesAndRegisters.push_back(this->registers);

			// Cannot use insert here because it dataSourcesAndRegisters requires 
			// constnessand dataSrc data are not const...
			for (std::reference_wrapper <T> data : dataSrc) {
				this->dataSourcesAndRegisters.push_back(data.get());
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
		* \param[in] prog the const Program that will be executed by the ProgramExecutionEngine.
		*/
		ProgramExecutionEngine(const Program& prog) : ProgramExecutionEngine(prog, prog.getEnvironment().getDataSources()) {};

		/**
		* \brief Method for changing the Program executed by a ProgramExecutionEngin.
		*
		* \param[in] prog the const Program that will be executed by the ProgramExecutionEngine.
		* \throws std::runtime_error if the Environment references by the
		* Program is incompatible with the dataSources of the ProgramExecutionEngine.
		*/
		void setProgram(const Program& prog);

		/**
		* \brief Method for changing the dataSources on which the Program will be executed.
		* 
		* \param[in] dataSrc The vector of DataHandler references with which 
		* the Program will be executed.
		* \throws std::runtime_error if the Environment references by the
		* Program is incompatible with the given dataSources.
		*/
		template <class T> void setDataSources(const std::vector<std::reference_wrapper<T>>& dataSrc);

		/**
		* \brief Get the DataHandler of the ProgramExecutionEngine.
		*
		* \return a vector containing references to the dataHandlers of the
		* dataSourses attribute (i.e. without the registers)
		*/
		const std::vector<std::reference_wrapper<const Data::DataHandler>>& getDataSources() const;

		/**
		* \brief Increments the programCounter and checks for the end of the Program.
		*
		* This method will automatically skip intron lines of the Program when
		* searching for the next Line to execute.
		*
		* \return true if the Program of the ProgramExecutionEngine has a Line
		* for the new programCounter value, and false otherwise.
		*/
		const bool next();

		/**
		* \brief Get the Program Line corresponding to the current programCounter.
		*
		* \return a const ref to the Line from the Program indexed by the
		* current programCounter.
		* \throw std::out_of_range if the programCounter exceeds the number of
		* lines of the program.
		*/
		const Line& getCurrentLine() const;

		/**
		* \brief Scale a location from the Environment largestAddressSpace to the
		*        largestAddressSpace of the given dataHandler, for the given data
		*        type.
		*
		* This function computes a valid location for the givenDataHandler,
		* with the givenDataType. This location is computed from the given
		* rawLocation with a simple modulo. The choice of the Modulo was made
		* for simplicity and faster execution, but may introduce a bias
		* towards low values. If this becomes a problem, a proper scaling with
		* a division would be needed.
		*
		* \param[in] rawLocation integer number between 0 and the environment
		*            largestAddressSpace.
		* \param[in] dataHandler the dataHandler whose data is being accessed.
		* \param[in] type the type of data accessed.
		* \return (rawLocation % dataHandler.largestAddressSpace(type))
		* \throw std::domain_error if the data type is not supported by the
		*        data handler.
		*/
		uint64_t scaleLocation(const uint64_t rawLocation, const Data::DataHandler& dataHandler, const std::type_info& type) const;

		/**
		* \brief Get the Instruction corresponding to the current programCounter.
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
		* This method fetches from the dataSourcesAndRegisters the operands indexed in
		* the current Line of the Program. To get the correct data, the method
		* Uses the data types of the current Instruction of the program.
		*
		* \param[in,out] operands std::vector where the fetched operands will be inserted.
		* \throws std::invalid_argument if the data type of the current Instruction
		*         is not provided by the indexed DataHandler.
		* \throws std::out_of_range if the given address is invalid for the indexed
		*         DataHandler, with the given data type, or if the indexed
		*         DataHandler does not exist.
		*/
		const void fetchCurrentOperands(std::vector<std::shared_ptr<const SupportedType>>& operands) const;

		/**
		* \brief Get the parameters for the current Instruction.
		*
		* This method retrieves the number of Parameter required by the
		* current Instruction and stores them into the given vector.
		*
		* \param[in,out] parameters std::vector where the fetched Parameter will be inserted.
		* \throw std::out_of_range if the programCounter is beyond the program end, if the
		*        instruction requires more parameter than available.
		*/
		const void fetchCurrentParameters(std::vector<std::reference_wrapper<const Parameter>>& parameters) const;

		/**
		* \brief Execute the current line of the program.
		*
		* \throws see fetchCurrentParameters, fetchCurrentOperands, getCurrentInstruction.
		*/
		void executeCurrentLine();

		/**
		* \brief Execute the program completely and returns the content of register 0.
		*
		* \param[in] ignoreException When true, all exceptions thrown when
		*            fetching current instructions, operands, parameters are
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
	template<class T>
	inline void ProgramExecutionEngine::setDataSources(const std::vector<std::reference_wrapper<T>>& dataSrc)
	{
		// Check that T is either convertible to a const DataHandler
		static_assert(std::is_convertible<T&, const Data::DataHandler&>::value);

		// Replace the references in attributes
		this->dataSources = dataSrc;
		for (auto idx = 0; idx < this->dataSources.size(); idx++) {
			this->dataSourcesAndRegisters.at(idx + 1) = dataSrc.at(idx);
		}

		// Set program to check compatibility with new data source
		this->setProgram(*this->program);
	}
};
#endif