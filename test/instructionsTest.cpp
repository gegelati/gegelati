/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
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

#include <array>

#include "data/dataHandler.h"
#include "data/untypedSharedPtr.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/lambdaInstruction.h"
#include "instructions/multByConstant.h"
#include "instructions/set.h"

TEST(InstructionsTest, ConstructorDestructorCall)
{
    ASSERT_NO_THROW({
        Instructions::Instruction* i =
            new Instructions::AddPrimitiveType<double>();
        delete i;
    }) << "Call to constructor for AddPrimitiveType<double> failed.";

    ASSERT_NO_THROW({
        Instructions::Instruction* i =
            new Instructions::AddPrimitiveType<int>();
        delete i;
    }) << "Call to constructor for AddPrimitiveType<int> failed.";
}

TEST(InstructionsTest, OperandListAndNbParam)
{
    Instructions::Instruction* i = new Instructions::AddPrimitiveType<double>();
    ASSERT_EQ(i->getNbOperands(), 2)
        << "Number of operands of Instructions::AddPrimitiveType<double> is "
           "different from 2";
    auto operands = i->getOperandTypes();
    ASSERT_EQ(operands.size(), 2)
        << "Operand list of AddPrimitiveType<double> is different from 2";
    ASSERT_STREQ(operands.at(0).get().name(), typeid(double).name())
        << "First operand of AddPrimitiveType<double> is not\""
        << typeid(double).name() << "\".";
    ASSERT_STREQ(operands.at(1).get().name(), typeid(double).name())
        << "Second operand of AddPrimitiveType<double> is not\""
        << typeid(double).name() << "\".";
    delete i;
}

TEST(InstructionsTest, CheckArgumentTypes)
{
    Instructions::Instruction* i = new Instructions::AddPrimitiveType<double>();
    double a{2.5};
    double b = 5.6;
    double c = 3.7;
    int d = 5;

    std::vector<Data::UntypedSharedPtr> vect;

    vect.emplace_back(&a, Data::UntypedSharedPtr::emptyDestructor<double>());
    vect.emplace_back(&b, Data::UntypedSharedPtr::emptyDestructor<double>());
    ASSERT_TRUE(i->checkOperandTypes(vect))
        << "Operands of valid types wrongfully classified as invalid.";
    vect.emplace_back(&c, Data::UntypedSharedPtr::emptyDestructor<double>());
    ASSERT_FALSE(i->checkOperandTypes(vect))
        << "Operands list of too long size wrongfully classified as valid.";
    vect.pop_back();
    vect.pop_back();
    vect.emplace_back(&d, Data::UntypedSharedPtr::emptyDestructor<int>());
    ASSERT_FALSE(i->checkOperandTypes(vect))
        << "Operands of invalid types wrongfully classified as valid";
    delete i;
}

TEST(InstructionsTest, Execute)
{
    Instructions::Instruction* i = new Instructions::AddPrimitiveType<double>();
    double a{2.6};
    double b = 5.5;
    int c = 3;

    std::vector<Data::UntypedSharedPtr> vect;
    vect.emplace_back(&a, Data::UntypedSharedPtr::emptyDestructor<double>());
    vect.emplace_back(&b, Data::UntypedSharedPtr::emptyDestructor<double>());
    ASSERT_EQ(i->execute(vect), 8.1)
        << "Execute method of AddPrimitiveType<double> returns an incorrect "
           "value with valid operands.";

    vect.pop_back();
    vect.emplace_back(&c, Data::UntypedSharedPtr::emptyDestructor<int>());
#ifndef NDEBUG
    ASSERT_EQ(i->execute(vect), 0.0)
        << "Execute method of AddPrimitiveType<double> returns an incorrect "
           "value with invalid operands.";
#else
    ASSERT_THROW(i->execute(vect), std::runtime_error)
        << "In NDEBUG mode, execute method of AddPrimitiveType<double> should "
           "throw an exception with invalid operands.";
#endif
    delete i;
}

TEST(InstructionsTest, SetAdd)
{
    Instructions::Set s;

    Instructions::AddPrimitiveType<double> i1;
    Instructions::AddPrimitiveType<double> i2;
    Instructions::AddPrimitiveType<float> i3;

    ASSERT_TRUE(s.add(i1))
        << "Add of instruction to empty Instructions::Set failed.";
    // Adding equivalent instructions is no longer forbidden.
    ASSERT_TRUE(s.add(i2)) << "Add of instruction already present in an "
                              "Instructions::Set should not fail.";
    ASSERT_TRUE(s.add(i3))
        << "Add of instruction to non empty Instructions::Set failed. (with a "
           "template instruction with different template param than an already "
           "present one";
}

TEST(InstructionsTest, SetGetNbInstruction)
{
    Instructions::Set s;

    ASSERT_EQ(s.getNbInstructions(), 0)
        << "Incorrect number of instructions in an empty Set.";

    Instructions::AddPrimitiveType<double> i1;
    Instructions::AddPrimitiveType<float> i2;
    s.add(i1);
    s.add(i2);
    ASSERT_EQ(s.getNbInstructions(), 2)
        << "Incorrect number of instructions in a non-empty Set.";
}

TEST(InstructionsTest, SetGetInstruction)
{
    Instructions::Set s;

    Instructions::AddPrimitiveType<float> iAdd;
    Instructions::AddPrimitiveType<double> iAddDouble;
    s.add(iAdd);
    s.add(iAddDouble);

    const Instructions::Instruction* res;
    ASSERT_NO_THROW(res = &s.getInstruction(1))
        << "Exception was thrown unexpectedly when calling Set::getInstruction "
           "with a valid index.";

    // Compare that the returned reference points to the right object.
    ASSERT_EQ(res, &iAddDouble)
        << "Incorrect Instruction was returned by valid Set::getInstruction.";

    // Check that exception is thrown when an invalid index is given.
    ASSERT_THROW(res = &s.getInstruction(2), std::out_of_range)
        << "Exception was not thrown when calling Set::getInstruction with an "
           "invalid index.";
}

TEST(InstructionsTest, SetGetNbMaxOperands)
{
    Instructions::Set s;

    ASSERT_EQ(s.getMaxNbOperands(), 0)
        << "Max number of operands returned by the empty Instructions::Set is "
           "incorrect.";

    Instructions::AddPrimitiveType<float> iAdd; // one operand
    auto minus = [](double a, double b) -> double {
        return a - b;
    }; // two operands

    s.add(iAdd);
    s.add(Instructions::LambdaInstruction<double, double>(minus));

    ASSERT_EQ(s.getMaxNbOperands(), 2) << "Max number of operands returned by "
                                          "the Instructions::Set is incorrect.";
}

#ifdef CODE_GENERATION
TEST(InstructionsTest, ConstructorDestructorCallPrint)
{
    ASSERT_NO_THROW({
        Instructions::Instruction* i =
            new Instructions::AddPrimitiveType<double>("$0 = $1 + $2;");
        delete i;
    }) << "Call to constructor for AddPrimitiveType<double>(std::string "
          "printTemplate) failed.";
}

TEST(InstructionsTest, isPrintable)
{
    Instructions::Instruction* i = new Instructions::AddPrimitiveType<double>();
    ASSERT_EQ(i->isPrintable(), false);
    delete i;

    i = new Instructions::AddPrimitiveType<double>("$0 = $1 + $2;");
    ASSERT_EQ(i->isPrintable(), true);
    delete i;
}

TEST(InstructionsTest, getPrintTemplate)
{
    std::string printTemplate{"$0 = $1 + $2;"};
    Instructions::Instruction* i =
        new Instructions::AddPrimitiveType<double>(printTemplate);
    ASSERT_EQ(i->getPrintTemplate(), printTemplate);
    delete i;
}

TEST(InstructionsTest, getPrintablePrimitiveOperandType)
{
    Instructions::Instruction* i = new Instructions::MultByConstant<double>();
    ASSERT_EQ(i->getPrintablePrimitiveOperandType(0), "double");
    ASSERT_EQ(i->getPrintablePrimitiveOperandType(1), "int32_t");
    delete i;
}

#endif // CODE_GENERATION