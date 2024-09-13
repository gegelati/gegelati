/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2021) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
 * Thomas Bourgoin <tbourgoi@insa-rennes.fr> (2021)
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

#include "data/dataHandler.h"
#include "data/primitiveTypeArray.h"
#include "data/primitiveTypeArray2D.h"
#include "data/untypedSharedPtr.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/lambdaInstruction.h"
#include "instructions/multByConstant.h"
#include "instructions/set.h"
#include "program/line.h"
#include "program/program.h"
#include "program/programExecutionEngine.h"

class ProgramEngineWithMemoryTest : public ::testing::Test
{
  protected:
    const size_t size{32};
    const double value0{1.0};
    std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
    Instructions::Set set;
    Environment* e;
    Program::Program* p;

    virtual void SetUp()
    {
        vect.push_back(
            *(new Data::PrimitiveTypeArray<double>((unsigned int)size)));

        ((Data::PrimitiveTypeArray<double>&)vect.at(0).get())
            .setDataAt(typeid(double), 25, value0);

        set.add(*(new Instructions::AddPrimitiveType<double>()));
        set.add(*(new Instructions::LambdaInstruction<double, double>(
            [](double a, double b) -> double { return a - b; })));

        e = new Environment(set, vect, 8, 0, true);
        p = new Program::Program(*e);

        Program::Line& l0 = p->addNewLine();
        l0.setInstructionIndex(1); // Instruction is addPrimitiveType<double>.
        l0.setOperand(0, 0, 0);    // 1st operand: 6th register.
        l0.setOperand(1, 1, 25);   // 2nd operand: 26th double in the
        // PrimitiveTypeArray of double.
        l0.setDestinationIndex(0); // Destination is register at index 1

        // Mark intron lines
        ASSERT_EQ(p->identifyIntrons(), 0);
    }

    virtual void TearDown()
    {
        delete p;
        delete e;
        delete (&(vect.at(0).get()));
        delete (&set.getInstruction(0));
    }
};

TEST_F(ProgramEngineWithMemoryTest, mapMemoryRegisterSize)
{
    Program::ProgramExecutionEngine progExecEng(*p);

    ASSERT_EQ(progExecEng.getMapMemoryRegisters().size(), 1)
        << "The size of mapMemoryRegisters should be of size 1 after setting "
           "one program.";

    Program::Program p1(*e);
    progExecEng.setProgram(p1);

    Program::Program p2(*e);
    progExecEng.setProgram(p2);

    Program::Program p3(*e);
    progExecEng.setProgram(p3);

    ASSERT_EQ(progExecEng.getMapMemoryRegisters().size(), 4)
        << "The size of mapMemoryRegisters should be of size 1 after setting "
           "four programs.";
}

TEST_F(ProgramEngineWithMemoryTest, mapMemoryRegisterFill)
{

    Program::ProgramExecutionEngine progExecEng(*p);

    progExecEng.setProgram(*p);

    ASSERT_EQ((double)*progExecEng.getMapMemoryRegisters()
                  .at(p)
                  ->getDataAt(typeid(double), 0)
                  .getSharedPointer<const double>(),
              0)
        << "The value of the register should be equal to 0 before iteration";

    progExecEng.iterateThroughtProgram(false);

    ASSERT_EQ((double)*progExecEng.getMapMemoryRegisters()
                  .at(p)
                  ->getDataAt(typeid(double), 0)
                  .getSharedPointer<const double>(),
              -1.0)
        << "The value of the register should be equal to 8.65 after one "
           "iteration";

    progExecEng.setProgram(*p);
    progExecEng.iterateThroughtProgram(false);

    ASSERT_EQ((double)*progExecEng.getMapMemoryRegisters()
                  .at(p)
                  ->getDataAt(typeid(double), 0)
                  .getSharedPointer<const double>(),
              -2.0)
        << "The value of the register should be equal to 8.65 after one "
           "iteration";
}

TEST_F(ProgramEngineWithMemoryTest, mapMemoryRegisterReset)
{

    Program::ProgramExecutionEngine progExecEng(*p);

    progExecEng.setProgram(*p);

    progExecEng.iterateThroughtProgram(false);
    progExecEng.iterateThroughtProgram(false);

    ASSERT_EQ((double)*progExecEng.getMapMemoryRegisters()
                  .at(p)
                  ->getDataAt(typeid(double), 0)
                  .getSharedPointer<const double>(),
              -2.0)
        << "The value of the register should be equal to 8.65 after one "
           "iteration";

    progExecEng.resetAllMemoryRegisters();

    ASSERT_EQ((double)*progExecEng.getMapMemoryRegisters()
                  .at(p)
                  ->getDataAt(typeid(double), 0)
                  .getSharedPointer<const double>(),
              0)
        << "The value of the register should be equal to 8.65 after one "
           "iteration";
}
