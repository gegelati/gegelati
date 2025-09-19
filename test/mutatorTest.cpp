/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2024 - 2025)
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

#include <algorithm>
#include <set>
#include <vector>

#include "data/dataHandler.h"
#include "data/primitiveTypeArray.h"
#include "environment.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/instruction.h"
#include "instructions/lambdaInstruction.h"
#include "instructions/multByConstant.h"
#include "mutator/lineMutator.h"
#include "mutator/programMutator.h"
#include "mutator/rng.h"
#include "mutator/tpgMutator.h"
#include "program/line.h"
#include "program/program.h"
#include "program/programExecutionEngine.h"
#include "selector/tournamentSelector.h"
#include "tpg/tpgExecutionEngine.h"
#include "tpg/tpgGraph.h"
#include "util/counterReset.h"

class MutatorTest : public ::testing::Test
{
  protected:
    const size_t size1{24};
    const size_t size2{32};
    const double value0{2.3};
    const float value1{4.2f};
    std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
    Instructions::Set set;
    Environment* e;
    Learn::LearningParameters params;
    Program::Program* p;
    std::shared_ptr<Program::Program> progPointer;

    MutatorTest() : e{nullptr}, p{nullptr} {};

    virtual void SetUp()
    {

        CounterReset::counterReset();
        vect.push_back(
            *(new Data::PrimitiveTypeArray<int>((unsigned int)size1)));
        vect.push_back(
            *(new Data::PrimitiveTypeArray<double>((unsigned int)size2)));

        ((Data::PrimitiveTypeArray<double>&)vect.at(1).get())
            .setDataAt(typeid(double), 25, value0);

        std::function<double(double, double)> minus =
            [](double a, double b) -> double { return a - b; };
        std::function<double(double, double)> add =
            [](double a, double b) -> double { return a + b; };

        set.add(*(new Instructions::MultByConstant<double>()));
        set.add(*(new Instructions::AddPrimitiveType<double>()));
        set.add(*(new Instructions::LambdaInstruction<double, double>(minus)));
        set.add(*(new Instructions::LambdaInstruction<double, double>(add)));

        // the environment and the programs have 5 Constant parameters
        params.nbRegisters = 8;
        params.nbProgramConstant = 5;
        e = new Environment(set, params, vect);
        p = new Program::Program(*e, false);
        progPointer =
            std::shared_ptr<Program::Program>(new Program::Program(*e, false));
    }

    virtual void TearDown()
    {
        delete p;
        delete e;
        delete (&(vect.at(0).get()));
        delete (&(vect.at(1).get()));
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
        delete (&set.getInstruction(2));
        delete (&set.getInstruction(3));
    }
};

TEST_F(MutatorTest, RNG)
{
    Mutator::RNG rng;
    rng.setSeed(0);

    // With this seed, the current pseudo-random number generator returns 24
    // on its first use
    ASSERT_EQ(rng.getUnsignedInt64(0, 100), 24)
        << "Returned pseudo-random value changed with a known seed.";

    ASSERT_EQ(rng.getDouble(0, 1.0), 0.99214520962982877)
        << "Returned pseudo-random value changed with a known seed.";
}

TEST_F(MutatorTest, LineMutatorInitRandomCorrectLine1)
{
    Mutator::RNG rng;
    rng.setSeed(0);

    // Add a pseudo-random lines to the program
    Program::Line& l0 = p->addNewLine();
    ASSERT_NO_THROW(Mutator::LineMutator::initRandomCorrectLine(l0, rng))
        << "Pseudo-Random correct line initialization failed within an "
           "environment where failure should not be possible.";
    // With this known seed
    // InstructionIndex=3 > lambda instruction (plus)
    // DestinationIndex=6
    // Operand 0= (0, 12) => 12th register
    // Covers: correct instruction, correct operand type (register), additional
    // uneeded operand (not register)
    ASSERT_EQ(l0.getInstructionIndex(), 3)
        << "Selected pseudo-random instructionIndex changed with a known seed.";
    ASSERT_EQ(l0.getDestinationIndex(), 6)
        << "Selected pseudo-random destinationIndex changed with a known seed.";
    ASSERT_EQ(l0.getOperand(0).first, 0)
        << "Selected pseudo-random operand data source index changed with a "
           "known seed.";
    ASSERT_EQ(l0.getOperand(0).second, 12)
        << "Selected pseudo-random operand location changed with a known seed.";

    // Add another pseudo-random lines to the program
    Program::Line& l1 = p->addNewLine();
    // Additionally covers correct operand type from data source
    // Instruction if lambda instruction(plus)
    // first operand is register 0
    ASSERT_NO_THROW(Mutator::LineMutator::initRandomCorrectLine(l1, rng))
        << "Pseudo-Random correct line initialization failed within an "
           "environment where failure should not be possible.";
    ASSERT_EQ(l1.getInstructionIndex(), 3)
        << "Selected pseudo-random instructionIndex changed with a known seed.";
    ASSERT_EQ(l1.getOperand(0).first, 0)
        << "Selected pseudo-random operand data source index changed with a "
           "known seed.";

    // Add another pseudo-random lines to the program
    // Additionally covers nothing
    Program::Line& l2 = p->addNewLine();
    Program::Line& l3 = p->addNewLine();
    ASSERT_NO_THROW(Mutator::LineMutator::initRandomCorrectLine(l2, rng))
        << "Pseudo-Random correct line initialization failed within an "
           "environment where failure should not be possible.";
    ASSERT_NO_THROW(Mutator::LineMutator::initRandomCorrectLine(l3, rng))
        << "Pseudo-Random correct line initialization failed within an "
           "environment where failure should not be possible.";

    // Add another pseudo-random lines to the program
    Program::Line& l4 = p->addNewLine();
    // Additionally covers additional uneeded operand (register)
    ASSERT_NO_THROW(Mutator::LineMutator::initRandomCorrectLine(l4, rng))
        << "Pseudo-Random correct line initialization failed within an "
           "environment where failure should not be possible.";
    ASSERT_EQ(l4.getInstructionIndex(), 3)
        << "Selected pseudo-random instructionIndex changed with a known seed.";
    ASSERT_EQ(l4.getOperand(1).first, 3)
        << "Selected pseudo-random operand data source index changed with a "
           "known seed.";

    Program::ProgramExecutionEngine progEngine(*p);
    ASSERT_NO_THROW(progEngine.executeProgram(false))
        << "Program with only correct random lines is unexpectedly not "
           "correct.";
}

TEST_F(MutatorTest, LineMutatorAlterLine)
{
    Mutator::RNG rng;
    Program::ProgramExecutionEngine pEE(*p);

    // Add a 0 lines to the program
    // i=0, d=0, op0=(0,0), op1=(0,0)
    Program::Line& l0 = p->addNewLine();

    // Alter instruction
    // i=, d=0, op0=(0,0), op1=(0,0)
    rng.setSeed(5);
    ASSERT_NO_THROW(Mutator::LineMutator::alterCorrectLine(l0, rng))
        << "Line mutation of a correct instruction should not throw.";
    ASSERT_EQ(l0.getInstructionIndex(), 2)
        << "Alteration with known seed changed its result.";
    ASSERT_NO_THROW(pEE.executeProgram()) << "Altered line is not executable.";

    // Alter destination
    // i=2, d=3, op0=(0,0), op1=(0,0)
    rng.setSeed(29);
    ASSERT_NO_THROW(Mutator::LineMutator::alterCorrectLine(l0, rng))
        << "Line mutation of a correct instruction should not throw.";
    ASSERT_EQ(l0.getDestinationIndex(), 3)
        << "Alteration with known seed changed its result.";
    ASSERT_NO_THROW(pEE.executeProgram()) << "Altered line is not executable.";

    // Alter operand 0 data source
    // i=2, d=3, op0=(0,0), op1=(0,0)
    rng.setSeed(8);
    ASSERT_NO_THROW(Mutator::LineMutator::alterCorrectLine(l0, rng))
        << "Line mutation of a correct instruction should not throw.";
    ASSERT_EQ(l0.getOperand(0).first, 3)
        << "Alteration with known seed changed its result.";
    ASSERT_NO_THROW(pEE.executeProgram()) << "Altered line is not executable.";

    // Alter operand 0 location
    // i=2, d=3, op0=(0,17), op1=(0,0)
    rng.setSeed(1);
    ASSERT_NO_THROW(Mutator::LineMutator::alterCorrectLine(l0, rng))
        << "Line mutation of a correct instruction should not throw.";
    ASSERT_EQ(l0.getOperand(0).second, 17)
        << "Alteration with known seed changed its result.";
    ASSERT_NO_THROW(pEE.executeProgram()) << "Altered line is not executable.";

    // Alter operand 1 data source
    // i=2, d=3, op0=(0,17), op1=(8,0)
    rng.setSeed(320);
    ASSERT_NO_THROW(Mutator::LineMutator::alterCorrectLine(l0, rng))
        << "Line mutation of a correct instruction should not throw.";
    ASSERT_EQ(l0.getOperand(1).first, 0)
        << "Alteration with known seed changed its result.";
    ASSERT_NO_THROW(pEE.executeProgram()) << "Altered line is not executable.";

    // Alter operand 1 location
    // i=2, d=3, op0=(0,17), op1=(0,8)
    rng.setSeed(2);
    ASSERT_NO_THROW(Mutator::LineMutator::alterCorrectLine(l0, rng))
        << "Line mutation of a correct instruction should not throw.";
    ASSERT_EQ(l0.getOperand(1).second, 28)
        << "Alteration with known seed changed its result.";
    ASSERT_NO_THROW(pEE.executeProgram()) << "Altered line is not executable.";

    // Alter instruction index
    // i=4, d=0, op0=(0,28), op1=(0,8)
    rng.setSeed(6);
    ASSERT_NO_THROW(Mutator::LineMutator::alterCorrectLine(l0, rng))
        << "Line mutation of a correct instruction should not throw.";
    ASSERT_EQ(l0.getInstructionIndex(), 2)
        << "Alteration with known seed changed its result.";
    ASSERT_EQ(l0.getDestinationIndex(), 4)
        << "Alteration with known seed changed its result.";
    ASSERT_EQ(l0.getOperand(0).first, 3)
        << "Alteration with known seed changed its result.";
    ASSERT_EQ(l0.getOperand(0).second, 17)
        << "Alteration with known seed changed its result.";
    ASSERT_EQ(l0.getOperand(1).first, 0)
        << "Alteration with known seed changed its result.";
    ASSERT_EQ(l0.getOperand(1).second, 28)
        << "Alteration with known seed changed its result.";
    ASSERT_NO_THROW(pEE.executeProgram()) << "Altered line is not executable.";
}

TEST_F(MutatorTest, LineMutatorAlterLineWithCompositeOperands)
{
    Mutator::RNG rng;

    // Setup for this test
    set.add(
        *(new Instructions::LambdaInstruction<const double[3], const double[3]>(
            [](const double* a, const double* b) -> double {
                return (a[0] - b[0] + a[1] - b[1] + a[2] - b[2]) / 3.0;
            })));

    Environment e2(set, params, vect);
    Program::Program p2(e2, false);

    Program::ProgramExecutionEngine pEE(p2);

    // Add a 0 line to the program
    // i=0, d=0, op0=(0,0), op1=(0,0)
    Program::Line& l0 = p2.addNewLine();

    // Alter instruction
    // i=2, d=0, op0=(0,0), op1=(0,0)
    rng.setSeed(5);
    ASSERT_NO_THROW(Mutator::LineMutator::alterCorrectLine(l0, rng))
        << "Line mutation of a correct instruction should not throw.";
    ASSERT_EQ(l0.getInstructionIndex(), 1)
        << "Alteration with known seed changed its result.";
    ASSERT_NO_THROW(pEE.executeProgram()) << "Altered line is not executable.";

    // Alter op1 location
    // i=2, d=0, op0=(0,0), op1=(0,16),  param=0
    rng.setSeed(4);
    ASSERT_NO_THROW(Mutator::LineMutator::alterCorrectLine(l0, rng))
        << "Line mutation of a correct instruction should not throw.";
    ASSERT_EQ(l0.getOperand(1).second, 16)
        << "Alteration with known seed changed its result.";
    ASSERT_NO_THROW(pEE.executeProgram()) << "Altered line is not executable.";

    // Alter op0 source
    // i=2, d=0, op0=(3,0), op1=(0,16),  param=0
    rng.setSeed(3);
    ASSERT_NO_THROW(Mutator::LineMutator::alterCorrectLine(l0, rng))
        << "Line mutation of a correct instruction should not throw.";
    ASSERT_EQ(l0.getOperand(0).first, 3)
        << "Alteration with known seed changed its result.";
    ASSERT_NO_THROW(pEE.executeProgram()) << "Altered line is not executable.";

    // Teardown for this test
    delete &set.getInstruction(4);
}

TEST_F(MutatorTest, ProgramMutatorDeleteRandomLine)
{
    const uint64_t nbLines = 10;
    Mutator::RNG rng;
    rng.setSeed(0);

    // Attempt removing on an empty program
    ASSERT_FALSE(Mutator::ProgramMutator::deleteRandomLine(*p, rng));
    ASSERT_EQ(p->getNbLines(), 0);

    // Attempt removing on a program with a single line
    p->addNewLine();
    ASSERT_FALSE(Mutator::ProgramMutator::deleteRandomLine(*p, rng));
    ASSERT_EQ(p->getNbLines(), 1);

    // Insert lines
    for (auto i = 0; i < nbLines - 1; i++) {
        p->addNewLine();
    }

    // Delete a random line
    ASSERT_TRUE(Mutator::ProgramMutator::deleteRandomLine(*p, rng));
    ASSERT_EQ(p->getNbLines(), nbLines - 1);
}

TEST_F(MutatorTest, ProgramMutatorInsertRandomLine)
{
    Mutator::RNG rng;
    rng.setSeed(0);

    // Insert in empty program
    ASSERT_NO_THROW(Mutator::ProgramMutator::insertRandomLine(*p, rng));
    ASSERT_EQ(p->getNbLines(), 1)
        << "Line insertion in an empty program failed.";

    // Insert in non empty program
    // in first position (with known seed)
    rng.setSeed(0);
    ASSERT_NO_THROW(Mutator::ProgramMutator::insertRandomLine(*p, rng));
    ASSERT_EQ(p->getNbLines(), 2)
        << "Line insertion in a non-empty program failed.";

    // Insert in non empty program
    // After last position (with known seed)
    rng.setSeed(1);
    ASSERT_NO_THROW(Mutator::ProgramMutator::insertRandomLine(*p, rng));
    ASSERT_EQ(p->getNbLines(), 3)
        << "Line insertion in a non-empty program failed.";

    // Insert in non empty program
    // In the middle position (with known seed)
    rng.setSeed(5);
    ASSERT_NO_THROW(Mutator::ProgramMutator::insertRandomLine(*p, rng));
    ASSERT_EQ(p->getNbLines(), 4)
        << "Line insertion in a non-empty program failed.";
}

TEST_F(MutatorTest, ProgramMutatorSwapRandomLines)
{
    Mutator::RNG rng;
    rng.setSeed(0);

    std::vector<Program::Line*> lines;
    // Nothing on empty program
    ASSERT_FALSE(Mutator::ProgramMutator::swapRandomLines(*p, rng));

    // Add a first line
    lines.push_back(&p->addNewLine());

    // Nothing on program with one line.
    ASSERT_FALSE(Mutator::ProgramMutator::swapRandomLines(*p, rng));

    // Add a second line
    lines.push_back(&p->addNewLine());

    // Exchanges the two line.
    ASSERT_TRUE(Mutator::ProgramMutator::swapRandomLines(*p, rng));
    ASSERT_EQ(lines.at(0), &p->getLine(1));
    ASSERT_EQ(lines.at(1), &p->getLine(0));

    // Add 8 lines
    for (auto i = 0; i < 8; i++) {
        lines.push_back(&p->addNewLine());
    }
    // Swap two random lines (with a known seed)
    ASSERT_TRUE(Mutator::ProgramMutator::swapRandomLines(*p, rng));
    // Only lines 4 and 7 are swapped
    ASSERT_EQ(lines.at(0), &p->getLine(1));
    ASSERT_EQ(lines.at(1), &p->getLine(0));
    ASSERT_EQ(lines.at(2), &p->getLine(2));
    ASSERT_EQ(lines.at(3), &p->getLine(3));
    ASSERT_EQ(lines.at(4), &p->getLine(7));
    ASSERT_EQ(lines.at(5), &p->getLine(5));
    ASSERT_EQ(lines.at(6), &p->getLine(6));
    ASSERT_EQ(lines.at(7), &p->getLine(4));
    ASSERT_EQ(lines.at(8), &p->getLine(8));
    ASSERT_EQ(lines.at(9), &p->getLine(9));
}

TEST_F(MutatorTest, ProgramMutatorAlterRandomLine)
{
    Mutator::RNG rng;
    rng.setSeed(0);

    // Nothing on empty program
    ASSERT_FALSE(Mutator::ProgramMutator::alterRandomLine(*p, rng));
    // Add 10 lines
    for (auto i = 0; i < 10; i++) {
        p->addNewLine();
    }
    // Alter a randomly selected line (with a known seed)
    // Parameter of Line 4 is altered.
    ASSERT_TRUE(Mutator::ProgramMutator::alterRandomLine(*p, rng));
}

TEST_F(MutatorTest, ProgramMutatorInitProgram)
{
    Mutator::RNG rng;
    rng.setSeed(0);

    Mutator::MutationParameters params;
    params.prog.initMaxProgramSize = 96;
    params.prog.initMinProgramSize = 1;
    params.prog.maxConstValue = 10;
    params.prog.minConstValue = 0;

    ASSERT_NO_THROW(Mutator::ProgramMutator::initRandomProgram(*p, params, rng))
        << "Empty Program Random init failed";
    ASSERT_EQ(p->getNbLines(), 15)
        << "Random number of line is not as expected (with known seed).";

    ASSERT_NO_THROW(Mutator::ProgramMutator::initRandomProgram(*p, params, rng))
        << "Non-Empty Program Random init failed";
    ASSERT_EQ(p->getNbLines(), 38)
        << "Random number of line is not as expected (with known seed).";

    // Count lines marked as introns (with a known seed).
    uint64_t nbIntrons = 0;
    for (auto i = 0; i < p->getNbLines(); i++) {
        if (p->isIntron(i)) {
            nbIntrons++;
        }
    }

    // Check nb intron lines with a known seed.
    ASSERT_EQ(nbIntrons, 36);
}

TEST_F(MutatorTest, ProgramMutatorMutateBehavior)
{
    Mutator::RNG rng;
    // specific for this test
    // we need an instruction with three operands to
    // trigger a special condition in LineMutator
    set.add(*(new Instructions::LambdaInstruction<const double, const double,
                                                  const double>(
        [](const double a, const double b, const double c) -> double {
            return (cos(a + b + c));
        })));

    Environment e2(set, params, vect);
    Program::Program p2(e2, false);

    Program::ProgramExecutionEngine pEE(p2);

    rng.setSeed(14);
    Program::Line& l = p2.addNewLine();
    Mutator::LineMutator::initRandomCorrectLine(l, rng);
    Program::Line& l2 = p2.addNewLine();
    Mutator::LineMutator::initRandomCorrectLine(l2, rng);
    Program::Line& l3 = p2.addNewLine();
    Mutator::LineMutator::initRandomCorrectLine(l3, rng);

    Mutator::MutationParameters params;
    params.prog.maxProgramSize = 15;
    params.prog.pDelete = 0.5;
    params.prog.pAdd = 0.0;
    params.prog.pMutate = 0.0;
    params.prog.pSwap = 0.0;
    params.prog.maxConstValue = 1;
    params.prog.minConstValue = 0;
    params.prog.pConstantMutation = 0.2;

    rng.setSeed(0);
    ASSERT_TRUE(Mutator::ProgramMutator::mutateProgram(p2, params, rng))
        << "Mutation did not occur with known seed.";
    ASSERT_EQ(p2.getNbLines(), 2)
        << "Wrong program mutation occured. Expected: Line deletion.";

    params.prog.pDelete = 0.0;
    params.prog.pAdd = 0.5;
    rng.setSeed(1);
    ASSERT_TRUE(Mutator::ProgramMutator::mutateProgram(p2, params, rng))
        << "Mutation did not occur with known seed.";
    ASSERT_EQ(p2.getNbLines(), 3)
        << "Wrong program mutation occured. Expected: Line insertion.";

    params.prog.pAdd = 0.0;
    params.prog.pMutate = 0.01;
    rng.setSeed(86);
    ASSERT_TRUE(Mutator::ProgramMutator::mutateProgram(p2, params, rng))
        << "Mutation did not occur with known seed.";

    params.prog.pMutate = 0.00;
    params.prog.pSwap = 0.1;
    rng.setSeed(1);
    ASSERT_TRUE(Mutator::ProgramMutator::mutateProgram(p2, params, rng))
        << "Mutation did not occur with known seed.";

    // mutate other instructions
    params.prog.pSwap = 0.0;
    params.prog.pMutate = 1;
    rng.setSeed(114);
    ASSERT_TRUE(Mutator::ProgramMutator::mutateProgram(p2, params, rng))
        << "Mutation did not occur with known seed.";

    // Teardown for this test
    delete &set.getInstruction(4);
}

TEST_F(MutatorTest, TPGMutatorInitRandomTPG)
{
    Mutator::RNG rng;
    rng.setSeed(0);
    TPG::TPGGraph tpg(*e);
    Mutator::MutationParameters params;

    uint64_t nbActions = 5;
    params.tpg.maxInitOutgoingEdges = 4;
    params.prog.maxProgramSize = 96;
    params.prog.pConstantMutation = 0.5;
    params.prog.minConstValue = 0;
    params.prog.maxConstValue = 1;

    ASSERT_NO_THROW(
        Mutator::TPGMutator::initRandomTPG(tpg, params, rng, nbActions))
        << "TPG Initialization failed.";
    auto vertexSet = tpg.getVertices();
    // Check number or vertex, roots, actions, teams, edges
    ASSERT_EQ(vertexSet.size(), nbActions + params.tpg.nbRoots)
        << "Number of vertices after initialization is incorrect.";
    ASSERT_EQ(tpg.getRootVertices().size(), params.tpg.nbRoots)
        << "Number of root vertices after initialization is incorrect.";
    ASSERT_EQ(std::count_if(vertexSet.begin(), vertexSet.end(),
                            [](const TPG::TPGVertex* vert) {
                                return dynamic_cast<const TPG::TPGAction*>(
                                           vert) != nullptr;
                            }),
              nbActions)
        << "Number of action vertex in the graph is incorrect.";
    ASSERT_EQ(std::count_if(vertexSet.begin(), vertexSet.end(),
                            [](const TPG::TPGVertex* vert) {
                                return dynamic_cast<const TPG::TPGTeam*>(
                                           vert) != nullptr;
                            }),
              params.tpg.nbRoots)
        << "Number of team vertex in the graph is incorrect.";
    ASSERT_GE(tpg.getEdges().size(), 2 * params.tpg.nbRoots)
        << "Insufficient number of edges in the initialized TPG.";
    ASSERT_LE(tpg.getEdges().size(),
              params.tpg.nbRoots * params.tpg.maxInitOutgoingEdges)
        << "Too many edges in the initialized TPG.";

    // Check number of Programs.
    std::set<Program::Program*> programs;
    std::for_each(tpg.getEdges().begin(), tpg.getEdges().end(),
                  [&programs](const std::unique_ptr<TPG::TPGEdge>& edge) {
                      programs.insert(&edge->getProgram());
                  });
    ASSERT_EQ(programs.size(), params.tpg.nbRoots * 2)
        << "Number of distinct program in the TPG is incorrect.";
    // Check that no team has the same program twice
    for (auto team : tpg.getRootVertices()) {
        std::set<Program::Program*> teamPrograms;
        std::for_each(team->getOutgoingEdges().begin(),
                      team->getOutgoingEdges().end(),
                      [&teamPrograms](const TPG::TPGEdge* edge) {
                          teamPrograms.insert(&edge->getProgram());
                      });
        ASSERT_EQ(teamPrograms.size(), team->getOutgoingEdges().size())
            << "A team is connected to the same program twice.";
    }

    // Cover bad parameterization error
    params.tpg.maxInitOutgoingEdges = 6;
    ASSERT_THROW(
        Mutator::TPGMutator::initRandomTPG(tpg, params, rng, nbActions),
        std::runtime_error)
        << "TPG Initialization should fail with bad parameters.";
    params.tpg.maxInitOutgoingEdges = 0;
    nbActions = 1;
    ASSERT_THROW(
        Mutator::TPGMutator::initRandomTPG(tpg, params, rng, nbActions),
        std::runtime_error)
        << "TPG Initialization should fail with bad parameters.";

    // Test coverage: maxInitOutgoingEdges < 2 and initNbTeams > 0
    nbActions = 10;
    params.tpg.ratioTeamsOverActions = 0.5;
    params.tpg.nbRoots = 4;
    params.tpg.maxInitOutgoingEdges = 1;
    ASSERT_THROW(
        Mutator::TPGMutator::initRandomTPG(tpg, params, rng, nbActions),
        std::runtime_error)
        << "Should throw when maxInitOutgoingEdges < 2 and teams exist.";

    params.tpg.maxInitOutgoingEdges = 2;
    params.tpg.nbRoots = 4;
    params.tpg.ratioTeamsOverActions = 0.5;
    params.tpg.useActionProgram = false;
    params.tpg.teamAccessAllActions = false;
    ASSERT_THROW(
        Mutator::TPGMutator::initRandomTPG(tpg, params, rng, nbActions),
        std::runtime_error)
        << "Should throw bad parameters.";

    // Test coverage: ratioTeamsOverActions = 1, useActionProgram = true,
    // teamAccessAllActions = false
    params.tpg.ratioTeamsOverActions = 1.0;
    params.tpg.useActionProgram = true;
    ASSERT_THROW(
        Mutator::TPGMutator::initRandomTPG(tpg, params, rng, nbActions),
        std::runtime_error)
        << "Should throw with useActionProgram enabled.";

    // Test coverage: useMultiActionProgram
    params.tpg.maxInitOutgoingEdges = 2;
    params.tpg.useActionProgram = true;
    params.tpg.useMultiActionProgram = true;
    params.tpg.nbRoots = 4;
    params.tpg.ratioTeamsOverActions = 0.5;
    ASSERT_THROW(
        Mutator::TPGMutator::initRandomTPG(tpg, params, rng, nbActions),
        std::runtime_error)
        << "Should throw with useMultiActionProgram enabled.";
}

TEST_F(MutatorTest, TPGMutatorInitRandomTPGContinuous)
{

    Mutator::RNG rng;
    Mutator::MutationParameters& mutParams = params.mutation;
    mutParams.tpg.maxInitOutgoingEdges = 4;
    mutParams.prog.maxProgramSize = 96;
    mutParams.prog.pConstantMutation = 0.5;
    mutParams.tpg.ratioTeamsOverActions = 0.5;
    mutParams.prog.minConstValue = 0;
    mutParams.prog.maxConstValue = 1;
    params.nbRegisters = 8;

    // Error on number of registers
    rng.setSeed(0);
    params.mutation.tpg.useActionProgram = false;
    uint64_t nbActions = 8;
    Environment ce0(set, params, vect, nbActions);
    TPG::TPGGraph tpg0(ce0);
    ASSERT_THROW(
        Mutator::TPGMutator::initRandomTPG(tpg0, mutParams, rng, nbActions),
        std::runtime_error)
        << "TPG Initialization should fail with bad parameters.";

    // No error
    rng.setSeed(0);
    nbActions = 7;
    params.mutation.tpg.useActionProgram = true;
    Environment ce1(set, params, vect, nbActions);
    TPG::TPGGraph tpg1(ce1);
    ASSERT_NO_THROW(
        Mutator::TPGMutator::initRandomTPG(tpg1, mutParams, rng, nbActions))
        << "TPG Initialization should not fail.";

    // Error on number of registers
    rng.setSeed(0);
    nbActions = 9;
    Environment ce2(set, params, vect, nbActions);
    TPG::TPGGraph tpg2(ce2);
    ASSERT_THROW(
        Mutator::TPGMutator::initRandomTPG(tpg2, mutParams, rng, nbActions),
        std::runtime_error)
        << "TPG Initialization should fail with bad parameters.";

    rng.setSeed(0);
    nbActions = 8;
    params.mutation.tpg.teamAccessAllActions = true;
    Environment ce3(set, params, vect, nbActions);
    TPG::TPGGraph tpg3(ce3);
    ASSERT_NO_THROW(
        Mutator::TPGMutator::initRandomTPG(tpg3, mutParams, rng, nbActions))
        << "TPG Initialization should fail with bad parameters.";

    auto vertexSet = tpg3.getVertices();
    // Check number or vertex, roots, actions, teams, edges
    ASSERT_EQ(vertexSet.size(),
              mutParams.tpg.nbRoots * 3 * mutParams.tpg.ratioTeamsOverActions +
                  mutParams.tpg.nbRoots *
                      (1 - mutParams.tpg.ratioTeamsOverActions))
        << "Number of vertices after initialization is incorrect.";
    ASSERT_EQ(tpg3.getRootVertices().size(), mutParams.tpg.nbRoots)
        << "Number of root vertices after initialization is incorrect.";
    ASSERT_EQ(
        std::count_if(vertexSet.begin(), vertexSet.end(),
                      [](const TPG::TPGVertex* vert) {
                          return dynamic_cast<const TPG::TPGAction*>(vert) !=
                                 nullptr;
                      }),
        mutParams.tpg.nbRoots * 2 * mutParams.tpg.ratioTeamsOverActions +
            mutParams.tpg.nbRoots * (1 - mutParams.tpg.ratioTeamsOverActions))
        << "Number of action vertex in the graph is incorrect.";
    ASSERT_EQ(std::count_if(vertexSet.begin(), vertexSet.end(),
                            [](const TPG::TPGVertex* vert) {
                                return dynamic_cast<const TPG::TPGTeam*>(
                                           vert) != nullptr;
                            }),
              mutParams.tpg.nbRoots * mutParams.tpg.ratioTeamsOverActions)
        << "Number of team vertex in the graph is incorrect.";
    ASSERT_GE(tpg3.getEdges().size(), mutParams.tpg.nbRoots * 2)
        << "Insufficient number of edges in the initialized TPG.";
    ASSERT_LE(tpg3.getEdges().size(),
              mutParams.tpg.nbRoots * (mutParams.tpg.maxInitOutgoingEdges + 1))
        << "Too many edges in the initialized TPG.";

    // Check number of Programs.
    std::set<Program::Program*> programs;
    std::for_each(tpg3.getEdges().begin(), tpg3.getEdges().end(),
                  [&programs](const std::unique_ptr<TPG::TPGEdge>& edge) {
                      programs.insert(&edge->getProgram());
                  });
    // 2 contexts programs and 2 actions program per roots for team roots, one
    // program for action roots
    ASSERT_EQ(programs.size(),
              4 * mutParams.tpg.nbRoots * mutParams.tpg.ratioTeamsOverActions +
                  mutParams.tpg.nbRoots *
                      (1 - mutParams.tpg.ratioTeamsOverActions))
        << "Number of distinct program in the TPG is incorrect.";

    // Test coverage: useMultiActionProgram
    mutParams.tpg.useMultiActionProgram = true;
    mutParams.tpg.useActionProgram = true;
    mutParams.tpg.nbRoots = 4;
    mutParams.tpg.ratioTeamsOverActions = 0.5;
    nbActions = 8;
    Environment ce4(set, params, vect, nbActions);
    TPG::TPGGraph tpg4(ce4);

    // Test coverage: teamAccessAllActions = true
    mutParams.tpg.useMultiActionProgram = false;
    mutParams.tpg.teamAccessAllActions = true;
    ASSERT_NO_THROW(
        Mutator::TPGMutator::initRandomTPG(tpg4, mutParams, rng, nbActions))
        << "Should not throw with teamAccessAllActions enabled.";

    params.nbRegisters = 8;
    mutParams.tpg.useActionProgram = true;
    mutParams.tpg.teamAccessAllActions = false;
    mutParams.tpg.ratioTeamsOverActions = 1.0;
    Environment ce5(set, params, vect, 4);
    TPG::TPGGraph tpg5(ce5);
    ASSERT_THROW(Mutator::TPGMutator::initRandomTPG(tpg5, mutParams, rng, 4),
                 std::runtime_error)
        << "Should throw when nbContinuousActions == 0.";
}

TEST_F(MutatorTest, TPGMutatorInitRandomTPGMAPLE)
{

    Mutator::RNG rng;
    Mutator::MutationParameters& mutParams = params.mutation;
    mutParams.tpg.maxInitOutgoingEdges = 4;
    mutParams.prog.maxProgramSize = 96;
    mutParams.prog.pConstantMutation = 0.5;
    mutParams.prog.minConstValue = 0;
    mutParams.prog.maxConstValue = 1;
    params.nbRegisters = 8;

    // Error on number of registers
    rng.setSeed(0);
    params.mutation.tpg.useActionProgram = true;
    params.mutation.tpg.useMultiActionProgram = true;
    params.mutation.tpg.nbActionEdgeInit = 10;
    params.mutation.tpg.ratioTeamsOverActions = 0.0;
    uint64_t nbActions = 5;
    Environment ce2(set, params, vect, nbActions);
    TPG::TPGGraph tpg2(ce2);
    ASSERT_THROW(
        Mutator::TPGMutator::initRandomTPG(tpg2, mutParams, rng, nbActions),
        std::runtime_error)
        << "TPG Initialization should fail with bad parameters.";

    // Error on number of registers
    rng.setSeed(0);
    nbActions = 8;
    params.mutation.tpg.nbActionEdgeInit = 3;
    Environment ce3(set, params, vect, nbActions);
    TPG::TPGGraph tpg3(ce3);
    ASSERT_NO_THROW(
        Mutator::TPGMutator::initRandomTPG(tpg3, mutParams, rng, nbActions))
        << "TPG Initialization should fail with bad parameters.";

    auto vertexSet = tpg3.getVertices();
    // Check number or vertex, roots, actions, teams, edges
    ASSERT_EQ(vertexSet.size(), mutParams.tpg.nbRoots)
        << "Number of vertices after initialization is incorrect.";
    ASSERT_EQ(tpg3.getRootVertices().size(), mutParams.tpg.nbRoots)
        << "Number of root vertices after initialization is incorrect.";
    ASSERT_EQ(std::count_if(vertexSet.begin(), vertexSet.end(),
                            [](const TPG::TPGVertex* vert) {
                                return dynamic_cast<const TPG::TPGAction*>(
                                           vert) != nullptr;
                            }),
              mutParams.tpg.nbRoots)
        << "Number of action vertex in the graph is incorrect.";
    ASSERT_EQ(std::count_if(vertexSet.begin(), vertexSet.end(),
                            [](const TPG::TPGVertex* vert) {
                                return dynamic_cast<const TPG::TPGTeam*>(
                                           vert) != nullptr;
                            }),
              0)
        << "Number of team vertex in the graph is incorrect.";
    ASSERT_EQ(tpg3.getEdges().size(),
              mutParams.tpg.nbRoots * params.mutation.tpg.nbActionEdgeInit)
        << "Number of edges in the initialized TPG is incorrect.";

    // Check number of Programs.
    std::set<Program::Program*> programs;
    std::for_each(tpg3.getEdges().begin(), tpg3.getEdges().end(),
                  [&programs](const std::unique_ptr<TPG::TPGEdge>& edge) {
                      programs.insert(&edge->getProgram());
                  });
    // 2 contexts programs and 2 actions program per roots
    ASSERT_EQ(programs.size(),
              mutParams.tpg.nbRoots * params.mutation.tpg.nbActionEdgeInit)
        << "Number of distinct program in the TPG is incorrect.";
}

TEST_F(MutatorTest, TPGMutatorRemoveRandomEdge)
{
    TPG::TPGGraph tpg(*e);
    const TPG::TPGTeam& vertex0 = tpg.addNewTeam();
    const TPG::TPGAction& vertex1 = tpg.addNewAction(0);
    const TPG::TPGTeam& vertex2 = tpg.addNewTeam();
    const TPG::TPGAction& vertex3 = tpg.addNewAction(1);
    const TPG::TPGEdge& edge0 = tpg.addNewEdge(vertex0, vertex1, progPointer);
    const TPG::TPGEdge& edge1 = tpg.addNewEdge(vertex0, vertex2, progPointer);
    const TPG::TPGEdge& edge2 = tpg.addNewEdge(vertex0, vertex3, progPointer);

    Mutator::RNG rng;
    rng.setSeed(0);
    ASSERT_NO_THROW(Mutator::TPGMutator::removeRandomEdge(tpg, vertex0, rng))
        << "Removing a random edge failed unexpectedly.";
    // Check properties of the tpg
    ASSERT_EQ(tpg.getEdges().size(), 2) << "No edge was removed from the TPG.";
    // With known seed edge 0 was removed
    ASSERT_EQ(
        std::count_if(tpg.getEdges().begin(), tpg.getEdges().end(),
                      [&edge0](const std::unique_ptr<TPG::TPGEdge>& other) {
                          return &edge0 == other.get();
                      }),
        0)
        << "With a known seed, edge0 should be removed from the TPG.";
    ASSERT_EQ(
        std::count_if(tpg.getEdges().begin(), tpg.getEdges().end(),
                      [&edge1](const std::unique_ptr<TPG::TPGEdge>& other) {
                          return &edge1 == other.get();
                      }),
        1)
        << "With a known seed, edge1 should not be removed from the TPG.";
    ASSERT_EQ(
        std::count_if(tpg.getEdges().begin(), tpg.getEdges().end(),
                      [&edge2](const std::unique_ptr<TPG::TPGEdge>& other) {
                          return &edge2 == other.get();
                      }),
        1)
        << "With a known seed, edge2 should not be removed from the TPG.";
}

TEST_F(MutatorTest, TPGMutatorAddRandomEdge)
{
    TPG::TPGGraph tpg(*e);
    const TPG::TPGTeam& vertex0 = tpg.addNewTeam();
    const TPG::TPGAction& vertex1 = tpg.addNewAction(0);
    const TPG::TPGTeam& vertex2 = tpg.addNewTeam();
    const TPG::TPGAction& vertex3 = tpg.addNewAction(1);
    const TPG::TPGAction& vertex4 = tpg.addNewAction(2);
    std::list<const TPG::TPGEdge*> edges;

    edges.push_back(&tpg.addNewEdge(vertex0, vertex1, progPointer));
    edges.push_back(&tpg.addNewEdge(vertex0, vertex2, progPointer));
    edges.push_back(&tpg.addNewEdge(vertex0, vertex3, progPointer));
    edges.push_back(&tpg.addNewEdge(vertex2, vertex4, progPointer));

    Mutator::RNG rng;
    rng.setSeed(0);
    // Run the add
    ASSERT_NO_THROW(
        Mutator::TPGMutator::addRandomEdge(tpg, vertex2, edges, rng))
        << "Adding an edge to the TPG should succeed.";

    // Check properties of the tpg
    ASSERT_EQ(tpg.getEdges().size(), 5) << "No edge was added from the TPG.";
    ASSERT_EQ(vertex2.getOutgoingEdges().size(), 2)
        << "The random edge was not added to the right team.";

    // Edge was added with vertex1 (with known seed)
    ASSERT_EQ(vertex1.getIncomingEdges().size(), 2)
        << "The random edge was not added with the right (pseudo)random "
           "destination.";

    // Force a failure
    TPG::TPGEdge newEdge(&vertex0, &vertex1, progPointer);
    ASSERT_THROW(
        Mutator::TPGMutator::addRandomEdge(tpg, vertex2, {&newEdge}, rng),
        std::runtime_error)
        << "Picking an edge not belonging to the graph should fail.";
}

TEST_F(MutatorTest, TPGMutatorMutateEdgeDestination)
{
    TPG::TPGGraph tpg(*e);
    const TPG::TPGTeam& vertex0 = tpg.addNewTeam();
    const TPG::TPGAction& vertex1 = tpg.addNewAction(0);
    const TPG::TPGAction& vertex2 = tpg.addNewAction(1);
    const TPG::TPGTeam& vertex3 = tpg.addNewTeam();
    const TPG::TPGTeam& vertex4 = tpg.addNewTeam();

    const TPG::TPGEdge& edge0 = tpg.addNewEdge(vertex0, vertex1, progPointer);
    const TPG::TPGEdge& edge1 = tpg.addNewEdge(vertex0, vertex3, progPointer);

    Mutator::MutationParameters params;
    params.tpg.pEdgeDestinationIsAction = 0.5;

    Mutator::RNG rng;
    rng.setSeed(2);
    ASSERT_NO_THROW(Mutator::TPGMutator::mutateEdgeDestination(
        tpg, &edge1, {&vertex3, &vertex4}, {&vertex1, &vertex2}, params, rng));
    // Check properties of the tpg
    ASSERT_EQ(tpg.getEdges().size(), 2)
        << "Number of edge should remain unchanged after destination change.";
    ASSERT_EQ(vertex0.getOutgoingEdges().size(), 2)
        << "The edge source should not be altered.";
    ASSERT_EQ(vertex3.getIncomingEdges().size(), 0)
        << "The edge Destination should be vertex4 (with known seed).";
    ASSERT_EQ(vertex4.getIncomingEdges().size(), 1)
        << "The edge Destination should be vertex4 (with known seed).";
}

TEST_F(MutatorTest, TPGMutatorMutateOutgoingEdge)
{
    Mutator::RNG rng;
    rng.setSeed(0);

    // Init a TPG
    TPG::TPGGraph tpg(*e);
    const TPG::TPGTeam& vertex0 = tpg.addNewTeam();
    const TPG::TPGAction& vertex1 = tpg.addNewAction(0);
    const TPG::TPGEdge& edge0 = tpg.addNewEdge(vertex0, vertex1, progPointer);

    // Init its program and fill the archive
    Mutator::MutationParameters params;
    Archive arch;
    TPG::TPGExecutionEngine tee(*e, &arch);
    params.prog.maxProgramSize = 96;
    params.prog.pConstantMutation = 0.5;
    params.prog.minConstValue = 0;
    params.prog.maxConstValue = 1;
    Mutator::ProgramMutator::initRandomProgram(*progPointer, params, rng);
    tee.executeFromRoot(vertex0);

    // Mutate (params selected for code coverage)
    params.prog.pAdd = 0.5;
    params.prog.pDelete = 0.5;
    params.prog.pMutate = 1.0;
    params.prog.pSwap = 1.0;
    params.tpg.pEdgeDestinationChange = 1.0;

    std::list<std::shared_ptr<Program::Program>> newPrograms;

    ASSERT_NO_THROW(Mutator::TPGMutator::mutateOutgoingEdge(
        tpg, &edge0, {&vertex0}, {&vertex1}, {&edge0}, newPrograms, params,
        rng));

    // Check that progPointer use count was decreased since the mutated program
    // is a copy of the original
    ASSERT_EQ(progPointer.use_count(), 1)
        << "Shared pointer should no longer be used inside the TPG after "
           "mutation.";
}

TEST_F(MutatorTest, TPGMutatorRemoveRandomActionEdge)
{
    TPG::TPGGraph tpg(*e);
    const TPG::TPGAction& action = tpg.addNewAction(0);
    auto prog1 = std::make_shared<Program::Program>(*e, true);
    auto prog2 = std::make_shared<Program::Program>(*e, true);
    tpg.addNewActionEdge(action, prog1, 0);
    tpg.addNewActionEdge(action, prog2, 1);

    Mutator::RNG rng;
    rng.setSeed(0);

    size_t before = action.getOutgoingEdges().size();
    ASSERT_NO_THROW(
        Mutator::TPGMutator::removeRandomActionEdge(tpg, action, rng));
    ASSERT_EQ(action.getOutgoingEdges().size(), before - 1);
}

TEST_F(MutatorTest, TPGMutatorAddRandomActionEdge)
{
    // Prepare a TPGAction and a list of candidate edges
    TPG::TPGGraph tpg(*e);
    const TPG::TPGAction& action = tpg.addNewAction(0);
    auto prog1 = std::make_shared<Program::Program>(*e, true);
    auto prog2 = std::make_shared<Program::Program>(*e, true);
    const TPG::TPGAction& action2 = tpg.addNewAction(1);
    tpg.addNewActionEdge(action, prog1, 0);
    tpg.addNewActionEdge(action2, prog2, 1);

    std::list<const TPG::TPGEdge*> edges;
    for (auto& edge : tpg.getEdges()) {
        edges.push_back(edge.get());
    }

    Mutator::RNG rng;
    rng.setSeed(0);

    // Can add an edge without throwing (even if nothing changes if no edge is
    // pickable)
    ASSERT_NO_THROW(
        Mutator::TPGMutator::addRandomActionEdge(tpg, action, edges, rng));

    edges.clear();
    // Can add an edge without throwing (even if nothing changes if no edge is
    // pickable)
    ASSERT_NO_THROW(
        Mutator::TPGMutator::addRandomActionEdge(tpg, action, edges, rng));
}

TEST_F(MutatorTest, TPGMutatorSwapActionEdges)
{
    // Prepare a TPGAction with at least 3 outgoing edges
    TPG::TPGGraph tpg(*e);
    const TPG::TPGAction& action = tpg.addNewAction(0);
    auto prog1 = std::make_shared<Program::Program>(*e, true);
    auto prog2 = std::make_shared<Program::Program>(*e, true);
    auto prog3 = std::make_shared<Program::Program>(*e, true);
    tpg.addNewActionEdge(action, prog1, 0);
    tpg.addNewActionEdge(action, prog2, 1);
    tpg.addNewActionEdge(action, prog3, 2);

    Mutator::RNG rng;
    // Set a known seed to ensure deterministic behavior and have equality
    // during random choice of swapping Should be change if determinism is
    // changed
    rng.setSeed(4);

    // We check that the swapActionEdges function works correctly
    auto before = std::vector<uint64_t>();
    for (auto edge : action.getOutgoingEdges()) {
        before.push_back(
            dynamic_cast<TPG::TPGActionEdge*>(edge)->getActionClass());
    }
    ASSERT_NO_THROW(Mutator::TPGMutator::swapActionEdges(tpg, action, rng));
    auto after = std::vector<uint64_t>();
    for (auto edge : action.getOutgoingEdges()) {
        after.push_back(
            dynamic_cast<TPG::TPGActionEdge*>(edge)->getActionClass());
    }
    // There must be at least one change
    ASSERT_NE(before, after);
}

TEST_F(MutatorTest, TPGMutatorMutateTPGAction_MultiAction)
{
    // Teste la mutation d'une action avec useMultiActionProgram
    Mutator::RNG rng;
    rng.setSeed(0);

    params.mutation.tpg.useActionProgram = true;
    params.mutation.tpg.useMultiActionProgram = true;
    params.mutation.tpg.pActionEdgeDeletion = 0.7;
    params.mutation.tpg.pActionEdgeAddition = 0.7;
    params.mutation.tpg.pSwapActionProgram = 0.7;
    params.mutation.tpg.pMutateActionProgram = 0.7;

    uint64_t nbActions = 5;
    Environment ce(set, params, vect, nbActions);

    TPG::TPGGraph tpg(ce);
    const TPG::TPGAction* action = &tpg.addNewAction(0);
    const TPG::TPGAction* actionUnused = &tpg.addNewAction(1);
    auto prog1 = std::make_shared<Program::Program>(ce, true);
    auto prog2 = std::make_shared<Program::Program>(ce, true);
    auto prog3 = std::make_shared<Program::Program>(ce, true);
    auto prog4 = std::make_shared<Program::Program>(ce, true);
    tpg.addNewActionEdge(*action, prog1, 0);
    tpg.addNewActionEdge(*action, prog2, 1);
    tpg.addNewActionEdge(*actionUnused, prog3, 2);
    tpg.addNewActionEdge(*actionUnused, prog4, 1);

    std::list<std::shared_ptr<Program::Program>> newPrograms;
    std::list<const TPG::TPGEdge*> preExistingEdges;
    std::for_each(
        tpg.getEdges().begin(), tpg.getEdges().end(),
        [&preExistingEdges](const std::unique_ptr<TPG::TPGEdge>& edge) {
            preExistingEdges.push_back(edge.get());
        });

    // Do it several times to cover all branches
    for (size_t i = 0; i < 5; i++) {
        const TPG::TPGAction* copiedAction =
            dynamic_cast<const TPG::TPGAction*>(&tpg.cloneVertex(*action));

        // Check that all branches are covered (deletion, addition, swap,
        // mutation)
        ASSERT_NO_THROW(Mutator::TPGMutator::mutateTPGAction(
            tpg, *copiedAction, preExistingEdges, newPrograms, params.mutation,
            rng));
    }
}

TEST_F(MutatorTest, TPGMutatorMutateAction)
{

    Mutator::RNG rng;
    rng.setSeed(0);

    params.mutation.tpg.useActionProgram = true;
    uint64_t nbActions = 8;
    Environment ce(set, params, vect, nbActions);

    std::shared_ptr<Program::Program> progPointer1 =
        std::shared_ptr<Program::Program>(new Program::Program(ce, true));

    // Init a TPG
    TPG::TPGGraph tpg(ce);
    const TPG::TPGTeam& vertex0 = tpg.addNewTeam();
    const TPG::TPGAction& vertex1 = tpg.addNewAction(0);
    const TPG::TPGEdge& edge0 = tpg.addNewEdge(vertex0, vertex1, progPointer);
    const TPG::TPGEdge& edge1 = tpg.addNewActionEdge(vertex1, progPointer1, 0);

    // Init its program and fill the archive
    Mutator::MutationParameters params;
    Archive arch;
    TPG::TPGExecutionEngine tee(ce, &arch);
    params.prog.maxProgramSize = 96;
    params.prog.pConstantMutation = 0.5;
    params.prog.minConstValue = 0;
    params.prog.maxConstValue = 1;
    Mutator::ProgramMutator::initRandomProgram(*progPointer1, params, rng);
    tee.executeFromRoot(vertex0);

    std::list<std::shared_ptr<Program::Program>> newPrograms;
    ASSERT_NO_THROW(Mutator::TPGMutator::mutateTPGAction(
        tpg, vertex1, {&edge0}, newPrograms, params, rng));

    // Check that progPointer use count was decreased since the mutated program
    // is a copy of the original
    ASSERT_EQ(progPointer1.use_count(), 1)
        << "Shared pointer should no longer be used inside the TPG after "
           "mutation.";
}

TEST_F(MutatorTest, TPGMutatorMutateTPGActionEdge_MultiAction)
{
    // Teste la mutation d'une action edge avec useMultiActionProgram
    Mutator::RNG rng;
    rng.setSeed(1);

    params.mutation.tpg.useActionProgram = true;
    params.mutation.tpg.useMultiActionProgram = true;
    params.mutation.tpg.pChangeActionClass =
        1.0; // Pour forcer le changement d'actionClass
    uint64_t nbActions = 3;
    Environment ce(set, params, vect, nbActions);

    TPG::TPGGraph tpg(ce);
    const TPG::TPGAction& action = tpg.addNewAction(0);
    auto prog = std::make_shared<Program::Program>(ce, true);
    tpg.addNewActionEdge(action, prog, 0);
    tpg.addNewActionEdge(action, std::make_shared<Program::Program>(ce, true),
                         1);

    std::list<std::shared_ptr<Program::Program>> newPrograms;
    auto edge =
        dynamic_cast<TPG::TPGActionEdge*>(*action.getOutgoingEdges().begin());

    // On vérifie que le changement d'actionClass est bien tenté
    ASSERT_NO_THROW(Mutator::TPGMutator::mutateTPGActionEdge(
        tpg, action, edge, newPrograms, params.mutation, rng));
}

TEST_F(MutatorTest, TPGMutatorOutgoingEdgeMutateAction)
{

    Mutator::RNG rng;
    rng.setSeed(0);

    params.mutation.tpg.useActionProgram = true;
    uint64_t nbActions = 8;
    Environment ce(set, params, vect, nbActions);

    std::shared_ptr<Program::Program> progPointer1 =
        std::shared_ptr<Program::Program>(new Program::Program(ce, true));

    // Init a TPG
    TPG::TPGGraph tpg(ce);
    const TPG::TPGTeam& vertex0 = tpg.addNewTeam();
    const TPG::TPGAction& vertex1 = tpg.addNewAction(0);
    const TPG::TPGEdge& edge0 = tpg.addNewEdge(vertex0, vertex1, progPointer);
    const TPG::TPGEdge& edge1 = tpg.addNewActionEdge(vertex1, progPointer1, 0);

    // Init its program and fill the archive
    Mutator::MutationParameters params;
    Archive arch;
    TPG::TPGExecutionEngine tee(ce, &arch);
    params.prog.maxProgramSize = 96;
    params.prog.pConstantMutation = 0.5;
    params.prog.minConstValue = 0;
    params.prog.maxConstValue = 1;
    params.tpg.probaContextOverActionProgram = 0;
    params.tpg.useActionProgram = true;
    Mutator::ProgramMutator::initRandomProgram(*progPointer1, params, rng);
    tee.executeFromRoot(vertex0);

    std::list<std::shared_ptr<Program::Program>> newPrograms;
    ASSERT_NO_THROW(Mutator::TPGMutator::mutateOutgoingEdge(
        tpg, &edge0, {&vertex0}, {&vertex1}, {&edge0}, newPrograms, params,
        rng));

    ASSERT_EQ(tpg.getNbVertices(), 3) << "Action should have been duplicated.";

    tpg.removeVertex(vertex1);

    // Check that progPointer use count was decreased since the mutated program
    // is a copy of the original
    ASSERT_EQ(progPointer1.use_count(), 1)
        << "Shared pointer should no longer be used inside the TPG after "
           "mutation.";
}

TEST_F(MutatorTest, TPGMutatorMutateTeam)
{
    Mutator::RNG rng;
    rng.setSeed(0);

    // Create a TPG
    TPG::TPGGraph tpg(*e);
    const TPG::TPGTeam& vertex0 = tpg.addNewTeam();
    const TPG::TPGAction& vertex1 = tpg.addNewAction(0);
    const TPG::TPGAction& vertex2 = tpg.addNewAction(1);
    const TPG::TPGEdge& edge0 = tpg.addNewEdge(vertex0, vertex1, progPointer);
    const TPG::TPGEdge& edge1 = tpg.addNewEdge(vertex0, vertex2, progPointer);
    const TPG::TPGAction& vertex3 = tpg.addNewAction(2);
    const TPG::TPGTeam& vertex4 = tpg.addNewTeam();
    const TPG::TPGEdge& edge2 = tpg.addNewEdge(vertex4, vertex3, progPointer);
    const TPG::TPGEdge& edge3 = tpg.addNewEdge(vertex0, vertex3, progPointer);

    Mutator::MutationParameters params;
    params.prog.maxProgramSize = 96;
    params.tpg.pEdgeDeletion = 0.7;
    params.tpg.pEdgeAddition = 0.7;
    params.tpg.pProgramMutation = 0.2;
    params.tpg.pEdgeDestinationChange = 0.1;
    params.tpg.pEdgeDestinationIsAction = 0.5;
    params.prog.pAdd = 0.5;
    params.prog.pDelete = 0.5;
    params.prog.pMutate = 1.0;
    params.prog.pSwap = 1.0;
    params.prog.pConstantMutation = 0.5;
    params.prog.minConstValue = 0;
    params.prog.maxConstValue = 1;

    // Init its program and fill the archive
    Archive arch;
    TPG::TPGExecutionEngine tee(*e, &arch);
    Mutator::ProgramMutator::initRandomProgram(*progPointer, params, rng);
    tee.executeFromRoot(vertex0);

    std::list<std::shared_ptr<Program::Program>> newPrograms;

    // Test the function in normal conditions
    // (only edge2 can be part of "preExistingEdges" since all other edges are
    // outgoing from vertex0, which would mean they are not pre-existing in
    // the mutation process.)
    ASSERT_NO_THROW(Mutator::TPGMutator::mutateTPGTeam(
        tpg, arch, vertex0, {&vertex0, &vertex4},
        {&vertex1, &vertex2, &vertex3}, {&edge2}, newPrograms, params, rng))
        << "Mutate team should not fail in these conditions.";

    // No other check really needed since individual mutation functions are
    // already covered in other unit tests.
}

TEST_F(MutatorTest, TPGMutatorMutateProgramBehaviorAgainstArchive)
{
    Mutator::RNG rng;
    rng.setSeed(0);

    // Init a TPG
    TPG::TPGGraph tpg(*e);
    const TPG::TPGTeam& vertex0 = tpg.addNewTeam();
    const TPG::TPGAction& vertex1 = tpg.addNewAction(0);
    const TPG::TPGEdge& edge0 = tpg.addNewEdge(vertex0, vertex1, progPointer);

    // Init its program and fill the archive
    Mutator::MutationParameters params;
    Archive arch;
    TPG::TPGExecutionEngine tee(*e, &arch);
    params.prog.maxProgramSize = 96;
    params.prog.pConstantMutation = 0.5;
    params.prog.minConstValue = 0;
    params.prog.maxConstValue = 1;

    Mutator::ProgramMutator::initRandomProgram(*progPointer, params, rng);
    tee.executeFromRoot(vertex0);

    // Mutate (params selected for code coverage)
    params.prog.pAdd = 0.5;
    params.prog.pDelete = 0.5;
    params.prog.pMutate = 1.0;
    params.prog.pSwap = 1.0;
    params.tpg.pEdgeDestinationChange = 1.0;

    std::list<std::shared_ptr<Program::Program>> newPrograms;

    Mutator::TPGMutator::mutateOutgoingEdge(tpg, &edge0, {&vertex0}, {&vertex1},
                                            {&edge0}, newPrograms, params, rng);

    ASSERT_NO_THROW(Mutator::TPGMutator::mutateProgramBehaviorAgainstArchive(
        newPrograms.front(), params, arch, rng))
        << "Mutating a Program behavior failed unexpectedly.";

    // Check the unicity against the Archive
    // Verify new program uniqueness
    Program::ProgramExecutionEngine pee(*newPrograms.front());
    double result = pee.executeProgram();
    std::map<size_t, double> hashesAndResults = {
        {arch.getCombinedHash(e->getDataSources()), result}};
    ASSERT_TRUE(arch.areProgramResultsUnique(hashesAndResults))
        << "Mutated program associated to the edge should return a unique bid "
           "on the environment.";

    // Test again for the newProgram parameter
    Mutator::ProgramMutator::initRandomProgram(*progPointer, params, rng);

    // Mutate (params selected for code coverage)
    params.prog.pNewProgram = 1;
    params.prog.pAdd = 0;
    params.prog.pDelete = 0;
    params.prog.pMutate = 0;
    params.prog.pSwap = 0;
    params.tpg.pEdgeDestinationChange = 0;

    newPrograms.clear();

    Mutator::TPGMutator::mutateOutgoingEdge(tpg, &edge0, {&vertex0}, {&vertex1},
                                            {&edge0}, newPrograms, params, rng);

    ASSERT_NO_THROW(Mutator::TPGMutator::mutateProgramBehaviorAgainstArchive(
        newPrograms.front(), params, arch, rng))
        << "Mutating a Program behavior failed unexpectedly.";

    // Check the unicity against the Archive
    // Verify new program uniqueness
    Program::ProgramExecutionEngine pee2(*newPrograms.front());
    result = pee2.executeProgram();

    hashesAndResults = {{arch.getCombinedHash(e->getDataSources()), result}};
    ASSERT_TRUE(arch.areProgramResultsUnique(hashesAndResults))
        << "Mutated program associated to the edge should return a unique bid "
           "on the environment.";
}

TEST_F(MutatorTest, TPGMutatorMutateNewProgramBehaviorsSequential)
{
    Mutator::RNG rng;
    rng.setSeed(0);

    TPG::TPGGraph tpg(*e);

    Mutator::MutationParameters params;

    uint64_t nbActions = 4;
    params.tpg.maxInitOutgoingEdges = 3;
    params.prog.maxProgramSize = 96;
    params.tpg.nbRoots = 7;
    // Proba as in Kelly's paper
    params.tpg.pEdgeDeletion = 0.7;
    params.tpg.pEdgeAddition = 0.7;
    params.tpg.pProgramMutation = 0.2;
    params.tpg.pEdgeDestinationChange = 0.1;
    params.tpg.pEdgeDestinationIsAction = 0.5;
    params.prog.pAdd = 0.5;
    params.prog.pDelete = 0.5;
    params.prog.pMutate = 1.0;
    params.prog.pSwap = 1.0;
    params.prog.pConstantMutation = 0.5;
    params.prog.minConstValue = 0;
    params.prog.maxConstValue = 10;
    Archive arch;

    Mutator::TPGMutator::initRandomTPG(tpg, params, rng, nbActions);
    // fill the archive before populating to test uniqueness of new prog
    TPG::TPGExecutionEngine tee(*e, &arch);
    for (auto rootVertex : tpg.getRootVertices()) {
        tee.executeFromRoot(*rootVertex);
    }

    // Create a list of Programs to mutate
    std::list<std::shared_ptr<Program::Program>> programs;
    for (auto& edge : tpg.getEdges()) {
        programs.emplace_back(new Program::Program(edge->getProgram()));
    }

    // Mutate them sequentially
    ASSERT_NO_THROW(Mutator::TPGMutator::mutateNewProgramBehaviors(
        0, programs, rng, params, arch))
        << "Program behavior mutation failed (sequentially).";
}

TEST_F(MutatorTest, TPGMutatorMutateNewProgramBehaviorsParallel)
{
    Mutator::RNG rng;
    rng.setSeed(0);

    TPG::TPGGraph tpg(*e);

    Mutator::MutationParameters params;

    uint64_t nbActions = 4;
    params.tpg.maxInitOutgoingEdges = 3;
    params.prog.maxProgramSize = 96;
    params.tpg.nbRoots = 7;
    // Proba as in Kelly's paper
    params.tpg.pEdgeDeletion = 0.7;
    params.tpg.pEdgeAddition = 0.7;
    params.tpg.pProgramMutation = 0.2;
    params.tpg.pEdgeDestinationChange = 0.1;
    params.tpg.pEdgeDestinationIsAction = 0.5;
    params.prog.pAdd = 0.5;
    params.prog.pDelete = 0.5;
    params.prog.pMutate = 1.0;
    params.prog.pSwap = 1.0;
    params.prog.pConstantMutation = 0.5;
    params.prog.minConstValue = 0;
    params.prog.maxConstValue = 10;
    Archive arch;

    Mutator::TPGMutator::initRandomTPG(tpg, params, rng, nbActions);
    // fill the archive before populating to test uniqueness of new prog
    TPG::TPGExecutionEngine tee(*e, &arch);
    for (auto rootVertex : tpg.getRootVertices()) {
        tee.executeFromRoot(*rootVertex);
    }

    // Create a list of Programs to mutate
    std::list<std::shared_ptr<Program::Program>> programs;
    for (auto& edge : tpg.getEdges()) {
        programs.emplace_back(new Program::Program(edge->getProgram()));
    }

    // Mutate them sequentially
    ASSERT_NO_THROW(Mutator::TPGMutator::mutateNewProgramBehaviors(
        4, programs, rng, params, arch))
        << "Program behavior mutation failed (In parallel).";
}

TEST_F(MutatorTest, TPGMutatorMutateNewProgramBehaviorsDeterminism)
{
    Mutator::RNG rng;

    TPG::TPGGraph tpg(*e);

    Mutator::MutationParameters params;

    uint64_t nbActions = 4;
    params.tpg.maxInitOutgoingEdges = 3;
    params.prog.maxProgramSize = 96;
    params.tpg.nbRoots = 7;
    // Proba as in Kelly's paper
    params.tpg.pEdgeDeletion = 0.7;
    params.tpg.pEdgeAddition = 0.7;
    params.tpg.pProgramMutation = 0.2;
    params.tpg.pEdgeDestinationChange = 0.1;
    params.tpg.pEdgeDestinationIsAction = 0.5;
    params.prog.pAdd = 0.5;
    params.prog.pDelete = 0.5;
    params.prog.pMutate = 1.0;
    params.prog.pSwap = 1.0;
    params.prog.pConstantMutation = 0.5;
    params.prog.minConstValue = 0;
    params.prog.maxConstValue = 10;
    Archive arch;

    Mutator::TPGMutator::initRandomTPG(tpg, params, rng, nbActions);
    // fill the archive before populating to test uniqueness of new prog
    TPG::TPGExecutionEngine tee(*e, &arch);
    for (auto rootVertex : tpg.getRootVertices()) {
        tee.executeFromRoot(*rootVertex);
    }

    // Create a list of Programs to mutate
    std::list<std::shared_ptr<Program::Program>> programsSequential;
    std::list<std::shared_ptr<Program::Program>> programsParallel;
    for (auto& edge : tpg.getEdges()) {
        programsSequential.emplace_back(
            new Program::Program(edge->getProgram()));
        programsParallel.emplace_back(new Program::Program(edge->getProgram()));
    }
    rng.setSeed(0);
    Mutator::TPGMutator::mutateNewProgramBehaviors(1, programsSequential, rng,
                                                   params, arch);

    rng.setSeed(0);
    Mutator::TPGMutator::mutateNewProgramBehaviors(4, programsParallel, rng,
                                                   params, arch);

    // Check determinism
    // Using nb lines of programs
    for (auto i = 0; i < programsParallel.size(); i++) {
        ASSERT_EQ(programsParallel.front()->getNbLines(),
                  programsSequential.front()->getNbLines())
            << "Different number of line in mutatedPrograms.";
        programsParallel.pop_front();
        programsSequential.pop_front();
    }
}

TEST_F(MutatorTest, TPGMutatorPopulate)
{
    Mutator::RNG rng;
    rng.setSeed(0);

    std::shared_ptr<TPG::TPGGraph> tpg = std::make_shared<TPG::TPGGraph>(*e);

    uint64_t nbActions = 4;
    params.mutation.tpg.maxInitOutgoingEdges = 3;
    params.mutation.prog.maxProgramSize = 96;
    params.mutation.tpg.nbRoots = 7;
    // Proba as in Kelly's paper
    params.mutation.tpg.pEdgeDeletion = 0.7;
    params.mutation.tpg.pEdgeAddition = 0.7;
    params.mutation.tpg.pProgramMutation = 0.2;
    params.mutation.tpg.pEdgeDestinationChange = 0.1;
    params.mutation.tpg.pEdgeDestinationIsAction = 0.5;
    params.mutation.prog.pAdd = 0.5;
    params.mutation.prog.pDelete = 0.5;
    params.mutation.prog.pMutate = 1.0;
    params.mutation.prog.pSwap = 1.0;
    params.mutation.prog.pConstantMutation = 0.5;
    params.mutation.prog.minConstValue = 0;
    params.mutation.prog.maxConstValue = 10;
    Archive arch;
    Selector::Selector selector(tpg, params);

    Mutator::TPGMutator::initRandomTPG(*tpg, params.mutation, rng, nbActions);
    // fill the archive before populating to test uniqueness of new prog
    TPG::TPGExecutionEngine tee(*e, &arch);
    for (auto rootVertex : tpg->getRootVertices()) {
        tee.executeFromRoot(*rootVertex);
    }

    // Check the correct execution
    ASSERT_NO_THROW(Mutator::TPGMutator::populateTPG(
        *tpg, selector, arch, params.mutation, rng, nbActions, 0))
        << "Populating a TPG failed.";
    // Check the number of roots
    ASSERT_EQ(tpg->getRootVertices().size(), params.mutation.tpg.nbRoots);

    // Increase coverage with a TPG that has no root team
    TPG::TPGGraph tpg2(*e);
    ASSERT_NO_THROW(Mutator::TPGMutator::populateTPG(
        tpg2, selector, arch, params.mutation, rng, nbActions, 0))
        << "Populating an empty TPG failed.";
}

TEST_F(MutatorTest, TPGMutatorPopulateActionRoots)
{
    Mutator::RNG rng;
    rng.setSeed(0);

    uint64_t nbActions = 5;
    Environment ce(set, params, vect, nbActions);

    std::shared_ptr<TPG::TPGGraph> tpg = std::make_shared<TPG::TPGGraph>(ce);

    params.mutation.tpg.maxInitOutgoingEdges = 3;
    params.mutation.prog.maxProgramSize = 96;
    params.mutation.tpg.nbRoots = 10;
    params.mutation.tpg.useActionProgram = true;
    params.mutation.tpg.useMultiActionProgram = true;
    params.mutation.tpg.ratioTeamsOverActions = 0.5;
    // Proba as in Kelly's paper
    params.mutation.tpg.pEdgeDeletion = 0.7;
    params.mutation.tpg.pEdgeAddition = 0.7;
    params.mutation.tpg.pProgramMutation = 0.2;
    params.mutation.tpg.pEdgeDestinationChange = 0.1;
    params.mutation.tpg.pEdgeDestinationIsAction = 0.5;
    params.mutation.prog.pAdd = 0.5;
    params.mutation.prog.pDelete = 0.5;
    params.mutation.prog.pMutate = 1.0;
    params.mutation.prog.pSwap = 1.0;
    params.mutation.prog.pConstantMutation = 0.5;
    params.mutation.prog.minConstValue = 0;
    params.mutation.prog.maxConstValue = 10;
    Archive arch;
    Selector::Selector selector(tpg, params);

    Mutator::TPGMutator::initRandomTPG(*tpg, params.mutation, rng, nbActions);
    // fill the archive before populating to test uniqueness of new prog
    TPG::TPGExecutionEngine tee(*e, &arch);
    for (auto rootVertex : tpg->getRootVertices()) {
        tee.executeFromRoot(*rootVertex);
    }

    // Check the correct execution
    ASSERT_NO_THROW(Mutator::TPGMutator::populateTPG(
        *tpg, selector, arch, params.mutation, rng, nbActions, 0))
        << "Populating a TPG failed.";
    // Check the number of roots
    ASSERT_EQ(tpg->getRootVertices().size(), params.mutation.tpg.nbRoots);

    size_t nbActionsRoots = 0;
    size_t nbTeamsRoots = 0;
    for (const auto& root : tpg->getRootVertices()) {
        if (dynamic_cast<const TPG::TPGAction*>(root)) {
            nbActionsRoots++;
        }
        else if (dynamic_cast<const TPG::TPGTeam*>(root)) {
            nbTeamsRoots++;
        }
    }
    // Check the ratio of teams over actions
    ASSERT_EQ(nbTeamsRoots, params.mutation.tpg.nbRoots *
                                params.mutation.tpg.ratioTeamsOverActions)
        << "The number of team roots is not as expected.";
    ASSERT_EQ(nbActionsRoots, params.mutation.tpg.nbRoots - nbTeamsRoots)
        << "The number of action roots is not as expected.";
}

TEST_F(MutatorTest, TPGMutatorPopulateTPGWithTournamentSelection)
{
    Mutator::RNG rng;
    rng.setSeed(0);

    uint64_t nbActions = 5;
    params.selection.selectionMode = "tournament";
    params.selection.tournament.sizeTournament = 2;
    params.selection.tournament.ratioSavedRoots = 0.1;
    Environment ce(set, params, vect, nbActions);

    std::shared_ptr<TPG::TPGGraph> tpg = std::make_shared<TPG::TPGGraph>(ce);

    params.mutation.tpg.maxInitOutgoingEdges = 3;
    params.mutation.prog.maxProgramSize = 96;
    params.mutation.tpg.nbRoots = 10;
    params.mutation.tpg.useActionProgram = true;
    params.mutation.tpg.useMultiActionProgram = true;
    params.mutation.tpg.ratioTeamsOverActions = 0.5;
    // Proba as in Kelly's paper
    params.mutation.tpg.pEdgeDeletion = 0.7;
    params.mutation.tpg.pEdgeAddition = 0.7;
    params.mutation.tpg.pProgramMutation = 0.2;
    params.mutation.tpg.pEdgeDestinationChange = 0.1;
    params.mutation.tpg.pEdgeDestinationIsAction = 0.5;
    params.mutation.prog.pAdd = 0.5;
    params.mutation.prog.pDelete = 0.5;
    params.mutation.prog.pMutate = 1.0;
    params.mutation.prog.pSwap = 1.0;
    params.mutation.prog.pConstantMutation = 0.5;
    params.mutation.prog.minConstValue = 0;
    params.mutation.prog.maxConstValue = 10;
    Archive arch;
    Selector::TournamentSelector selector(tpg, params);

    Mutator::TPGMutator::initRandomTPG(*tpg, params.mutation, rng, nbActions);
    // fill the archive before populating to test uniqueness of new prog
    TPG::TPGExecutionEngine tee(*e, &arch);

    // Do fake results to fill the verticesToDelete set.
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  const TPG::TPGVertex*>
        fakeResults;
    for (auto rootVertex : tpg->getRootVertices()) {
        std::shared_ptr<Learn::EvaluationResult> er =
            std::make_shared<Learn::EvaluationResult>(rng.getDouble(0, 1), 1);
        fakeResults.insert(std::make_pair(er, rootVertex));
    }
    selector.doSelection(fakeResults, rng);

    // Check the correct execution
    ASSERT_NO_THROW(Mutator::TPGMutator::populateTPG(
        *tpg, selector, arch, params.mutation, rng, nbActions, 0))
        << "Populating a TPG failed.";
    // Check the number of roots
    ASSERT_EQ(tpg->getRootVertices().size(), params.mutation.tpg.nbRoots);

    ASSERT_EQ(selector.getVerticesToDelete().size(), 0)
        << "After populateTPG with tournament selection, the set of vertices "
           "to delete should be empty.";
}
