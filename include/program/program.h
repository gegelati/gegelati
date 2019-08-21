#ifndef PROGRAM_H
#define PROGRAM_H

#include <vector>

#include "environment.h"
#include "program/line.h"

namespace Program {
	/**
	* \brief The Program class contains a list of program lines that can be executed
	* within a well defined Environment.
	*/
	class Program {
	protected:
		/// Environment within which the Program will be executed.
		const Environment& environment;

		/// Lines of the program stored as raw bits.
		std::vector<Line*> lines;

		/// Delete the default constructor.
		Program() = delete;

	public:
		/**
		* \brief Main constructor of the Program.
		*
		* \param[in] e the reference to the Environment that will be referenced in the Program attributes.
		*/
		Program(const Environment& e) : environment{ e } {
			// Call the computeLineSize() function to check the validity of the environment.
			Line::computeLineSize(this->environment);
		};

		/**
		* \brief Destructor for the Program class.
		*
		* This destructor deallocates all memory allocated for Program lines (if any).
		*/
		~Program();

		/**
		* \brief Add a new line to the Program with only 0 bits.
		*
		* \return a non-const reference to the newly added Line.
		*/
		Line & addNewLine();

		/**
		* \brief Remove a Line from the Program.
		*
		* Remove the Line at the given index from the Program and free the 
		* associated memory.
		*
		* \param[in] idx the integer index of the Program Line to remove.
		* \throw std::out_of_range if the index is too large.
		*/
		void removeLine(const uint64_t idx);

		/**
		* \brief Get the environment associated to the Program at construction.
		* 
		* \return a const reference to the Environment of the Program and all its 
		** Line.
		*/
		const Environment& getEnvironment() const;

		/**
		* \brief Get the number of lines in the Program.
		*
		* \return the number of line contained in the Program.
		*/
		size_t getNbLines() const;

		/**
		* \brief Get a const ref to a Line of the Program.
		*
		* \param[in] index The integer index of the retrieved Line within the Program.
		* \return a const reference to the indexed Line of the Program.
		* \throw std::out_of_range if the index is too large.
		*/
		const Line & getLine(uint64_t index) const;

		/**
		* \brief Get a non-const ref to a Line of the Program.
		*
		* \param[in] index The integer index of the retrieved Line within the Program.
		* \return a const reference to the indexed Line of the Program.
		* \throw std::out_of_range if the index is too large.
		*/
		Line& getLine(uint64_t index);
	};
}
#endif