/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

#include <gtest/gtest.h>
#include <vector>

#include "data/dataHandler.h"
#include "data/primitiveTypeArray.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/lambdaInstruction.h"
#include "instructions/set.h"
#include "program/line.h"
#include "program/program.h"

class ProgramTest : public ::testing::Test
{
  protected:
    const size_t size1{24};
    const size_t size2{32};
    std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
    Instructions::Set set;
    Environment* e;

    virtual void SetUp()
    {
        vect.push_back(
            *(new Data::PrimitiveTypeArray<double>((unsigned int)size1)));
        vect.push_back(
            *(new Data::PrimitiveTypeArray<int>((unsigned int)size2)));

        set.add(*(new Instructions::AddPrimitiveType<int>()));
        std::function<double(double,double)> minus = [](double a, double b)
        {
            return a-b;
        };
        set.add(*(new Instructions::LambdaInstruction<double,double>(minus)));

        e = new Environment(set, vect, 8);
    }

    virtual void TearDown()
    {
        delete e;
        delete (&(vect.at(0).get()));
        delete (&(vect.at(1).get()));
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
    }
};

TEST_F(ProgramTest, ProgramConstructor)
{
    Program::Program* p;
    ASSERT_NO_THROW({ p = new Program::Program(*e,5); })
        << "Something went wrong when constructing a Program with a valid "
           "Environment.";

    ASSERT_NO_THROW({ delete p; })
        << "Something went wrong when destructing a Program with a valid "
           "Environment and empty lines.";
}

TEST_F(ProgramTest, ProgramGetEnvironment)
{
    Program::Program p(*e,5);

    ASSERT_EQ(&p.getEnvironment(), e)
        << "Environment of a Program differs from the one given at "
           "construction. (pointer comparison)";
}

TEST_F(ProgramTest, AddEmptyLineAtKnownPosition)
{
    Program::Program* p = new Program::Program(*e,5);
    Program::Line* l;
    ASSERT_NO_THROW(l = &p->addNewLine(0);)
        << "Inserting a single empty line at position 0 in an empty program "
           "should not be an issue at insertion.";

    // Check that line is set to only zero values.
    ASSERT_EQ(l->getDestinationIndex(), 0)
        << "New line Destination is not set to 0.";
    ASSERT_EQ(l->getInstructionIndex(), 0)
        << "New line Instruction is not set to 0.";
    for (int i = 0; i < e->getMaxNbOperands(); i++) {
        ASSERT_EQ(l->getOperand(i).first, 0)
            << "New line operand source index is not set to 0.";
        ASSERT_EQ(l->getOperand(i).second, 0)
            << "New line operand location is not set to 0.";
    }

    ASSERT_THROW(p->addNewLine(2), std::out_of_range)
        << "Insertion of a line beyond the program end should fail.";

    ASSERT_NO_THROW(p->addNewLine(0);)
        << "Inserting a single empty line at position 0 in a nonempty program "
           "should not be an issue at insertion.";
    ASSERT_NO_THROW(p->addNewLine(p->getNbLines());)
        << "Inserting a single empty line at the end of a nonempty program "
           "should not be an issue at insertion.";
    ASSERT_NO_THROW(p->addNewLine(1);)
        << "Inserting a single empty line in the middle of a nonempty program "
           "should not be an issue at insertion.";

    ASSERT_NO_THROW(delete p;)
        << "Destructing a non empty program should not be an issue.";
}

TEST_F(ProgramTest, AddEmptyLineAndDestruction)
{
    Program::Program* p = new Program::Program(*e,5);
    Program::Line* l;
    ASSERT_NO_THROW(l = &p->addNewLine();)
        << "Inserting a single empty line in an empty program should not be an "
           "issue at insertion.";

    // Check that line is set to only zero values.
    ASSERT_EQ(l->getDestinationIndex(), 0)
        << "New line Destination is not set to 0.";
    ASSERT_EQ(l->getInstructionIndex(), 0)
        << "New line Instruction is not set to 0.";
    for (int i = 0; i < e->getMaxNbOperands(); i++) {
        ASSERT_EQ(l->getOperand(i).first, 0)
            << "New line operand source index is not set to 0.";
        ASSERT_EQ(l->getOperand(i).second, 0)
            << "New line operand location is not set to 0.";
    }

    ASSERT_NO_THROW(delete p;)
        << "Destructing a non empty program should not be an issue.";
}

TEST_F(ProgramTest, CopyConstructor)
{
    Program::Program* p0 = new Program::Program(*e,5);
    Program::Line& l = p0->addNewLine();

    // Initialize some line attributes
    l.setDestinationIndex(1);
    l.setInstructionIndex(1);
    l.setOperand(0, 2, 24);

    // Create a copy of p0.
    Program::Program p1(*p0);

    // Check that environment of the program are the same (pointer)
    ASSERT_EQ(&p0->getEnvironment(), &p1.getEnvironment())
        << "Environment reference was not copied on Program copy construction "
           "(pointer comparison).";
    // Check that line in the program are not the same (pointer)
    ASSERT_NE(&p0->getLine(0), &p1.getLine(0))
        << "Line in the program was not duplicated on Program copy "
           "construction (pointer comparison).";

    // Change the original program to make sure accessed values are duplicates
    // and not pointers to the same data
    l.setDestinationIndex(0);
    l.setInstructionIndex(0);
    l.setOperand(0, 0, 0);

    // Check that line attributes have been duplicated
    // May be redundant with lineTest...?
    ASSERT_EQ(p1.getLine(0).getDestinationIndex(), 1)
        << "Line destinationIndex value was not copied on Program copy.";
    ASSERT_EQ(p1.getLine(0).getInstructionIndex(), 1)
        << "Line instructionIndex value was not copied on Program copy.";
    ASSERT_EQ(p1.getLine(0).getOperand(0).first, 2)
        << "Line operand.dataSource index value was not copied on Program "
           "copy.";
    ASSERT_EQ(p1.getLine(0).getOperand(0).second, 24)
        << "Line operand.location value was not copied on Program copy.";
}

TEST_F(ProgramTest, ProgramSwapLines)
{
    Program::Program p(*e,5);

    std::vector<Program::Line*> lines;
    for (auto i = 0; i < 10; i++) {
        lines.push_back(&p.addNewLine());
    }

    ASSERT_NO_THROW(p.swapLines(2, 7))
        << "Swapping line with valid indexes failed.";
    ASSERT_EQ(lines.at(7), &p.getLine(2))
        << "Swapping line did not give the expected result. (pointer "
           "comparison)";
    ASSERT_EQ(lines.at(2), &p.getLine(7))
        << "Swapping line did not give the expected result. (pointer "
           "comparison)";

    ASSERT_THROW(p.swapLines(3, 10), std::out_of_range)
        << "Swapping lines beyond the program length should throw an "
           "exception.";
}

TEST_F(ProgramTest, getProgramNbLines)
{
    Program::Program p(*e,5);
    ASSERT_EQ(p.getNbLines(), 0) << "Empty program nb lines should be 0.";
    p.addNewLine();
    ASSERT_EQ(p.getNbLines(), 1)
        << "A single line was just added to the Program.";
}

TEST_F(ProgramTest, GetProgramLine)
{
    Program::Program p(*e,5);
    Program::Line& l1 = p.addNewLine();
    Program::Line& l2 = p.addNewLine();
    Program::Line& l3 = p.addNewLine();

    Program::Line& l4 = p.getLine(0);
    ASSERT_EQ(&l4, &l1)
        << "Line retrieved is not the right one (based on pointer comparison).";

    const Program::Program& constP = p;
    const Program::Line& l5 = constP.getLine(1);
    ASSERT_EQ(&l5, &l2)
        << "Line retrieved is not the right one (based on pointer comparison).";

    ASSERT_THROW(p.getLine(3), std::out_of_range)
        << "Getting line outside of the Program did not fail as expected.";
    ASSERT_THROW(constP.getLine(3), std::out_of_range)
        << "Getting line outside of the Program did not fail as expected.";
}

TEST_F(ProgramTest, RemoveProgramLine)
{
    Program::Program p(*e,5);
    Program::Line& l1 = p.addNewLine();
    Program::Line& l2 = p.addNewLine();
    Program::Line& l3 = p.addNewLine();

    ASSERT_NO_THROW(p.removeLine(1))
        << "Could not remove a line with a valid index.";
    ASSERT_EQ(p.getNbLines(), 2)
        << "Program length after removal of a line is incorrect.";
    ASSERT_THROW(p.removeLine(2), std::out_of_range)
        << "Removing a non-existing line should throw an exception.";
}

TEST_F(ProgramTest, identifyIntronsAndIsIntron)
{
    // Create a new environment with instruction accessing arrays
    set.add(
        *new Instructions::LambdaInstruction<const double[2], const double[2]>(
            [](const double a[2], const double b[2]) {
                return a[0] * b[0] + a[1] * b[1];
            }));

    Environment localE(set, vect, 8);

    // Create a program with 2 introns
    Program::Program p(localE,5);
    Program::Line& l1 = p.addNewLine();
    Program::Line& l2 = p.addNewLine();
    Program::Line& l3 = p.addNewLine();
    Program::Line& l4 = p.addNewLine();

    // L4: Register 0 = func(Register {1,2}, DataSource_1{[4],[5]})
    l4.setDestinationIndex(0);
    l4.setOperand(0, 0, 1);
    l4.setOperand(1, 1, 4);
    l4.setInstructionIndex(2); // Lambda

    // L3: Register 3 = Datasource_1[0] + DataSource_1[0] (Intron)
    l3.setDestinationIndex(3);
    l3.setOperand(0, 1, 0);
    l3.setOperand(1, 1, 0);
    l3.setInstructionIndex(0);

    // L2: Register 1 = Datasource_1[2] + DataSource_1[2]
    l2.setDestinationIndex(1);
    l2.setOperand(0, 1, 2);
    l2.setOperand(1, 1, 2);
    l2.setInstructionIndex(0);

    // L1: Register 0 = Register 1 * constant (Intron)
    l1.setDestinationIndex(0);
    l1.setOperand(0, 0, 1);
    l1.setInstructionIndex(1); // MultByConst

    // Identify introns
    uint64_t nbIntrons = 0;
    ASSERT_NO_THROW(nbIntrons = p.identifyIntrons())
        << "Identification of intron lines failed unexpectedly.";
    ASSERT_EQ(nbIntrons, 2)
        << "Number of identified introns is not as expected.";

    // Check which line is an intron
    ASSERT_TRUE(p.isIntron(0))
        << "Line 0 wrongfully detected as not an intron.";
    ASSERT_FALSE(p.isIntron(1)) << "Line 1 wrongfully detected as an intron.";
    ASSERT_TRUE(p.isIntron(2))
        << "Line 2 wrongfully detected as not an intron.";
    ASSERT_FALSE(p.isIntron(3)) << "Line 3 wrongfully detected as an intron.";

    // cleanup
    delete (&set.getInstruction(2));
}

TEST_F(ProgramTest, constants)
{
    // Create a program with constants
    Program::Program p(*e,5);

    //add some constants to the program (-2,-1,0,1)
    for(int j = 0; j < 4; j++)
    {
        p.setConstantAt(j,j-2);
    }

	auto constants = p.getConstantHandler();
	size_t c_size =  0;
	ASSERT_NO_THROW(c_size = constants.getAddressSpace(typeid(Data::Constant))) 
		<< "The accessor to the programs constants failed";
    ASSERT_EQ(p.getConstantsAddressSpace(), c_size)
        << "The returned size of the constant adress space is incorrect";
    //access a constant
    ASSERT_EQ(p.getConstantAt(2), 0)
        << "The accessed constant has the wrong value";
    //access a constant out of range
    ASSERT_THROW(p.getConstantAt(10), std::out_of_range)
        << "Accessing a constant out of range should throw an exception.";
    //modify a constant
    p.setConstantAt(0,5);
    ASSERT_EQ(p.getConstantAt(0), 5)
        << "The value of the constant should have changed";
    //modify a constant out of range
    ASSERT_THROW(p.setConstantAt(10,5), std::out_of_range)
        << "modifying a constant out of range should throw an exception.";
}