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

	public:
		ProgramExecutionEngine(const Program& prog) : program{ prog }, registers(prog.getEnvironment().getNbRegisters()) {};
	};
};
#endif