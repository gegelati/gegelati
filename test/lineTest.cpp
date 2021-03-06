/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
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

class LineTest : public ::testing::Test
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
        auto minus = [](double a, double b) -> double { return a - b; };
        set.add(*(new Instructions::LambdaInstruction<double, double>(minus)));

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

TEST_F(LineTest, LineConstructor)
{
    Program::Line* l;

    ASSERT_NO_THROW({ l = new Program::Line(*e); })
        << "Something went wrong when constructing a Line with a valid "
           "Environment.";

    ASSERT_NO_THROW({ delete l; }) << "Something went wrong when destructing a "
                                      "Line with a valid Environment.";
}

TEST_F(LineTest, CopyConstructor)
{
    Program::Line* l0 = new Program::Line(*e);

    // Create a copy
    Program::Line* l1;
    ASSERT_NO_THROW(l1 = new Program::Line(*l0);) << "Copy constructor fails.";

    // Check duplication of instructionIndex
    l0->setInstructionIndex(1);
    l0->setDestinationIndex(1);
    l0->setOperand(0, 1, 1);
    ASSERT_EQ(l1->getInstructionIndex(), 0)
        << "The Line instructionIndex was not deeply copied.";
    ASSERT_EQ(l1->getDestinationIndex(), 0)
        << "The Line destinationIndex was not deeply copied.";
    ASSERT_EQ(l1->getOperand(0).first, 0)
        << "The Line operand 0 dataSource index was not deeply copied.";
    ASSERT_EQ(l1->getOperand(0).second, 0)
        << "The Line operand 0 location was not deeply copied.";

    ASSERT_NO_THROW(delete l0;) << "Destructing a copied line failed.";
    ASSERT_NO_THROW(delete l1;) << "Destructing a line copy failed.";
}

TEST_F(LineTest, LineGetEnvironment)
{
    Program::Line l(*e);

    ASSERT_EQ(&l.getEnvironment(), e)
        << "Environment of the Line does not corresponds to the one given at "
           "creation (pointer comparison)";
}

TEST_F(LineTest, LineDestinatioInstructionSetters)
{
    Program::Line l(*e);

    ASSERT_TRUE(l.setDestinationIndex(UINT64_MAX, false))
        << "With checks deactivated, destinationIndex should be successfully "
           "settable to abberant value.";
    ASSERT_FALSE(l.setDestinationIndex(UINT64_MAX))
        << "With checks activated, destinationIndex should not be successfully "
           "settable to abberant value.";
    ASSERT_TRUE(l.setDestinationIndex(5))
        << "Set destinationIndex to valid value failed.";

    ASSERT_TRUE(l.setInstructionIndex(UINT64_MAX, false))
        << "With checks deactivated, instructionIndex should be successfully "
           "settable to abberant value.";
    ASSERT_FALSE(l.setInstructionIndex(UINT64_MAX))
        << "With checks activated, instructionIndex should not be successfully "
           "settable to abberant value.";
    ASSERT_TRUE(l.setInstructionIndex(1))
        << "Set destinationIndex to valid value failed.";
}

TEST_F(LineTest, LineDestinationInstructionGetters)
{
    Program::Line l(*e);

    l.setDestinationIndex(5, false);
    ASSERT_EQ(l.getDestinationIndex(), 5)
        << "Get after set returned the wrong value.";

    l.setInstructionIndex(1, false);
    ASSERT_EQ(l.getInstructionIndex(), 1)
        << "Get after set returned the wrong value.";
}

TEST_F(LineTest, LineOperandAccessors)
{
    Program::Line l(*e); // with the given environment, there are two operands
                         // per line and 3 data sources.

    // There are only 2 operands
    ASSERT_THROW(l.setOperand(2, 0, 0), std::range_error)
        << "Setting value of an incorrectly indexed operand did not fail.";
    // There are only 3 data Sources
    ASSERT_FALSE(l.setOperand(0, 3, 0))
        << "Setting value of a correctly indexed operand with invalid "
           "dataSource index did not fail as expected.";
    // There are only 8 registers but scaling of operand location will take care
    // of this.
    ASSERT_TRUE(l.setOperand(0, 0, 9))
        << "Setting value of a correctly indexed operand with valid dataSource "
           "(registers) and a location larger than the corresponding "
           "largestAddressSpace for this operand pair should succeed.";
    // There are only 24 places in dataSource 1 but scaling of operand location
    // will take care of this.
    ASSERT_TRUE(l.setOperand(1, 1, 24))
        << "Setting value of a correctly indexed operand with valid dataSource "
           "(not registers) and a location larger than the corresponding "
           "largestAddressSpace for this dataHandler (but lower than the "
           "environment largestAddressSpace) should succeed.";
    // There are only 32 places in the largest dataSource of the envirnoment.
    ASSERT_FALSE(l.setOperand(0, 0, 32))
        << "Setting value of a correctly indexed operand with valid dataSource "
           "(not registers) and a location larger than the largestAddressSpace "
           "of the environment should fail.";
    // Deactivate checks
    ASSERT_TRUE(l.setOperand(0, 3, 0, false))
        << "Setting value of a correctly indexed operand with invalid "
           "dataSource index failed when check were deactivated.";
    ASSERT_TRUE(l.setOperand(0, 0, 32, false))
        << "Setting value of a correctly indexed operand with valid dataSource "
           "(not registers) and a location larger than the largestAddressSpace "
           "of the environment should succeed without checks.";

    // Valid Set
    ASSERT_TRUE(l.setOperand(1, 1, 12))
        << "Setting value of a correctly indexed operand (with valid "
           "dataSource index and location) failed.";

    ASSERT_EQ(l.getOperand(1).first, 1)
        << "Get after valid set of operand dataSource index failed.";
    ASSERT_EQ(l.getOperand(1).second, 12)
        << "Get after valid set of operand location failed.";

    // There are only 2 operands
    ASSERT_THROW(l.getOperand(2), std::range_error)
        << "Getting value of an incorrectly indexed operand did not fail.";
}

TEST_F(LineTest, OperatorEquality)
{
    Program::Line l1(*e),
        l2(*e); // with the given environment, there are two operands
                // per line, one param, and 3 data sources.

    ASSERT_EQ(l1, l2)
        << "Lines built with default constructor should be equal.";

    l1.setInstructionIndex(1);

    ASSERT_NE(l1, l2)
        << "Lines built with different instruction should not be equal.";

    l2.setInstructionIndex(1);
    l1.setDestinationIndex(2);

    ASSERT_NE(l1, l2)
        << "Lines built with different destination should not be equal.";

    l2.setDestinationIndex(2);
    l1.setOperand(1, 1, 2);

    ASSERT_NE(l1, l2)
        << "Lines built with different operand should not be equal.";

    l2.setOperand(1, 1, 2);

    ASSERT_EQ(l1, l2)
        << "Lines with identical indexes and operands should be equal.";
}
