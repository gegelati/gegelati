#ifndef LINE_H
#define LINE_H

#include "environment.h"
#include "parameter.h"

namespace Program {
	/**
	* Class used to store information of a single line of a Program.
	*/
	class Line {

	protected:
		/// Environment within which the Program will be executed.
		const Environment& environment;

		/// index of the Instruction of the Set of the Environment.
		uint64_t instruction;

		/// index of the register to which the result of this line should be written.
		uint64_t destination;

		/// Array storing the parameters for this instruction.
		const Parameter* parameters;

		/// Array storing the operands pair (each with a source and an address)
		const std::pair<uint64_t, uint64_t>* operands;

		/// Delete the default constructor.
		Line() = delete;

	public:
		/**
		* \brief Constructor for a Line of a program.
		*
		* Size of attributes will be determined by the given Environment.
		*
		* \param[in] env the const reference to the Environment for this Program::Line.
		*/
		Line(const Environment& env) :
			environment{ env },
			instruction{ 0 },
			destination{ 0 },
			parameters{ (Parameter*)calloc(env.getMaxNbParameters(), sizeof(Parameter)) },
			operands{ (std::pair<uint64_t, uint64_t>*)calloc(env.getMaxNbOperands(), sizeof(std::pair<uint64_t, uint64_t>)) }{};

		/**
		* Destructor of a Program::Line.
		*
		* Dealocates the memory allocated for attributes.
		*/
		~Line() {
			free((void*)this->parameters);
			free((void*)this->operands);
		}
	};
};

#endif
