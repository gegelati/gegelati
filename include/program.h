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

	/// Lines of the program stored as raw bits.
	std::vector<char*> lines;

	/// Delete the default constructor.
	Program() = delete;

public:
	/**
	* \brief Main constructor of the Program.
	*
	* \param[in] e the reference to the Environment that will be referenced in the Program attributes.
	*/
	Program(const Environment& e) : environment{ e } {};

	/**
	* \brief Destructor for the Program class.
	*
	* This destructor deallocates all memory allocated for Program lines (if any).
	*/
	~Program() {
		while (!lines.empty()) {
			char* line = lines.back();
			free(line);
			lines.pop_back();
		}
	}


};

#endif