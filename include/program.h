#ifndef PROGRAM_H
#define PROGRAM_H

#include <vector>

#include "environment.h"

/**
* \brief The Program class contains a list of program lines that can be executed
* within a well defined Environment.
*/
class Program {
protected:
	/// Environment within which the Program will be executed.
	const Environment& environment;

	/// Size of the lines (in bits)
	const size_t lineSize;

	/// Actual line size (in bytes). Only the lineSize first bits will be used.
	const size_t actualLineSize;

	/// Lines of the program stored as raw bits.
	std::vector<char*> lines;

	/// Delete the default constructor.
	Program() = delete;

	/**
	* \brief Static method used to compute the size of Program lines based on information from the Enviroment.
	*
	* The Program line size, expressed in bits, is computed with the following formula:
	* $ceil(log2(n)) + ceil(log2(i)) + m*(ceil(log2(nb_{src}))+ceil(log2(largestAddressSpace)) + p*32$
	* See PROJECT/doc/instructions.md for more details.
	*
	* \param[in] env The Environment whose information is used.
	* \return the computed line size.
	* \throw std::domain_error in cases where the given Environment is 
	* parameterized with no registers, contains no Instruction, Instruction 
	* with no operands, no DataHandler or DataHandler with no addressable Space.
	*/
	static const size_t computeLineSize(const Environment& env);

public:
	/**
	* \brief Main constructor of the Program.
	*
	* \param[in] e the reference to the Environment that will be referenced in the Program attributes.
	*/
	Program(const Environment& e) : environment{ e }, lineSize{ computeLineSize(e) }, 
		actualLineSize{ (size_t)ceill(((long double)computeLineSize(e)/8.0l)) }{};

	/**
	* \brief Destructor for the Program class.
	*
	* This destructor deallocates all memory allocated for Program lines (if any).
	*/
	~Program();

	/**
	* \brief Get the value of the lineSize attribute.
	*
	* \return the value computed at construction time.
	*/
	size_t getLineSize() const;

	/**
	* \brief Add a new line to the Program with only 0 bits.
	*
	* \throw std::bad_alloc in case the new line allocation failed.
	*/
	void addNewLine();

	/**
	* \brief Get the number of lines in the Program.
	*
	* \return the number of line contained in the Program.
	*/
	size_t getNbLines() const;
};

#endif