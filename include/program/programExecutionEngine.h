#ifndef PROGRAM_EXECUTION_ENGINE_H
#define PROGRAM_EXECUTION_ENGINE_H

#include "dataHandlers/primitiveTypeArray.h"
#include "program/program.h"

namespace Program {
	/**
	* \brief Class in charge of executing a Program within its Environment.
	*/
	class ProgramExecutionEngine {
	protected:
		/// The program executed by the ProgramExecutionEngine instance.
		const Program& program;

		/// Default constructor is deleted.
		ProgramExecutionEngine() = delete;

		/// Registers used for the Program execution.
		DataHandlers::PrimitiveTypeArray<double> registers;

		/// Data sources used in the Program. (retrieved from the environment at construction time)
		std::vector < std::reference_wrapper<const DataHandlers::DataHandler >> dataSources;

		/// Program counter of the execution engine.
		uint64_t programCounter;

	public:
		/**
		* \brief Constructor of the class.
		*
		* The constructor initialize the number of registers accordingly
		* with the Environment of the given Program.
		*
		* \param[in] prog the const Program that will be executed by the ProgramExecutionEngine.
		*/
		ProgramExecutionEngine(const Program& prog) : program{ prog }, registers(prog.getEnvironment().getNbRegisters()), programCounter{ 0 } {
			// Setup the data sources
			dataSources.push_back(this->registers);
			std::vector<std::reference_wrapper<DataHandlers::DataHandler>> environmentDataSources = this->program.getEnvironment().getDataSources();
			dataSources.insert(dataSources.end(), environmentDataSources.begin(), environmentDataSources.end());
		};

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
		* This method fetches from the dataSources the operands indexed in
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
		const void fetchOperands(std::vector<std::reference_wrapper<const SupportedType>>& operands) const;
	};
};
#endif