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

#include "data/constantHandler.h"
#include "data/dataHandler.h"
#include "data/untypedSharedPtr.h"
#include "instructions/multByConstant.h"
#include "instructions/set.h"

TEST(MultByConstParamTest, ExecutePrimitiveType)
{
    int a{2};
    Data::Constant b{5};
    double c = 4.04;
    std::vector<Data::UntypedSharedPtr> vect;
    vect.emplace_back(&a, Data::UntypedSharedPtr::emptyDestructor<int>());
    vect.emplace_back(
        &b, Data::UntypedSharedPtr::emptyDestructor<Data::Constant>());

    Instructions::MultByConstant<int>* instruction;

    ASSERT_NO_THROW(instruction = new Instructions::MultByConstant<int>())
        << "Constructing a new multByConstParam Instruction failed.";

	ASSERT_EQ(instruction->getNbOperands(), 2)
		<< "The multByConstant Instruction should use two operands.";
    ASSERT_EQ(instruction->execute(vect), 10)
        << "Result returned by the instruction is not as expected.";

    // Execute with wrong types of operands.
    vect.pop_back();
    vect.emplace_back(&c, Data::UntypedSharedPtr::emptyDestructor<double>());
#ifndef NDEBUG
    ASSERT_EQ(instruction->execute(vect), 0.0)
        << "Instructions executed with wrong types of operands should return "
           "0.0";
#else
    ASSERT_THROW(instruction->execute(vect), std::runtime_error)
        << "In NDEBUG mode, execution of a LambdaInstruction with wrong "
           "argument types should fail.";
#endif

    ASSERT_NO_THROW(delete instruction)
        << "Destruction of the MultByConstParam instruction failed.";
}