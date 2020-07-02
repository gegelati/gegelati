/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2020)
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
#include "instructions/multByConstParam.h"
#include "instructions/set.h"

TEST(LambdaInstructionsTest, ExecutePrimitiveType)
{
    double a{2.6};
    double b = 5.5;
    int c = 3;

    std::vector<Data::UntypedSharedPtr> vect;
    vect.emplace_back(&a, Data::UntypedSharedPtr::emptyDestructor<double>());
    vect.emplace_back(&b, Data::UntypedSharedPtr::emptyDestructor<double>());

    auto minus = [](double a, double b) { return a - b; };

    Instructions::LambdaInstruction<double, double>* instruction;
    ASSERT_NO_THROW(
        (instruction =
             new Instructions::LambdaInstruction<double, double>(minus)))
        << "Constructing a new lambdaInstruction failed.";

    ASSERT_EQ(instruction->execute({}, vect), -2.9)
        << "Result returned by the instruction is not as expected.";

    // Execute with wrong types of operands.
    vect.pop_back();
    vect.emplace_back(&c, Data::UntypedSharedPtr::emptyDestructor<int>());
#ifndef NDEBUG
    ASSERT_EQ(instruction->execute({}, vect), 0.0)
        << "Instructions executed with wrong types of operands should return "
           "0.0";
#else
    ASSERT_THROW(instruction->execute({}, vect), std::runtime_error)
        << "In NDEBUG mode, execution of a LambdaInstruction with wrong "
           "argument types should fail.";
#endif

    ASSERT_NO_THROW(delete instruction)
        << "Destruction of the LambdaInstruction failed.";
}

#define arrayA 1.1, 2.2, 3.3
#define arrayB 6.5, 4.3, 2.1
TEST(LambdaInstructionsTest, ExecuteArray)
{
    double arrA[3]{arrayA};
    double arrB[3]{arrayB};

    std::function<double(const double[3], const double[3])> mac =
        [](const double a[3], const double b[3]) {
            return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
        };

    // Build the instruction
    Instructions::LambdaInstruction<const double[3], const double[3]>*
        instruction;
    ASSERT_NO_THROW((instruction = new Instructions::LambdaInstruction<
                         const double[3], const double[3]>(mac)));
    ASSERT_NE(instruction, nullptr);

    // Test execution
    std::vector<Data::UntypedSharedPtr> arguments;
    arguments.emplace_back(
        std::make_shared<Data::UntypedSharedPtr::Model<const double[]>>(
            new double[3]{arrayA}));
    arguments.emplace_back(
        std::make_shared<Data::UntypedSharedPtr::Model<const double[]>>(
            new double[3]{arrayB}));
    ASSERT_EQ(instruction->execute({}, arguments), 23.54)
        << "Result returned by the instruction is not as expected.";
}

TEST(LambdaInstructionsTest, ExecuteAllTypesMixed)
{

    // Test with mixed primitive types
    std::function<double(double, double, int)> func1 =
        [](double d, double e, int i) { return (d + e) * i; };
    Instructions::LambdaInstruction<double, double, int> instruction1(func1);

    std::vector<Data::UntypedSharedPtr> vect;
    double a = 1.0;
    double b = 1.1;
    int c = 2;

    vect.emplace_back(&a, Data::UntypedSharedPtr::emptyDestructor<double>());
    vect.emplace_back(&b, Data::UntypedSharedPtr::emptyDestructor<double>());
    vect.emplace_back(&c, Data::UntypedSharedPtr::emptyDestructor<int>());
    ASSERT_EQ(instruction1.execute({}, vect), 4.2)
        << "Result of the LambdaInstruction with heterogeneous primitive "
           "argument types is incorrect.";

    // Test with mixed primitive types and c-style array
    std::function<double(const double[2], double, const int[1])> func2 =
        [](const double d[2], double e, const int i[1]) {
            return (d[1] + d[0] + e) * *i;
        };
    Instructions::LambdaInstruction<const double[2], double, const int[]>
        instruction2(func2);

    std::vector<Data::UntypedSharedPtr> vect2;
    vect2.emplace_back(
        std::make_shared<Data::UntypedSharedPtr::Model<const double[]>>(
            new double[2]{1.0, 2.0}));
    vect2.emplace_back(&b, Data::UntypedSharedPtr::emptyDestructor<double>());
    vect2.emplace_back(
        std::make_shared<Data::UntypedSharedPtr::Model<const int[]>>(
            new int[1]{2}));

    ASSERT_EQ(instruction2.execute({}, vect2), 8.2)
        << "Result of the LambdaInstruction with heterogeneous argument types "
           "is incorrect.";

    // Test wrong number of argument detection.
    vect2.pop_back();
#ifndef NDEBUG
    ASSERT_EQ(instruction2.execute({}, vect2), 0.0)
        << "Result of the LambdaInstruction with wrong number of arguments "
           "should be 0.";
#else
    ASSERT_THROW(instruction2.execute({}, vect2), std::runtime_error)
        << "In NDEBUG mode, execution of a LambdaInstruction with wrong number "
           "of arguments should fail.";
#endif

    // Test wrong argument type
    vect2.emplace_back(&c, Data::UntypedSharedPtr::emptyDestructor<int>());
#ifndef NDEBUG
    ASSERT_EQ(instruction2.execute({}, vect2), 0.0)
        << "Result of the LambdaInstruction with wrong argument types should "
           "be 0.";
#else
    ASSERT_THROW(instruction2.execute({}, vect2), std::runtime_error)
        << "In NDEBUG mode, execution of a LambdaInstruction with wrong "
           "argument types should fail.";
    ;
#endif
}
