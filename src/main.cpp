#include <iostream>

#include "instruction.h"
#include "instructionAdd.h"

int main() {
	std::cout << "Hello TPG World" << std::endl ;

	Instruction* i = new InstructionAdd<double>();

	system("PAUSE");
	return 0;
}