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
		Parameter* const parameters;

		/// Array storing the operands pair (each with an index for the 
		/// DataHandlers of the Environment, and a location within it.)
		std::pair<uint64_t, uint64_t>* const operands;

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

		/**
		* \brief Getter for the destination of this Line.
		*
		* \return the value of the destination attribute.
		*/
		uint64_t getDestination() const;

		/**
		* \brief Setter for the destination of this Line.
		*
		* Optionnaly, the validity of the given value can be checked with
		* regards to the Environment of the Line. If the given value is not
		* valid (i.e. it exceeds the number of available destinations) the
		* attribute will not be overwritten.
		*
		* \param[in] dest the new value for the destination attribute.
		* \param[in] check whether the validity of the given new value.
		*
		* \return true if checks are not activated, otherwise, return whether
		* the value was valid and thus the writing was performed or not.
		*/
		bool setDestination(const uint64_t dest, const bool check = true);

		/**
		* \brief Getter for the instruction of this Line.
		*
		* \return the value of the instruction attribute.
		*/
		uint64_t getInstruction() const;

		/**
		* \brief Setter for the instruction of this Line.
		*
		* Optionnaly, the validity of the given value can be checked with
		* regards to the Environment of the Line. If the given value is not
		* valid (i.e. it exceeds the number of available instructions) the
		* attribute will not be overwritten.
		*
		* \param[in] instr the new value for the instruction attribute.
		* \param[in] check whether the validity of the given new value.
		*
		* \return true if checks are not activated, otherwise, return whether
		* the value was valid and thus the writing was performed or not.
		*/
		bool setInstruction(const uint64_t instr, const bool check = true);

		/**
		* \brief Getter for the parameters of this Line.
		*
		* \param[in] idx the index of the accessed Parameter.
		* \return the const reference of the parameter at the given index.
		* \throw std::range_error if the given index exceeds the number of
		* Parameter of the Line.
		*/
		const Parameter& getParameter(const uint64_t idx) const;

		/**
		* \brief Setter for the parameters of this Line.
		*
		* \param[in] idx the index of the set Parameter.
		* \param[in] p the new value for the parameter.
		* \throw std::range_error if the given index exceeds the number of
		* Parameter of the Line.
		*/
		void setParameter(const uint64_t idx, const Parameter p);

		/**
		*
		* \brief Getter for the operands of this Line.
		*
		* \param[in] idx the index of the accessed operand.
		* \return the value of the operand pair at the given index.
		* \throw std::range_error if the given index exceeds the number of
		* Operands of the Line.
		*/
		const std::pair<uint64_t, uint64_t>& getOperand(const uint64_t idx) const;

		/**
		* \brief Setter for the operands of this Line.
		*
		* Optionnaly, the validity of the given values can be checked with
		* regards to the Environment of the Line. If the given values is not
		* valid (i.e. the dataHandler index exceeds the number of DataHandler
		* of the Environment, or the location exceeds the largestAddressSpace
		* of the indexed DataHandler) the attribute will not be overwritten.
		*
		* \param[in] idx the index of the Operand to write.
		* \param[in] dataIndex index of a DataHandler.
		* \param[in] location the location of the operand value within the
		*            DataHandler.
		* \param[in] check whether the validity of the given new value.
		*
		* \return true if checks are not activated, otherwise, return whether
		* the value was valid and thus the writing was performed or not.
		* \throw std::range_error if the given index exceeds the number of
		* Operands of the Line.
		*/
		bool setOperand(const uint64_t idx, const uint64_t dataIndex, const uint64_t location, const bool check = true);
	};
};

#endif
