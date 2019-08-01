#include <iostream>

#include "instruction.h"
#include "instructionAdd.h"

int main() {
	std::cout << "Hello TPG World" << std::endl ;

	Instructions::Instruction* i = new Instructions::AddPrimitiveType<double>();

	return 0;
}