#include <gtest/gtest.h>

#include "code_gen/ProgramGenerationEngine.h"
#include "code_gen/LambdaPrintableInstruction.h"

class ProgramGenerationEngineTest : public ::testing::Test{
  protected:
    std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
    const size_t size1{32};
    Instructions::Set set;
    Environment* e;
    Program::Program* p;

    virtual void SetUp()
    {
        vect.push_back(
            *(new Data::PrimitiveTypeArray<double>((unsigned int)size1)));


        auto add = [](double a, double b)->double{return a+b;};
        auto sub = [](double a, double b)->double{return a-b;};
        set.add(*(new Instructions::LambdaPrintableInstruction<double, double>("$0 = $1 + $2;", add)));
        set.add(*(new Instructions::LambdaPrintableInstruction<double, double>("$0 = $1 - $2;",sub)));
        set.add(*(new Instructions::LambdaInstruction<double, double>(add)));


        e = new Environment(set, vect, 8);
        p = new Program::Program(*e);

        Program::Line& l0 = p->addNewLine();
        l0.setInstructionIndex(0); // Instruction is add.
        //Reg[5] = in1[0] + in1[1];
        l0.setOperand(0, 1, 0);    // 1st operand: parameter 0.
        l0.setOperand(0, 1, 1);    // 2nd operand: parameter 1.
        l0.setDestinationIndex(5); // Destination is register at index 5 (6th)

        Program::Line& l1 = p->addNewLine();
        //Reg[1] = reg[5] + in1[25];
        l1.setInstructionIndex(0); // Instruction is add.
        l1.setOperand(0, 0, 5);    // 1st operand: 6th register.
        l1.setOperand(1, 1, 25);   // 2nd operand: parameter 26.
        l1.setDestinationIndex(1); // Destination is register at index 1

        // Intron line
        Program::Line& l2 = p->addNewLine();
        //Reg[5] = reg[3] - in1[0];
        l2.setInstructionIndex(1); // Instruction is minus.
        l2.setOperand(0, 0, 3);    // 1st operand: 3rd register.
        l2.setOperand(1, 1, 0);    // 2nd operand: parameter 0.
        l2.setDestinationIndex(5); // Destination is register at index 0

        Program::Line& l3 = p->addNewLine();
        //Reg[0] = reg[1] - in1[1];
        l3.setInstructionIndex(1); // Instruction is minus.
        l3.setOperand(0, 0, 1);    // 1st operand: 1st register.
        l3.setOperand(1, 1, 1);    // 2nd operand: 1st parameter.
        l3.setDestinationIndex(0); // Destination is register at index 0

        Program::Line& l4 = p->addNewLine();
        //Reg[0] = reg[0] + in1[5];
        l4.setInstructionIndex(1);                 // Instruction is minus.
        l4.setOperand(0, 0, 0); // 1st operand: 0th and 1st registers.
        l4.setOperand(1, 1, 5); // 2nd operand : parameter 6.
        l4.setDestinationIndex(0); // Destination is register at index 0

        // Mark intron lines
        ASSERT_EQ(p->identifyIntrons(), 1);
    }

    virtual void TearDown()
    {
        delete p;
        delete e;
        delete (&(vect.at(0).get()));
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
        delete (&set.getInstruction(2));
    }

};

TEST_F(ProgramGenerationEngineTest, ConstructorDestructor){
    CodeGen::ProgramGenerationEngine* progGen;
    ASSERT_NO_THROW(progGen = new CodeGen::ProgramGenerationEngine("constructor", *e)) <<  "Construction failed.";

    ASSERT_NO_THROW(delete progGen) << "Destruction failed.";

    ASSERT_THROW(progGen = new CodeGen::ProgramGenerationEngine("", *e), std::invalid_argument) <<  "Construction should fail, filename is empty.";

}

TEST_F(ProgramGenerationEngineTest, generateCurrentLine){


    CodeGen::ProgramGenerationEngine progGen("genCurrentLine", *e);

    progGen.setProgram(*p);
    ASSERT_NO_THROW(progGen.generateCurrentLine()) << "Can't generate the first line";

    Program::Program* p1 = new Program::Program(*e);
    Program::Line& l0 = p1->addNewLine();
    l0.setInstructionIndex(2); // Instruction is add(non printable).
    //Reg[5] = in1[0] + in1[1];
    l0.setOperand(0, 1, 0);    // 1st operand: parameter 0.
    l0.setOperand(0, 1, 1);    // 2nd operand: parameter 1.
    l0.setDestinationIndex(5); // Destination is register at index 5 (6th)
    progGen.setProgram(*p1);

    ASSERT_THROW(progGen.generateCurrentLine(), std::runtime_error)
        << "Should not be able to generate line, the instruction is not printable";

    delete p1;
}

TEST_F(ProgramGenerationEngineTest, generateProgram){
    CodeGen::ProgramGenerationEngine progGen("genProgram", *p);

    ASSERT_NO_THROW(progGen.generateProgram(1)) <<
        "Out of range exception while generating the program";
}

//TEST_F(ProgramGenerationEngineTest, completeFormat){
//    CodeGen::ProgramGenerationEngine progGen("completFormat", *e);
//
//
//}
