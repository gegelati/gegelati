/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2021 - 2023) :
 *
 * Elinor Montmasson <elinor.montmasson@gmail.com> (2022)
 * Karol Desnos <kdesnos@insa-rennes.fr> (2021 - 2023)
 * Mickaël Dardaillon <mdardail@insa-rennes.fr> (2022)
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

#ifdef CODE_GENERATION
#include <cstddef>
#include <gtest/gtest.h>
#include <iostream>

#if defined(_MSC_VER) || (__MINGW32__)
// C++17 not available in gcc7 or clang7
#include <filesystem>
#endif

#include "environment.h"
#include "instructions/lambdaInstruction.h"
#include "instructions/set.h"
#include "tpg/tpgGraph.h"
#include "tpg/tpgVertex.h"

#include "codeGen/tpgGenerationEngineFactory.h"
#include "codeGen/tpgStackGenerationEngine.h"
#include "codeGen/tpgSwitchGenerationEngine.h"
#include "goldenReferenceComparison.h"

class TPGGenerationEngineTest : public ::testing::Test
{
  protected:
    std::string path;
    std::string cmdCompile;
    std::string cmdExec;
    const size_t s1{8};
    Instructions::Set set;
    Environment* e = nullptr;
    std::vector<std::reference_wrapper<const Data::DataHandler>> data;
    Data::PrimitiveTypeArray<double> currentState{s1};
    std::unique_ptr<CodeGen::TPGGenerationEngine> tpgGen = nullptr;
    TPG::TPGGraph* tpg = nullptr;

    virtual void SetUp()
    {
        path = TESTS_DAT_PATH;
        path += "codeGen";

        data.emplace_back(currentState);

        auto add = [](double a, double b) -> double { return a + b; };
        auto sub = [](double a, double b) -> double { return a - b; };
        set.add(*(new Instructions::LambdaInstruction<double, double>(
            add, "$0 = $1 + $2;")));
        set.add(*(new Instructions::LambdaInstruction<double, double>(
            sub, "$0 = $1 - $2;")));

        e = new Environment(set, data, 8);
        tpg = new TPG::TPGGraph(*e);

        cmdCompile = TESTS_DAT_PATH "codeGen/";
#if defined(_MSC_VER) || (__MINGW32__)
        // Set working directory to BIN_DIR_PATH where the "src" directory was
        // created.
        std::filesystem::current_path(BIN_DIR_PATH);
        cmdCompile += "compile.bat ";
        cmdExec = BIN_DIR_PATH "/bin/debug/";
#elif __GNUC__
        cmdCompile += "compile.sh ";
        cmdExec = "./bin/";
#endif
#ifdef DEBUG
        cmdCompile += "1 ";
#else
        cmdCompile += "0 ";
#endif // DEBUG
        cmdCompile += TESTS_DAT_PATH " ";
    }

    virtual void TearDown()
    {

        delete e;
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
        delete tpg;
    }
};

TEST_F(TPGGenerationEngineTest, ConstructorDestructor)
{
    ASSERT_NO_THROW(tpgGen =
                        std::make_unique<CodeGen::TPGStackGenerationEngine>(
                            "constructorWithStackSize", *tpg, "./src/"))
        << "Failed to construct a TPGGenerationEngine with a filename and a "
           "TPG, a path and the size of the call stack";

    ASSERT_NO_THROW(tpgGen.reset()) << "Destruction failed.";
}

TEST_F(TPGGenerationEngineTest, TPGGenerationEngineFactoryCreateSwitch)
{
    CodeGen::TPGGenerationEngineFactory factorySwitch;
    ASSERT_NO_THROW(tpgGen = factorySwitch.create("constructor", *tpg))
        << "Failed to construct a TPGGenerationEngine with a filename and a "
           "TPG";

    ASSERT_NE(dynamic_cast<CodeGen::TPGSwitchGenerationEngine*>(tpgGen.get()),
              nullptr)
        << "Created TPGGenerationEngine has incorrect type.";

    ASSERT_NO_THROW(tpgGen.reset()) << "Destruction failed.";

    ASSERT_NO_THROW(
        tpgGen = factorySwitch.create("constructorWithPath", *tpg, "./src/"))
        << "Failed to construct a TPGGenerationEngine with a filename and a "
           "TPG and a path";

    ASSERT_NO_THROW(tpgGen.reset()) << "Destruction failed.";

    std::fstream out;
    out.open("./src/rdOnly.c", std::ofstream::out);
    if (!out.is_open()) {
        out.open("./src/rdOnly.c", std::ofstream::in);
    }
    ASSERT_TRUE(out.is_open()) << "Error can't open file ./src/rdOnly.c";
    out.close();
    ASSERT_TRUE(!out.is_open()) << "Error can't close file ./src/rdOnly.c";

#if defined(_MSC_VER) || (__MINGW32__)
    ASSERT_EQ(system("attrib +R ./src/rdOnly.c"), 0)
        << "Fail to change the file as read only";
#elif __GNUC__
    ASSERT_EQ(system("chmod 444 ./src/rdOnly.c"), 0)
        << "Fail to change the file as read only";
#endif

    ASSERT_THROW(tpgGen = factorySwitch.create("rdOnly", *tpg, "./src/"),
                 std::runtime_error)
        << "Construction should fail because the file rdOnly is in read only "
           "status.";
}

TEST_F(TPGGenerationEngineTest, TPGGenerationEngineFactoryCreateStack)
{
    auto& team = tpg->addNewTeam();
    auto& action = tpg->addNewAction(0);
    tpg->addNewEdge(team, action, std::make_shared<Program::Program>(*e));

    CodeGen::TPGGenerationEngineFactory factoryStack(
        CodeGen::TPGGenerationEngineFactory::generationEngineMode::stackMode);
    ASSERT_NO_THROW(tpgGen = factoryStack.create("constructor", *tpg))
        << "Failed to construct a TPGGenerationEngine with a filename and a "
           "TPG";

    ASSERT_NE(dynamic_cast<CodeGen::TPGStackGenerationEngine*>(tpgGen.get()),
              nullptr)
        << "Created TPGGenerationEngine has incorrect type.";

    ASSERT_NO_THROW(tpgGen.reset()) << "Destruction failed.";

    ASSERT_NO_THROW(
        tpgGen = factoryStack.create("constructorWithPath", *tpg, "./src/"))
        << "Failed to construct a TPGGenerationEngine with a filename and a "
           "TPG and a path";

    ASSERT_NO_THROW(tpgGen.reset()) << "Destruction failed.";
}

TEST_F(TPGGenerationEngineTest, TPGGenerationEngineFactoryCreateNoMode)
{
    // Create the factory with a non-existing mode.
    CodeGen::TPGGenerationEngineFactory factoryStack(
        (CodeGen::TPGGenerationEngineFactory::generationEngineMode)-1);
    ASSERT_NO_THROW(tpgGen = factoryStack.create("constructor", *tpg))
        << "Failed to construct a TPGGenerationEngine with a filename and a "
           "TPG";
    ASSERT_EQ(tpgGen, nullptr) << "Factory should return a null pointer when a "
                                  "non existing mode is used.";
}

TEST_F(TPGGenerationEngineTest, OneLeafNoInstruction)
{
    const TPG::TPGVertex* leaf = (&tpg->addNewAction(1));
    const TPG::TPGVertex* root = (&tpg->addNewTeam());

    const std::shared_ptr<Program::Program> progA(new Program::Program(*e));

    tpg->addNewEdge(*root, *leaf, progA);

    ASSERT_EQ(tpg->getNbRootVertices(), 1)
        << "number of root is not 1 in OneLeafNoInstruction";

    ASSERT_EQ(tpg->getNbVertices(), 2)
        << "bad number of vertices in OneLeafNoInstruction";

    ASSERT_EQ(tpg->getEdges().size(), 1)
        << "bad number of edges in OneLeafNoInstruction";

    CodeGen::TPGGenerationEngineFactory factory;
    tpgGen = factory.create("OneLeafNoInstruction", *tpg, "./src/");
    tpgGen->generateTPGGraph();
    // call the destructor to close the file
    tpgGen.reset();

    std::vector<std::string> fileGenerated{
        "OneLeafNoInstruction.c", "OneLeafNoInstruction.h",
        "OneLeafNoInstruction_program.c", "OneLeafNoInstruction_program.h"};

    ASSERT_TRUE(compare_files("./src/" + fileGenerated[0],
                              TESTS_DAT_PATH "codeGen/OneLeafNoInstruction/" +
                                  fileGenerated[0] + "_ref"))
        << "Error the source file holding the functions of the node of TGP "
           "generated is different from the golden reference.";
    ASSERT_TRUE(compare_files("./src/" + fileGenerated[1],
                              TESTS_DAT_PATH "codeGen/OneLeafNoInstruction/" +
                                  fileGenerated[1] + "_ref"))
        << "Error the header file holding the functions of the node of TGP "
           "generated is different from the golden reference.";
    ASSERT_TRUE(compare_files("./src/" + fileGenerated[2],
                              TESTS_DAT_PATH "codeGen/OneLeafNoInstruction/" +
                                  fileGenerated[2] + "_ref"))
        << "Error the source file holding the functions of the program of TGP "
           "generated is different from the golden reference.";
    ASSERT_TRUE(compare_files("./src/" + fileGenerated[3],
                              TESTS_DAT_PATH "codeGen/OneLeafNoInstruction/" +
                                  fileGenerated[3] + "_ref"))
        << "Error the header file holding the functions of the program of TGP "
           "generated is different from the golden reference.";

    cmdCompile += "OneLeafNoInstruction";
    ASSERT_EQ(system(cmdCompile.c_str()), 0)
        << "Compilation failed in OneLeafNoInstruction.";
}

/// Extension for built executables
#if defined(_MSC_VER) || (__MINGW32__)
std::string executableExtension = ".exe ";
#elif __GNUC__
std::string executableExtension = " ";
#endif

#define TEST_BOTH_MODE(TEST_NAME, TEST_CODE)                                   \
    TEST_F(TPGGenerationEngineTest, TEST_NAME##Switch)                         \
    {                                                                          \
        CodeGen::TPGGenerationEngineFactory factory(                           \
            CodeGen::TPGGenerationEngineFactory::generationEngineMode::        \
                switchMode);                                                   \
        TEST_CODE                                                              \
    }                                                                          \
    TEST_F(TPGGenerationEngineTest, TEST_NAME##Stack)                          \
    {                                                                          \
        CodeGen::TPGGenerationEngineFactory factory(                           \
            CodeGen::TPGGenerationEngineFactory::generationEngineMode::        \
                stackMode);                                                    \
        TEST_CODE                                                              \
    }

TEST_BOTH_MODE(OneLeaf, {
    const TPG::TPGVertex* leaf = (&tpg->addNewAction(1));
    const TPG::TPGVertex* root = (&tpg->addNewTeam());

    const std::shared_ptr<Program::Program> prog1(new Program::Program(*e));
    Program::Line& prog1L1 = prog1->addNewLine();
    // reg[0] = in1[0] - reg[1]
    prog1L1.setDestinationIndex(0);
    prog1L1.setInstructionIndex(1);
    prog1L1.setOperand(0, 1, 0);
    prog1L1.setOperand(1, 0, 1);

    prog1->identifyIntrons();

    tpg->addNewEdge(*root, *leaf, prog1);

    ASSERT_EQ(tpg->getNbRootVertices(), 1)
        << "number of root is not 1 in OneLeaf";

    ASSERT_EQ(tpg->getNbVertices(), 2) << "bad number of vertices in OneLeaf";

    ASSERT_EQ(tpg->getEdges().size(), 1) << "bad number of edges in OneLeaf";

    tpgGen = factory.create("OneLeaf", *tpg, "./src/");
    tpgGen->generateTPGGraph();
    // call the destructor to close the file
    tpgGen.reset();
    cmdCompile += "OneLeaf";
    ASSERT_EQ(system(cmdCompile.c_str()), 0);

    cmdExec += "OneLeaf" + executableExtension;

    cmdExec += " 1 4.5";
    std::cout << cmdExec << std::endl;
    ASSERT_EQ(system(cmdExec.c_str()), 0)
        << "Error wrong action returned in test OneLeaf.";
});

TEST_BOTH_MODE(TwoLeaves, {
    const TPG::TPGVertex* leaf = (&tpg->addNewAction(1));
    const TPG::TPGVertex* leaf2 = (&tpg->addNewAction(2));
    const TPG::TPGVertex* root = (&tpg->addNewTeam());

    const std::shared_ptr<Program::Program> prog1(new Program::Program(*e));
    Program::Line& prog1L1 = prog1->addNewLine();
    // reg[0] = in1[0] + in1[1];
    prog1L1.setDestinationIndex(0);
    prog1L1.setInstructionIndex(0);
    prog1L1.setOperand(0, 1, 0);
    prog1L1.setOperand(1, 1, 1);

    const std::shared_ptr<Program::Program> prog2(new Program::Program(*e));
    Program::Line& prog2L1 = prog2->addNewLine();
    // reg[0] = in1[0] + in1[2];
    prog2L1.setDestinationIndex(0);
    prog2L1.setInstructionIndex(0);
    prog2L1.setOperand(0, 1, 0);
    prog2L1.setOperand(1, 1, 2);

    tpg->addNewEdge(*root, *leaf, prog1);
    tpg->addNewEdge(*root, *leaf2, prog2);

    ASSERT_EQ(tpg->getNbRootVertices(), 1)
        << "number of root is not 1 in TwoLeaves";

    ASSERT_EQ(tpg->getNbVertices(), 3) << "bad number of vertices in TwoLeaves";

    ASSERT_EQ(tpg->getEdges().size(), 2) << "bad number of edges in TwoLeaves";

    tpgGen = factory.create("TwoLeaves", *tpg, "./src/");
    tpgGen->generateTPGGraph();
    // call the destructor to close the file
    tpgGen.reset();
    cmdCompile += "TwoLeaves";
    ASSERT_EQ(system(cmdCompile.c_str()), 0)
        << "Error while compiling the test TwoLeaves.";

    cmdExec += "TwoLeaves" + executableExtension;

    ASSERT_EQ(system((cmdExec + path + "/TwoLeaves/DataTwoLeaves.csv").c_str()),
              0)
        << "Error wrong action returned in test TwoLeaves.";
});

TEST_BOTH_MODE(ThreeLeaves, {
    // P1 < P2 = P3
    const TPG::TPGVertex* leaf = (&tpg->addNewAction(1));
    const TPG::TPGVertex* leaf2 = (&tpg->addNewAction(2));
    const TPG::TPGVertex* leaf3 = (&tpg->addNewAction(3));
    const TPG::TPGVertex* root = (&tpg->addNewTeam());

    const std::shared_ptr<Program::Program> prog1(new Program::Program(*e));
    Program::Line& prog1L1 = prog1->addNewLine();
    // reg[0] = in1[0] + in1[1];
    prog1L1.setDestinationIndex(0);
    prog1L1.setInstructionIndex(0);
    prog1L1.setOperand(0, 1, 0);
    prog1L1.setOperand(1, 1, 1);

    const std::shared_ptr<Program::Program> prog2(new Program::Program(*e));
    Program::Line& prog2L1 = prog2->addNewLine();
    // reg[0] = in1[0] + in1[2];
    prog2L1.setDestinationIndex(0);
    prog2L1.setInstructionIndex(0);
    prog2L1.setOperand(0, 1, 0);
    prog2L1.setOperand(1, 1, 2);

    const std::shared_ptr<Program::Program> prog3(new Program::Program(*e));
    Program::Line& prog3L1 = prog3->addNewLine();
    // reg[0] = in1[0] + in1[3];
    prog3L1.setDestinationIndex(0);
    prog3L1.setInstructionIndex(0);
    prog3L1.setOperand(0, 1, 0);
    prog3L1.setOperand(1, 1, 3);

    tpg->addNewEdge(*root, *leaf, prog1);
    tpg->addNewEdge(*root, *leaf2, prog2);
    tpg->addNewEdge(*root, *leaf3, prog3);

    ASSERT_EQ(tpg->getNbRootVertices(), 1)
        << "number of root is not 1 in ThreeLeaves.";

    ASSERT_EQ(tpg->getNbVertices(), 4)
        << "bad number of vertices in ThreeLeaves.";

    ASSERT_EQ(tpg->getEdges().size(), 3)
        << "bad number of edges in ThreeLeaves.";

    tpgGen = factory.create("ThreeLeaves", *tpg, "./src/");
    tpgGen->generateTPGGraph();
    // call the destructor to close the file
    tpgGen.reset();
    cmdCompile += "ThreeLeaves";
    ASSERT_EQ(system(cmdCompile.c_str()), 0)
        << "Error while compiling the test ThreeLeaves.";

    cmdExec += "ThreeLeaves" + executableExtension;

    ASSERT_EQ(
        system((cmdExec + path + "/ThreeLeaves/DataThreeLeaves.csv").c_str()),
        0)
        << "Error wrong action returned in test ThreeLeaves.";
});

TEST_BOTH_MODE(OneTeamOneLeaf, {
    const TPG::TPGVertex* root = (&tpg->addNewTeam());
    const TPG::TPGVertex* T1 = (&tpg->addNewTeam());
    const TPG::TPGVertex* leaf = (&tpg->addNewAction(1));

    const std::shared_ptr<Program::Program> prog1(new Program::Program(*e));
    Program::Line& prog1L1 = prog1->addNewLine();
    // reg[0] = in1[0] + reg[1];
    prog1L1.setDestinationIndex(0);
    prog1L1.setInstructionIndex(0);
    prog1L1.setOperand(0, 1, 0);
    prog1L1.setOperand(1, 0, 1);

    const std::shared_ptr<Program::Program> prog2(new Program::Program(*e));
    Program::Line& prog2L1 = prog2->addNewLine();
    // reg[0] = in1[0] - in1[1];
    prog2L1.setDestinationIndex(0);
    prog2L1.setInstructionIndex(1);
    prog2L1.setOperand(0, 1, 0);
    prog2L1.setOperand(1, 1, 1);

    TPG::TPGEdge edge1 = tpg->addNewEdge(*root, *T1, prog1);
    TPG::TPGEdge edge2 = tpg->addNewEdge(*T1, *leaf, prog2);

    ASSERT_EQ(tpg->getNbRootVertices(), 1)
        << "number of root is not 1 in OneTeamOneLeaf";

    ASSERT_EQ(tpg->getNbVertices(), 3)
        << "bad number of vertices in OneTeamOneLeaf";

    ASSERT_EQ(tpg->getEdges().size(), 2)
        << "bad number of edges in OneTeamOneLeaf";

    tpgGen = factory.create("OneTeamOneLeaf", *tpg, "./src/");
    tpgGen->generateTPGGraph();
    // call the destructor to close the file
    tpgGen.reset();
    cmdCompile += "OneTeamOneLeaf";
    ASSERT_EQ(system(cmdCompile.c_str()), 0)
        << "Error while compiling the test OneTeamOneLeaf";

    cmdExec += "OneTeamOneLeaf" + executableExtension;

    cmdExec += "1 4.5 6.8";

    ASSERT_EQ(system(cmdExec.c_str()), 0)
        << "Error wrong action returned in test OneTeamOneLeaf";
});

TEST_BOTH_MODE(OneTeamTwoLeaves, {
    const TPG::TPGVertex* root = (&tpg->addNewTeam());
    const TPG::TPGVertex* T1 = (&tpg->addNewTeam());
    const TPG::TPGVertex* leaf = (&tpg->addNewAction(1));
    const TPG::TPGVertex* leaf2 = (&tpg->addNewAction(2));

    const std::shared_ptr<Program::Program> prog1(new Program::Program(*e));
    Program::Line& prog1L1 = prog1->addNewLine();
    // reg[0] = in1[0] - reg[1];
    prog1L1.setDestinationIndex(0);
    prog1L1.setInstructionIndex(0);
    prog1L1.setOperand(0, 1, 0);
    prog1L1.setOperand(1, 0, 1);

    const std::shared_ptr<Program::Program> prog2(new Program::Program(*e));
    Program::Line& prog2L1 = prog2->addNewLine();
    // reg[0] = in1[0] - in1[1];
    prog2L1.setDestinationIndex(0);
    prog2L1.setInstructionIndex(1);
    prog2L1.setOperand(0, 1, 0);
    prog2L1.setOperand(1, 1, 1);

    const std::shared_ptr<Program::Program> prog3(new Program::Program(*e));
    Program::Line& prog3L1 = prog3->addNewLine();
    // reg[0] = in1[0] + in1[1];
    prog3L1.setDestinationIndex(0);
    prog3L1.setInstructionIndex(0);
    prog3L1.setOperand(0, 1, 0);
    prog3L1.setOperand(1, 1, 1);

    tpg->addNewEdge(*root, *T1, prog1);
    tpg->addNewEdge(*T1, *leaf, prog2);
    tpg->addNewEdge(*T1, *leaf2, prog3);
    ASSERT_EQ(tpg->getNbRootVertices(), 1)
        << "number of root is not 1 in OneTeamTwoLeaves";

    ASSERT_EQ(tpg->getNbVertices(), 4)
        << "bad number of vertices in OneTeamTwoLeaves";

    ASSERT_EQ(tpg->getEdges().size(), 3)
        << "bad number of edges in OneTeamTwoLeaves";

    tpgGen = factory.create("OneTeamTwoLeaves", *tpg, "./src/");
    tpgGen->generateTPGGraph();
    // call the destructor to close the file
    tpgGen.reset();
    cmdCompile += "OneTeamTwoLeaves";
    ASSERT_EQ(system(cmdCompile.c_str()), 0)
        << "Error while compiling the test OneTeamTwoLeaves.";

    cmdExec += "OneTeamTwoLeaves" + executableExtension;

    ASSERT_EQ(
        system((cmdExec + path + "/OneTeamTwoLeaves/DataOneTeamTwoLeaves.csv")
                   .c_str()),
        0)
        << "Error wrong action returned in test OneTeamTwoLeaves.";
});

TEST_BOTH_MODE(TwoTeams, {
    const TPG::TPGVertex* root = (&tpg->addNewTeam());
    const TPG::TPGVertex* T1 = (&tpg->addNewTeam());
    const TPG::TPGVertex* T2 = (&tpg->addNewTeam());
    const TPG::TPGVertex* leaf = (&tpg->addNewAction(1));
    const TPG::TPGVertex* leaf2 = (&tpg->addNewAction(2));

    const std::shared_ptr<Program::Program> prog1(new Program::Program(*e));
    Program::Line& prog1L1 = prog1->addNewLine();
    // reg[0] = in1[0] + in1[1];
    prog1L1.setDestinationIndex(0);
    prog1L1.setInstructionIndex(0);
    prog1L1.setOperand(0, 1, 0);
    prog1L1.setOperand(1, 1, 1);

    const std::shared_ptr<Program::Program> prog2(new Program::Program(*e));
    Program::Line& prog2L1 = prog2->addNewLine();
    // reg[0] = in1[1] + in1[2];
    prog2L1.setDestinationIndex(0);
    prog2L1.setInstructionIndex(0);
    prog2L1.setOperand(0, 1, 1);
    prog2L1.setOperand(1, 1, 2);

    const std::shared_ptr<Program::Program> prog3(new Program::Program(*e));
    Program::Line& prog3L1 = prog3->addNewLine();
    // reg[0] = in1[1] + in1[3];
    prog3L1.setDestinationIndex(0);
    prog3L1.setInstructionIndex(0);
    prog3L1.setOperand(0, 1, 1);
    prog3L1.setOperand(1, 1, 3);

    const std::shared_ptr<Program::Program> prog4(new Program::Program(*e));
    Program::Line& prog4L1 = prog4->addNewLine();
    // reg[0] = in1[1] + in1[4];
    prog4L1.setDestinationIndex(0);
    prog4L1.setInstructionIndex(0);
    prog4L1.setOperand(0, 1, 1);
    prog4L1.setOperand(1, 1, 4);

    tpg->addNewEdge(*root, *T1, prog1);
    tpg->addNewEdge(*T1, *leaf, prog2);
    tpg->addNewEdge(*T1, *T2, prog3);
    tpg->addNewEdge(*T2, *leaf2, prog4);

    ASSERT_EQ(tpg->getNbRootVertices(), 1)
        << "number of root is not 1 in TwoTeams";

    ASSERT_EQ(tpg->getNbVertices(), 5) << "bad number of vertices in TwoTeams";

    ASSERT_EQ(tpg->getEdges().size(), 4) << "bad number of edges in TwoTeams";

    tpgGen = factory.create("TwoTeams", *tpg, "./src/");
    tpgGen->generateTPGGraph();
    // call the destructor to close the file
    tpgGen.reset();

    cmdCompile += "TwoTeams";
    ASSERT_EQ(system(cmdCompile.c_str()), 0)
        << "Error while compiling the test TwoTeams.";

    cmdExec += "TwoTeams" + executableExtension;

    ASSERT_EQ(system((cmdExec + path + "/TwoTeams/DataTwoTeams.csv").c_str()),
              0)
        << "Error wrong action returned in test TwoTeams.";
});

TEST_BOTH_MODE(TwoTeamsNegativeBid, {
    const TPG::TPGVertex* root = (&tpg->addNewTeam());
    const TPG::TPGVertex* T1 = (&tpg->addNewTeam());
    const TPG::TPGVertex* T2 = (&tpg->addNewTeam());
    const TPG::TPGVertex* leaf = (&tpg->addNewAction(1));
    const TPG::TPGVertex* leaf2 = (&tpg->addNewAction(2));
    const TPG::TPGVertex* leaf3 = (&tpg->addNewAction(3));

    const std::shared_ptr<Program::Program> prog1(new Program::Program(*e));
    Program::Line& prog1L1 = prog1->addNewLine();
    // reg[0] = in1[0] + in1[1];
    prog1L1.setDestinationIndex(0);
    prog1L1.setInstructionIndex(0);
    prog1L1.setOperand(0, 1, 0);
    prog1L1.setOperand(1, 1, 1);

    const std::shared_ptr<Program::Program> prog2(new Program::Program(*e));
    Program::Line& prog2L1 = prog2->addNewLine();
    // reg[0] = in1[1] - in1[2];
    prog2L1.setDestinationIndex(0);
    prog2L1.setInstructionIndex(1);
    prog2L1.setOperand(0, 1, 1);
    prog2L1.setOperand(1, 1, 2);

    const std::shared_ptr<Program::Program> prog3(new Program::Program(*e));
    Program::Line& prog3L1 = prog3->addNewLine();
    // reg[0] = in1[1] + in1[3];
    prog3L1.setDestinationIndex(0);
    prog3L1.setInstructionIndex(0);
    prog3L1.setOperand(0, 1, 1);
    prog3L1.setOperand(1, 1, 3);

    const std::shared_ptr<Program::Program> prog4(new Program::Program(*e));
    Program::Line& prog4L1 = prog4->addNewLine();
    // reg[0] = in1[1] + in1[4];
    prog4L1.setDestinationIndex(0);
    prog4L1.setInstructionIndex(0);
    prog4L1.setOperand(0, 1, 1);
    prog4L1.setOperand(1, 1, 4);

    const std::shared_ptr<Program::Program> prog5(new Program::Program(*e));
    Program::Line& prog5L1 = prog5->addNewLine();
    // reg[0] = in1[1] + in1[6];
    prog5L1.setDestinationIndex(0);
    prog5L1.setInstructionIndex(0);
    prog5L1.setOperand(0, 1, 1);
    prog5L1.setOperand(1, 1, 6);

    tpg->addNewEdge(*root, *T1, prog1);
    tpg->addNewEdge(*T1, *leaf, prog2);
    tpg->addNewEdge(*T1, *T2, prog3);
    tpg->addNewEdge(*T2, *leaf2, prog4);
    tpg->addNewEdge(*T2, *leaf3, prog5);

    ASSERT_EQ(tpg->getNbRootVertices(), 1)
        << "number of root is not 1 in TwoTeamsNegativeBid";

    ASSERT_EQ(tpg->getNbVertices(), 6)
        << "bad number of vertices in TwoTeamsNegativeBid";

    ASSERT_EQ(tpg->getEdges().size(), 5)
        << "bad number of edges in TwoTeamsNegativeBid";

    tpgGen = factory.create("TwoTeamsNegativeBid", *tpg, "./src/");
    tpgGen->generateTPGGraph();
    // call the destructor to close the file
    tpgGen.reset();

    cmdCompile += "TwoTeamsNegativeBid";
    ASSERT_EQ(system(cmdCompile.c_str()), 0)
        << "Error while compiling the test TwoTeamsNegativeBid.";

    cmdExec += "TwoTeamsNegativeBid" + executableExtension;

    ASSERT_EQ(system((cmdExec + path +
                      "/TwoTeamsNegativeBid/DataTwoTeamsNegativeBid.csv")
                         .c_str()),
              0)
        << "Error wrong action returned in test TwoTeamsNegativeBid.";
});

static void setProgLine(const std::shared_ptr<Program::Program> prog,
                        int operand)
{
    Program::Line& line = prog->addNewLine();
    // reg[0] = in1[operand] + reg[8] (= 0)
    line.setDestinationIndex(0);
    line.setInstructionIndex(0);
    line.setOperand(0, 1, operand);
    line.setOperand(1, 0, 1);
}

TEST_BOTH_MODE(ThreeTeamsThreeLeaves, {
    const TPG::TPGVertex* A1 = (&tpg->addNewAction(1));
    const TPG::TPGVertex* A2 = (&tpg->addNewAction(2));
    const TPG::TPGVertex* A0 = (&tpg->addNewAction(0));
    const TPG::TPGVertex* T1 = (&tpg->addNewTeam());
    const TPG::TPGVertex* T2 = (&tpg->addNewTeam());
    const TPG::TPGVertex* T3 = (&tpg->addNewTeam());

    const std::shared_ptr<Program::Program> prog1(new Program::Program(*e));
    const std::shared_ptr<Program::Program> prog2(new Program::Program(*e));
    const std::shared_ptr<Program::Program> prog3(new Program::Program(*e));
    const std::shared_ptr<Program::Program> prog4(new Program::Program(*e));
    const std::shared_ptr<Program::Program> prog5(new Program::Program(*e));
    const std::shared_ptr<Program::Program> prog6(new Program::Program(*e));

    setProgLine(prog1, 0);
    // reg[0] = in1[0] + reg[1] (reg[1] = 0)
    setProgLine(prog2, 1);
    // reg[0] = in1[1] + reg[1] (reg[1] = 0)
    setProgLine(prog3, 2);
    // reg[0] = in1[2] + reg[1] (reg[1] = 0)
    setProgLine(prog4, 3);
    // reg[0] = in1[3] + reg[1] (reg[1] = 0)
    setProgLine(prog5, 4);
    // reg[0] = in1[4] + reg[1] (reg[1] = 0)
    setProgLine(prog6, 5);
    // reg[0] = in1[5] + reg[1] (reg[1] = 0)

    tpg->addNewEdge(*T1, *T2, prog1);
    tpg->addNewEdge(*T1, *A1, prog2);
    tpg->addNewEdge(*T1, *T3, prog3);

    tpg->addNewEdge(*T2, *A0, prog4);
    tpg->addNewEdge(*T2, *T3, prog5);

    tpg->addNewEdge(*T3, *A2, prog6);

    ASSERT_EQ(tpg->getNbRootVertices(), 1)
        << "number of root is not 1 in ThreeTeamsThreeLeaves";

    ASSERT_EQ(tpg->getNbVertices(), 6)
        << "bad number of vertices in ThreeTeamsThreeLeaves";

    ASSERT_EQ(tpg->getEdges().size(), 6)
        << "bad number of edges in ThreeTeamsThreeLeaves";

    tpgGen = factory.create("ThreeTeamsThreeLeaves", *tpg, "./src/");
    tpgGen->generateTPGGraph();
    // call the destructor to close the file
    tpgGen.reset();
    cmdCompile += "ThreeTeamsThreeLeaves";
    ASSERT_EQ(system(cmdCompile.c_str()), 0)
        << "Error while compiling the test ThreeTeamsThreeLeaves";

    cmdExec += "ThreeTeamsThreeLeaves" + executableExtension;

    ASSERT_EQ(system((cmdExec + path +
                      "/ThreeTeamsThreeLeaves/"
                      "DataThreeTeamsThreeLeaves.csv")
                         .c_str()),
              0)
        << "Error wrong action returned in test "
           "ThreeTeamsThreeLeaves.";
});

TEST_F(TPGGenerationEngineTest, codeGenMultiActionSwitchDisabled)
{
    tpg->setNbEdgesActivable(2);

    CodeGen::TPGGenerationEngineFactory factory(
        CodeGen::TPGGenerationEngineFactory::generationEngineMode::switchMode);

    const TPG::TPGVertex* leaf = (&tpg->addNewAction(1, 0));
    const TPG::TPGVertex* leaf2 = (&tpg->addNewAction(1, 1));
    const TPG::TPGVertex* root = (&tpg->addNewTeam());

    const std::shared_ptr<Program::Program> prog1(new Program::Program(*e));
    Program::Line& prog1L1 = prog1->addNewLine();
    // reg[0] = in1[0] + in1[1];
    prog1L1.setDestinationIndex(0);
    prog1L1.setInstructionIndex(0);
    prog1L1.setOperand(0, 1, 0);
    prog1L1.setOperand(1, 1, 1);

    const std::shared_ptr<Program::Program> prog2(new Program::Program(*e));
    Program::Line& prog2L1 = prog2->addNewLine();
    // reg[0] = in1[0] + in1[2];
    prog2L1.setDestinationIndex(0);
    prog2L1.setInstructionIndex(0);
    prog2L1.setOperand(0, 1, 0);
    prog2L1.setOperand(1, 1, 2);

    tpg->addNewEdge(*root, *leaf, prog1);
    tpg->addNewEdge(*root, *leaf2, prog2);

    ASSERT_EQ(tpg->getNbRootVertices(), 1)
        << "number of root is not 1 in TwoLeaves";

    ASSERT_EQ(tpg->getNbVertices(), 3) << "bad number of vertices in TwoLeaves";

    ASSERT_EQ(tpg->getEdges().size(), 2) << "bad number of edges in TwoLeaves";

    tpgGen = factory.create("TwoLeavesMultiAction", *tpg, "./src/");

    ASSERT_THROW(tpgGen->generateTPGGraph(), std::runtime_error)
        << "Generation of code should fail on switch mode with a multiAction "
           "case.";
}

TEST_F(TPGGenerationEngineTest, codeGenMultiActionStackToMuchEdgesActivable)
{
    tpg->setNbEdgesActivable(3);

    CodeGen::TPGGenerationEngineFactory factory(
        CodeGen::TPGGenerationEngineFactory::generationEngineMode::stackMode);

    const TPG::TPGVertex* leaf = (&tpg->addNewAction(1, 0));
    const TPG::TPGVertex* leaf2 = (&tpg->addNewAction(1, 1));
    const TPG::TPGVertex* root = (&tpg->addNewTeam());

    const std::shared_ptr<Program::Program> prog1(new Program::Program(*e));
    Program::Line& prog1L1 = prog1->addNewLine();
    // reg[0] = in1[0] + in1[1];
    prog1L1.setDestinationIndex(0);
    prog1L1.setInstructionIndex(0);
    prog1L1.setOperand(0, 1, 0);
    prog1L1.setOperand(1, 1, 1);

    const std::shared_ptr<Program::Program> prog2(new Program::Program(*e));
    Program::Line& prog2L1 = prog2->addNewLine();
    // reg[0] = in1[0] + in1[2];
    prog2L1.setDestinationIndex(0);
    prog2L1.setInstructionIndex(0);
    prog2L1.setOperand(0, 1, 0);
    prog2L1.setOperand(1, 1, 2);

    tpg->addNewEdge(*root, *leaf, prog1);
    tpg->addNewEdge(*root, *leaf2, prog2);

    ASSERT_EQ(tpg->getNbRootVertices(), 1)
        << "number of root is not 1 in TwoLeaves";

    ASSERT_EQ(tpg->getNbVertices(), 3) << "bad number of vertices in TwoLeaves";

    ASSERT_EQ(tpg->getEdges().size(), 2) << "bad number of edges in TwoLeaves";

    tpgGen = factory.create("TwoLeavesMultiAction", *tpg, "./src/");

    ASSERT_THROW(tpgGen->generateTPGGraph(), std::runtime_error)
        << "Generation of code should fail on stack mode with the "
           "nbEdgesActivable set to three.";
}

TEST_F(TPGGenerationEngineTest, TwoLeavesStackMultiAction)
{

    tpg->setNbEdgesActivable(2);

    CodeGen::TPGGenerationEngineFactory factory(
        CodeGen::TPGGenerationEngineFactory::generationEngineMode::stackMode);

    const TPG::TPGVertex* leaf = (&tpg->addNewAction(1, 0));
    const TPG::TPGVertex* leaf2 = (&tpg->addNewAction(1, 1));
    const TPG::TPGVertex* root = (&tpg->addNewTeam());

    const std::shared_ptr<Program::Program> prog1(new Program::Program(*e));
    Program::Line& prog1L1 = prog1->addNewLine();
    // reg[0] = in1[0] + in1[1];
    prog1L1.setDestinationIndex(0);
    prog1L1.setInstructionIndex(0);
    prog1L1.setOperand(0, 1, 0);
    prog1L1.setOperand(1, 1, 1);

    const std::shared_ptr<Program::Program> prog2(new Program::Program(*e));
    Program::Line& prog2L1 = prog2->addNewLine();
    // reg[0] = in1[0] + in1[2];
    prog2L1.setDestinationIndex(0);
    prog2L1.setInstructionIndex(0);
    prog2L1.setOperand(0, 1, 0);
    prog2L1.setOperand(1, 1, 2);

    tpg->addNewEdge(*root, *leaf, prog1);
    tpg->addNewEdge(*root, *leaf2, prog2);
    ASSERT_EQ(tpg->getNbRootVertices(), 1)
        << "number of root is not 1 in TwoLeaves";

    ASSERT_EQ(tpg->getNbVertices(), 3) << "bad number of vertices in TwoLeaves";

    ASSERT_EQ(tpg->getEdges().size(), 2) << "bad number of edges in TwoLeaves";

    tpgGen = factory.create("TwoLeavesMultiAction", *tpg, "./src/");
    tpgGen->generateTPGGraph();
    // call the destructor to close the file
    tpgGen.reset();
    cmdCompile += "TwoLeavesMultiAction";
    ASSERT_EQ(system(cmdCompile.c_str()), 0)
        << "Error while compiling the test TwoLeaves MultiAction.";
    cmdExec += "TwoLeavesMultiAction" + executableExtension;

    ASSERT_EQ(system((cmdExec + path +
                      "/TwoLeavesMultiAction/DataTwoLeavesMultiAction.csv")
                         .c_str()),
              0)
        << "Error wrong action returned in test TwoLeaves.";
};

TEST_F(TPGGenerationEngineTest, FourLeavesStackMultiAction)
{

    tpg->setNbEdgesActivable(2);

    CodeGen::TPGGenerationEngineFactory factory(
        CodeGen::TPGGenerationEngineFactory::generationEngineMode::stackMode);

    const TPG::TPGVertex* leaf = (&tpg->addNewAction(1, 0));
    const TPG::TPGVertex* leaf2 = (&tpg->addNewAction(2, 0));
    const TPG::TPGVertex* leaf3 = (&tpg->addNewAction(1, 1));
    const TPG::TPGVertex* leaf4 = (&tpg->addNewAction(2, 1));
    const TPG::TPGVertex* root = (&tpg->addNewTeam());

    const std::shared_ptr<Program::Program> prog1(new Program::Program(*e));
    Program::Line& prog1L1 = prog1->addNewLine();
    // reg[0] = in1[0] + in1[1];
    prog1L1.setDestinationIndex(0);
    prog1L1.setInstructionIndex(0);
    prog1L1.setOperand(0, 1, 0);
    prog1L1.setOperand(1, 1, 1);

    const std::shared_ptr<Program::Program> prog2(new Program::Program(*e));
    Program::Line& prog2L1 = prog2->addNewLine();
    // reg[0] = in1[0] + in1[2];
    prog2L1.setDestinationIndex(0);
    prog2L1.setInstructionIndex(0);
    prog2L1.setOperand(0, 1, 0);
    prog2L1.setOperand(1, 1, 2);

    const std::shared_ptr<Program::Program> prog3(new Program::Program(*e));
    Program::Line& prog3L1 = prog3->addNewLine();
    // reg[0] = in1[1] + in1[3];
    prog3L1.setDestinationIndex(0);
    prog3L1.setInstructionIndex(0);
    prog3L1.setOperand(0, 1, 1);
    prog3L1.setOperand(1, 1, 3);

    const std::shared_ptr<Program::Program> prog4(new Program::Program(*e));
    Program::Line& prog4L1 = prog4->addNewLine();
    // reg[0] = in1[1] - in1[3];
    prog4L1.setDestinationIndex(0);
    prog4L1.setInstructionIndex(0);
    prog4L1.setOperand(0, 1, 1);
    prog4L1.setOperand(1, 1, 4);

    tpg->addNewEdge(*root, *leaf, prog1);
    tpg->addNewEdge(*root, *leaf2, prog2);
    tpg->addNewEdge(*root, *leaf3, prog3);
    tpg->addNewEdge(*root, *leaf4, prog4);

    ASSERT_EQ(tpg->getNbRootVertices(), 1)
        << "number of root is not 1 in FourLeaves";

    ASSERT_EQ(tpg->getNbVertices(), 5)
        << "bad number of vertices in FourLeaves";

    ASSERT_EQ(tpg->getEdges().size(), 4) << "bad number of edges in FourLeaves";

    tpgGen = factory.create("FourLeavesMultiAction", *tpg, "./src/");
    tpgGen->generateTPGGraph();
    // call the destructor to close the file
    tpgGen.reset();
    cmdCompile += "FourLeavesMultiAction";
    ASSERT_EQ(system(cmdCompile.c_str()), 0)
        << "Error while compiling the test TwoLeaves MultiAction.";
    cmdExec += "FourLeavesMultiAction" + executableExtension;

    ASSERT_EQ(system((cmdExec + path +
                      "/FourLeavesMultiAction/DataFourLeavesMultiAction.csv")
                         .c_str()),
              0)
        << "Error wrong action returned in test FourLeaves.";
};

#endif // CODE_GENERATION
