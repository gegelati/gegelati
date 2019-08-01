#include <iostream>

#include "instructions/instruction.h"
#include "instructions/addPrimitiveType.h"

int main() {
	std::cout << "Hello TPG World" << std::endl ;

	Instructions::Instruction* i = new Instructions::AddPrimitiveType<double>();

	return 0;
}