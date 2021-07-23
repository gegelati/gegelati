#ifdef CODE_GENERATION
#include <gtest/gtest.h>
#include <stddef.h>

#include "code_gen/TpgGenerationEngine.h"
#include "environment.h"
#include "file/tpgGraphDotImporter.h"
#include "instructions/lambdaInstruction.h"
#include "instructions/set.h"
#include "tpg/tpgAction.h"
#include "tpg/tpgGraph.h"
#include "tpg/tpgTeam.h"
#include "tpg/tpgVertex.h"

class TPGGenerationEngineTest : public ::testing::Test
{
  protected:
    std::string path;
    std::string cmdCompile;
    std::string cmdExec;
    const size_t s1{8};
    Instructions::Set set;
    Environment* e;
    std::vector<std::reference_wrapper<const Data::DataHandler>> data;
    Data::PrimitiveTypeArray<double> currentState{s1};
    CodeGen::TPGGenerationEngine* tpgGen;
    TPG::TPGGraph* tpg;
    std::ifstream dataCSV;
    std::string dataLine;
    std::string cmdBuildSystem{"cd " BIN_DIR_PATH "/buildCodeGen; cmake -DDIR=" BIN_DIR_PATH " " TESTS_DAT_PATH "codeGen; cd -"};

    virtual void SetUp()
    {
        path = TESTS_DAT_PATH;
        path += "codeGen";

        data.emplace_back(currentState);

        auto add = [](double a, double b) -> double { return a + b; };
        auto sub = [](double a, double b) -> double { return a - b; };
        set.add(*(new Instructions::LambdaInstruction<double, double>(
            "$0 = $1 + $2;", add)));
        set.add(*(new Instructions::LambdaInstruction<double, double>(
            "$0 = $1 - $2;", sub)));

        e = new Environment(set, data, 8);
        tpg = new TPG::TPGGraph(*e);
#ifdef _MSC_VER
    cmdCompile = "dir=" BIN_DIR_PATH " nmake -C " + path + " ";
#elif __GNUC__
    cmdCompile = "dir=" BIN_DIR_PATH  " make -C " + path + " ";
#endif
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
                        new CodeGen::TPGGenerationEngine("constructor", *tpg))
        << "Failed to construct a TPGGenerationEngine with a filename and a "
           "TPG";

    ASSERT_NO_THROW(delete tpgGen) << "Destruction failed.";

    ASSERT_NO_THROW(tpgGen = new CodeGen::TPGGenerationEngine(
                        "constructorWithPath", *tpg, BIN_DIR_PATH "/src/"))
        << "Failed to construct a TPGGenerationEngine with a filename and a "
           "TPG and a path";

    ASSERT_NO_THROW(delete tpgGen) << "Destruction failed.";

    ASSERT_NO_THROW(
        tpgGen = new CodeGen::TPGGenerationEngine(
            "constructorWithStackSize", *tpg, BIN_DIR_PATH "/src/", 15))
        << "Failed to construct a TPGGenerationEngine with a filename and a "
           "TPG, a path and the size of the call stack";

    ASSERT_NO_THROW(delete tpgGen) << "Destruction failed.";

    ASSERT_THROW(
        tpgGen = new CodeGen::TPGGenerationEngine(
            "constructorErrorStackSize", *tpg, BIN_DIR_PATH "/src/", 0),
        std::runtime_error)
        << "Should fail, try to construct a TPGGenerationEngine with the size "
           "of the call stack equal to 0";

    // ASSERT_NO_THROW(delete tpgGen) << "Destruction failed.";
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

    tpgGen = new CodeGen::TPGGenerationEngine("OneLeafNoInstruction", *tpg,
                                              BIN_DIR_PATH "/src/");
    tpgGen->generateTPGGraph();
    // call the destructor to close the file
    delete tpgGen;

    cmdCompile += "OneLeafNoInstruction";
    ASSERT_EQ(system(cmdCompile.c_str()), 0)
        << "Compilation failed in OneLeafNoInstruction";
}

TEST_F(TPGGenerationEngineTest, OneLeaf)
{
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

    // Version avec 1 root, 1 action, 1 programme
    tpg->addNewEdge(*root, *leaf, prog1);

    ASSERT_EQ(tpg->getNbRootVertices(), 1)
        << "number of root is not 1 in OneLeaf";

    ASSERT_EQ(tpg->getNbVertices(), 2) << "bad number of vertices in OneLeaf";

    ASSERT_EQ(tpg->getEdges().size(), 1) << "bad number of edges in OneLeaf";

    tpgGen =
        new CodeGen::TPGGenerationEngine("OneLeaf", *tpg, BIN_DIR_PATH "/src/");
    tpgGen->generateTPGGraph();
    // call the destructor to close the file
    delete tpgGen;
    cmdCompile += "OneLeaf";
    ASSERT_EQ(system(cmdCompile.c_str()), 0);
    cmdExec = BIN_DIR_PATH "/bin/OneLeaf 1 4.5";
    ASSERT_EQ(system(cmdExec.c_str()), 0)
        << "Error wrong action returned in test OneLeaf";
}

TEST_F(TPGGenerationEngineTest, TwoLeaves)
{
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

    // Version : choix de 2 action à partir de la root
    tpg->addNewEdge(*root, *leaf, prog1);
    tpg->addNewEdge(*root, *leaf2, prog2);

    ASSERT_EQ(tpg->getNbRootVertices(), 1)
        << "number of root is not 1 in TwoLeaves";

    ASSERT_EQ(tpg->getNbVertices(), 3) << "bad number of vertices in TwoLeaves";

    ASSERT_EQ(tpg->getEdges().size(), 2) << "bad number of edges in TwoLeaves";

    tpgGen = new CodeGen::TPGGenerationEngine("TwoLeaves", *tpg,
                                              BIN_DIR_PATH "/src/");
    tpgGen->generateTPGGraph();
    // call the destructor to close the file
    delete tpgGen;
    cmdCompile += "TwoLeaves";
    ASSERT_EQ(system(cmdCompile.c_str()), 0)
        << "Error wrong action returned in test TwoLeaves";
    dataCSV.open(path + "/DataTwoLeaves.csv");

    while (std::getline(dataCSV, dataLine)) {
        cmdExec = BIN_DIR_PATH "/bin/TwoLeaves " + dataLine;
        ASSERT_EQ(system(cmdExec.c_str()), 0)
            << "Error wrong action returned in test TwoLeaves with input value "
               ": " +
                   dataLine;
    }
}

TEST_F(TPGGenerationEngineTest, ThreeLeaves)
{
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

    // Version : choix de 2 action à partir de la root
    tpg->addNewEdge(*root, *leaf, prog1);
    tpg->addNewEdge(*root, *leaf2, prog2);
    tpg->addNewEdge(*root, *leaf3, prog3);

    ASSERT_EQ(tpg->getNbRootVertices(), 1)
        << "number of root is not 1 in ThreeLeaves";

    ASSERT_EQ(tpg->getNbVertices(), 4)
        << "bad number of vertices in ThreeLeaves";

    ASSERT_EQ(tpg->getEdges().size(), 3)
        << "bad number of edges in ThreeLeaves";

    tpgGen = new CodeGen::TPGGenerationEngine("ThreeLeaves", *tpg,
                                              BIN_DIR_PATH "/src/");
    tpgGen->generateTPGGraph();
    // call the destructor to close the file
    delete tpgGen;
    cmdCompile += "ThreeLeaves";
    ASSERT_EQ(system(cmdCompile.c_str()), 0)
        << "Error wrong action returned in test ThreeLeaves";

    dataCSV.open(path + "/DataThreeLeaves.csv");

    while (std::getline(dataCSV, dataLine)) {
        cmdExec = BIN_DIR_PATH "/bin/ThreeLeaves " + dataLine;
        ASSERT_EQ(system(cmdExec.c_str()), 0)
            << "Error wrong action returned in test ThreeLeaves with input "
               "value "
               ": " +
                   dataLine;
    }
}

TEST_F(TPGGenerationEngineTest, OneTeamOneLeaf)
{
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

    // Version avec team intermédiaire
    TPG::TPGEdge edge1 = tpg->addNewEdge(*root, *T1, prog1);
    TPG::TPGEdge edge2 = tpg->addNewEdge(*T1, *leaf, prog2);

    ASSERT_EQ(tpg->getNbRootVertices(), 1)
        << "number of root is not 1 in OneTeamOneLeaf";

    ASSERT_EQ(tpg->getNbVertices(), 3)
        << "bad number of vertices in OneTeamOneLeaf";

    ASSERT_EQ(tpg->getEdges().size(), 2)
        << "bad number of edges in OneTeamOneLeaf";

    tpgGen = new CodeGen::TPGGenerationEngine("OneTeamOneLeaf", *tpg,
                                              BIN_DIR_PATH "/src/");
    tpgGen->generateTPGGraph();
    // call the destructor to close the file
    delete tpgGen;
    cmdCompile += "OneTeamOneLeaf";
    ASSERT_EQ(system(cmdCompile.c_str()), 0)
        << "Error wrong action returned in test OneTeamOneLeaf";
    cmdExec = BIN_DIR_PATH "/bin/OneTeamOneLeaf 1 4.5 6.8";
    ASSERT_EQ(system(cmdExec.c_str()), 0)
        << "Error wrong action returned in test OneTeamOneLeaf";
}

TEST_F(TPGGenerationEngineTest, OneTeamTwoLeaves)
{
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

    // Version avec team intermédiaire
    tpg->addNewEdge(*root, *T1, prog1);
    tpg->addNewEdge(*T1, *leaf, prog2);
    tpg->addNewEdge(*T1, *leaf2, prog3);
    ASSERT_EQ(tpg->getNbRootVertices(), 1)
        << "number of root is not 1 in OneTeamTwoLeaves";

    ASSERT_EQ(tpg->getNbVertices(), 4)
        << "bad number of vertices in OneTeamTwoLeaves";

    ASSERT_EQ(tpg->getEdges().size(), 3)
        << "bad number of edges in OneTeamTwoLeaves";

    tpgGen = new CodeGen::TPGGenerationEngine("OneTeamTwoLeaves", *tpg,
                                              BIN_DIR_PATH "/src/");
    tpgGen->generateTPGGraph();
    // call the destructor to close the file
    delete tpgGen;
    cmdCompile += "OneTeamTwoLeaves";
    ASSERT_EQ(system(cmdCompile.c_str()), 0)
        << "Error wrong action returned in test OneTeamTwoLeaves";

    dataCSV.open(path + "/DataOneTeamTwoLeaves.csv");

    while (std::getline(dataCSV, dataLine)) {
        cmdExec = BIN_DIR_PATH "/bin/OneTeamTwoLeaves " + dataLine;
        ASSERT_EQ(system(cmdExec.c_str()), 0)
            << "Error wrong action returned in test OneTeamTwoLeaves with "
               "input value "
               ": " +
                   dataLine;
    }
}

TEST_F(TPGGenerationEngineTest, TwoTeamsOneCycle)
{
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

    const std::shared_ptr<Program::Program> prog5(new Program::Program(*e));
    Program::Line& prog5L1 = prog5->addNewLine();
    // reg[0] = in1[1] + in1[5];
    prog5L1.setDestinationIndex(0);
    prog5L1.setInstructionIndex(0);
    prog5L1.setOperand(0, 1, 1);
    prog5L1.setOperand(1, 1, 5);

    // Version avec team intermédiaire
    tpg->addNewEdge(*root, *T1, prog1);
    tpg->addNewEdge(*T1, *leaf, prog2);
    tpg->addNewEdge(*T1, *T2, prog3);
    tpg->addNewEdge(*T2, *leaf2, prog4);
    tpg->addNewEdge(*T2, *T1, prog5);

    ASSERT_EQ(tpg->getNbRootVertices(), 1)
        << "number of root is not 1 in TwoTeamsOneCycle";

    ASSERT_EQ(tpg->getNbVertices(), 5)
        << "bad number of vertices in TwoTeamsOneCycle";

    ASSERT_EQ(tpg->getEdges().size(), 5)
        << "bad number of edges in TwoTeamsOneCycle";

    tpgGen = new CodeGen::TPGGenerationEngine("TwoTeamsOneCycle", *tpg,
                                              BIN_DIR_PATH "/src/");
    tpgGen->generateTPGGraph();
    // call the destructor to close the file
    delete tpgGen;

    cmdCompile += "TwoTeamsOneCycle";
    ASSERT_EQ(system(cmdCompile.c_str()), 0)
        << "Error wrong action returned in test TwoTeamsOneCycle";

    dataCSV.open(path + "/DataTwoTeamsOneCycle.csv");

    while (std::getline(dataCSV, dataLine)) {
        cmdExec = BIN_DIR_PATH "/bin/TwoTeamsOneCycle " + dataLine;
        ASSERT_EQ(system(cmdExec.c_str()), 0)
            << "Error wrong action returned in test TwoTeamsOneCycle with "
               "input value "
               ": " +
                   dataLine;
    }
}

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

TEST_F(TPGGenerationEngineTest, ThreeTeamsOneCycleThreeLeaves)
{
    // P1 > P2
    // P1 > P3
    // P6 > P5
    // P7 > P4

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
    const std::shared_ptr<Program::Program> prog7(new Program::Program(*e));

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
    setProgLine(prog7, 6);
    // reg[0] = in1[6] + reg[1] (reg[1] = 0)

    // Version avec team intermédiaire
    tpg->addNewEdge(*T1, *T2, prog1);
    tpg->addNewEdge(*T1, *A1, prog2);
    tpg->addNewEdge(*T1, *T3, prog3);

    tpg->addNewEdge(*T2, *A0, prog4);
    tpg->addNewEdge(*T2, *T3, prog5);

    tpg->addNewEdge(*T3, *A2, prog6);
    tpg->addNewEdge(*T3, *T2, prog7);

    ASSERT_EQ(tpg->getNbRootVertices(), 1)
        << "number of root is not 1 in ThreeTeamsOneCycleThreeLeaves";

    ASSERT_EQ(tpg->getNbVertices(), 6)
        << "bad number of vertices in ThreeTeamsOneCycleThreeLeaves";

    ASSERT_EQ(tpg->getEdges().size(), 7)
        << "bad number of edges in ThreeTeamsOneCycleThreeLeaves";

    tpgGen = new CodeGen::TPGGenerationEngine("ThreeTeamsOneCycleThreeLeaves",
                                              *tpg, BIN_DIR_PATH "/src/");
    tpgGen->generateTPGGraph();
    // call the destructor to close the file
    delete tpgGen;
    cmdCompile += "ThreeTeamsOneCycleThreeLeaves";
    ASSERT_EQ(system(cmdCompile.c_str()), 0)
        << "Error wrong action returned in test ThreeTeamsOneCycleThreeLeaves";

    dataCSV.open(path + "/DataThreeTeamsOneCycleThreeLeaves.csv");

    while (std::getline(dataCSV, dataLine)) {
        cmdExec = BIN_DIR_PATH "/bin/ThreeTeamsOneCycleThreeLeaves " + dataLine;
        ASSERT_EQ(system(cmdExec.c_str()), 0)
            << "Error wrong action returned in test "
               "ThreeTeamsOneCycleThreeLeaves with input value "
               ": " +
                   dataLine;
    }
}
#endif // CODE_GENERATION