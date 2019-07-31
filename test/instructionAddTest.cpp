#include <gtest/gtest.h>
#include "instructionAdd.h"

TEST(InstructionAdd, ConstructorCallDouble) {
	Instruction* i = new InstructionAdd<double>();
	ASSERT_NE(i, nullptr);
}

TEST(InstructionAdd, ConstructorCallInt) {
	Instruction* i = new InstructionAdd<int>();
	ASSERT_NE(i, nullptr);
}
