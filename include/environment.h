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
	const size_t nbRegisters;

	/// Number of Instruction in the Instructions::Set.
	const size_t nbInstructions;

	/// Maxmimum number of operands of the Instructions::Set.
	const size_t maxNbOperands;

	/// Maximum number of Parameter of the Instruction::Set..
	const size_t maxNbParameters;

	/// Number of DataHandler from which data can be accessed.
	const size_t nbDataSources;

	/// Size of the largestAddressSpace of DataHandlers
	const size_t largestAddressSpace;

	/**
	* \brief Static method used when constructing a new Environment to compute 
	* the largest AddressSpace of a set of DataHandler.
	*
	* \param[in] dHandlers reference to the set of DataHandler whose largest largestAddressSpace is searched.
	* \return the found value, or 0 default value if the given std::vector was empty.
	*/
	static size_t computeLargestAddressSpace(const std::vector<std::reference_wrapper<DataHandlers::DataHandler>>& dHandlers);

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
		const unsigned int nbRegs) : instructionSet{ iSet }, dataSources{ dHandlers }, nbRegisters{ nbRegs }, 
		nbInstructions{ iSet.getNbInstructions() }, maxNbOperands{ iSet.getMaxNbOperands() }, 
		maxNbParameters{ iSet.getMaxNbParameters() }, nbDataSources{ dHandlers.size() + 1 }, largestAddressSpace{computeLargestAddressSpace(dHandlers)} {};


};

#endif
