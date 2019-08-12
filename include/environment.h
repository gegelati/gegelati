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
	* \param[in] nbRegisters the number of registers of the environment.
	* \param[in] dHandlers reference to the set of DataHandler whose largest largestAddressSpace is searched.
	* \return the found value, or 0 default value if the given std::vector was empty.
	*/
	static size_t computeLargestAddressSpace(const size_t nbRegisters, const std::vector<std::reference_wrapper<DataHandlers::DataHandler>>& dHandlers);

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
		maxNbParameters{ iSet.getMaxNbParameters() }, nbDataSources{ dHandlers.size() + 1 }, largestAddressSpace{ computeLargestAddressSpace(nbRegs, dHandlers) } {};

	/**
	* \brief Get the size of the number of registers of this Environment.
	*
	* \return the value of the nbRegisters attribute.
	*/
	size_t getNbRegisters() const;

	/**
	* \brief Get the size of the number of Instruction within the Instructions::Set.
	*
	* \return the value of the nbInstructions attribute.
	*/
	size_t getNbInstructions() const;

	/**
	* \brief Get the size of the maximum number of operands of Instructions::Set.
	*
	* \return the value of the maxNbOperands attribute.
	*/
	size_t getMaxNbOperands() const;

	/**
	* \brief Get the size of the maximum number of Parameter of Instructions::Set.
	*
	* \return the value of the maxNbParamaters attribute.
	*/
	size_t getMaxNbParameters() const;

	/**
	* \brief Get the size of the number of DataHandlers.
	*
	* \return the value of the nbDataSources attribute.
	*/
	size_t getNbDataSources() const;

	/**
	* \brief Get the size of the largestAddressSpace of DataHandlers.
	*
	* \return the value of the LargestAddressSpace attribute.
	*/
	size_t getLargestAddressSpace() const;

	/**
	* \brief Get the DataHandler of the Environment.
	*
	* \return a const reference to the dataSources attribute of this Environment.
	*/
	const std::vector<std::reference_wrapper<DataHandlers::DataHandler>>& getDataSources() const;
	
	/**
	* \brief Get the Instruction Set of the Environment.
	*
	* \return a const reference to the instructionSet attribute of this Environment.
	*/
	const Instructions::Set& getInstructionSet() const;

};

#endif
