#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "dataHandlers/dataHandler.h"
#include "instructions/instruction.h"
#include "instructions/set.h"

/**
* \brief The Environment class contains all information needed to execute a Program.
*
* To execute a Program, and size adequately its ProgramLine, a fixed
* Instruction Set, a list of available input DataHandler, and the number of
* available registers is needed.
*
* To ensure viability of Program based on a given Environment, all attributes
* of an Environment are copied in const attributes at construction time.
*/
class Environment {
protected:
	/// Reference to the set of Instruction used by Program running within this Environment.
	const Instructions::Set instructionSet;

	/// List of DataHandler that can be accessed within this Environment.
	const std::vector<std::reference_wrapper<DataHandlers::DataHandler>> dataSources;

	/// Number of registers
	const unsigned int nbRegisters;

private:
	/// Default constructor deleted for its uselessness.
	Environment() = delete;

public:
	/**
	* \brief Constructor with initialization of all attributes.
	*
	* To ensure viability of Program based on a given Environment, all attributes
	* of an Environment are copied in const attributes at construction time.
	*
	* \param[in] iSet the Instructions::Set whose Instruction will be used in this Environment.
	* \param[in] dHandlers the list of DataHandler that will be used in this Environment.
	* \param[in] nbRegs the number of double registers in this Environment.
	*/
	Environment(const Instructions::Set& iSet,
		const std::vector<std::reference_wrapper<DataHandlers::DataHandler>>& dHandlers,
		const unsigned int nbRegs) : instructionSet{ iSet }, dataSources{ dHandlers }, nbRegisters{ nbRegs } {};
};

#endif
