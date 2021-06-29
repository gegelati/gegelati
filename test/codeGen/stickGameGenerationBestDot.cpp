#include <gtest/gtest.h>
#include <stddef.h>

#include "environment.h"
#include "instructions/set.h"
#include "code_gen/LambdaPrintableInstruction.h"
#include "tpg/tpgAction.h"
#include "tpg/tpgTeam.h"
#include "tpg/tpgVertex.h"
#include "tpg/tpgGraph.h"
#include "file/tpgGraphDotImporter.h"
#include "code_gen/TpgGenerationEngine.h"

class StickGameGenerationBestDotTest : public ::testing::Test{
  protected:
    const size_t s1{8};
    Instructions::Set set;
    Environment* e;
    Environment* eTicTacToe;
    Environment* ePendulum;
    std::vector<std::reference_wrapper<const Data::DataHandler>> data;
    Data::PrimitiveTypeArray<double> currentState{s1};

    virtual void SetUp(){


        auto add = [](double a, double b)->double{return a+b;};
        auto sub = [](double a, double b)->double{return a-b;};
        set.add(*(new Instructions::LambdaPrintableInstruction<double, double>("$0 = $1 + $2;", add)));
        set.add(*(new Instructions::LambdaPrintableInstruction<double, double>("$0 = $1 - $2;",sub)));

        e = new Environment(set, data, 8);

    }

    virtual void TearDown(){

        delete e;
    }
};

TEST_F(TPGGenerationEngineTest, OneLeafNoInstruction){
    TPG::TPGGraph tpg(*e);

    const TPG::TPGVertex* leaf = (&tpg.addNewAction(1));
    const TPG::TPGVertex* root = (&tpg.addNewTeam());

    std::cout << "templated type : " << e->getDataSources().at(0).get().getTemplateType() << std::endl;
    const std::shared_ptr<Program::Program> progA(new Program::Program(*e));
    std::cout << "size : " << tpg.getEnvironment().getNbDataSources() << std::endl;
    std::cout << typeid(tpg.getEnvironment().getDataSources().at(0)).name() << std::endl;

    tpg.addNewEdge(*root, *leaf, progA);

    ASSERT_EQ(tpg.getNbRootVertices(), 1)
        << "number of root is not 1 in OneLeafNoInstruction";

    ASSERT_EQ(tpg.getNbVertices(),2)
        << "bad number of vertices in OneLeafNoInstruction";

    ASSERT_EQ(tpg.getEdges().size(), 1)
        << "bad number of edges in OneLeafNoInstruction";

    CodeGen::TPGGenerationEngine tpgGen("OneLeafNoInstruction", tpg);
    tpgGen.generateTPGGraph();


}

TEST_F(TPGGenerationEngineTest, OneLeaf){
    TPG::TPGGraph tpg(*e);

    const TPG::TPGVertex* leaf = (&tpg.addNewAction(1));
    const TPG::TPGVertex* root = (&tpg.addNewTeam());

    const std::shared_ptr<Program::Program> prog1(new Program::Program(*e));
    Program::Line& prog1L1 = prog1->addNewLine();
    //reg[0] = in1[0] - reg[1]
    prog1L1.setDestinationIndex(0);
    prog1L1.setInstructionIndex(1);
    prog1L1.setOperand(0,1,0);
    prog1L1.setOperand(1,0,1);

    prog1->identifyIntrons();

    //Version avec 1 root, 1 action, 1 programme
    tpg.addNewEdge(*root, *leaf, prog1);

    ASSERT_EQ(tpg.getNbRootVertices(), 1)
        << "number of root is not 1 in OneLeaf";

    ASSERT_EQ(tpg.getNbVertices(),2)
        << "bad number of vertices in OneLeaf";

    ASSERT_EQ(tpg.getEdges().size(), 1)
        << "bad number of edges in OneLeaf";

    CodeGen::TPGGenerationEngine tpgGen("OneLeaf", tpg);
    tpgGen.generateTPGGraph();
}

TEST_F(TPGGenerationEngineTest, TwoLeaves){
    TPG::TPGGraph tpg(*e);

    const TPG::TPGVertex* leaf = (&tpg.addNewAction(1));
    const TPG::TPGVertex* leaf2 = (&tpg.addNewAction(2));
    const TPG::TPGVertex* root = (&tpg.addNewTeam());

    const std::shared_ptr<Program::Program> prog1(new Program::Program(*e));
    Program::Line& prog1L1 = prog1->addNewLine();
    //reg[0] = in1[0] + in1[1];
    prog1L1.setDestinationIndex(0);
    prog1L1.setInstructionIndex(0);
    prog1L1.setOperand(0,1,0);
    prog1L1.setOperand(1,1,1);

    const std::shared_ptr<Program::Program> prog2(new Program::Program(*e));
    Program::Line& prog2L1 = prog2->addNewLine();
    //reg[0] = in1[0] + in1[2];
    prog2L1.setDestinationIndex(0);
    prog2L1.setInstructionIndex(0);
    prog2L1.setOperand(0,1,0);
    prog2L1.setOperand(1,1,2);

    //Version : choix de 2 action à partir de la root
    tpg.addNewEdge(*root, *leaf, prog1);
    tpg.addNewEdge(*root, *leaf2, prog2);

    ASSERT_EQ(tpg.getNbRootVertices(), 1)
        << "number of root is not 1 in TwoLeaves";

    ASSERT_EQ(tpg.getNbVertices(),3)
        << "bad number of vertices in TwoLeaves";

    ASSERT_EQ(tpg.getEdges().size(), 2)
        << "bad number of edges in TwoLeaves";

    CodeGen::TPGGenerationEngine tpgGen("TwoLeaves", tpg);
    tpgGen.generateTPGGraph();
}

TEST_F(TPGGenerationEngineTest, OneTeamOneLeaf){
    TPG::TPGGraph tpg(*e);

    const TPG::TPGVertex* root = (&tpg.addNewTeam());
    const TPG::TPGVertex* T1 = (&tpg.addNewTeam());
    const TPG::TPGVertex* leaf = (&tpg.addNewAction(1));

    const std::shared_ptr<Program::Program> prog1(new Program::Program(*e));
    Program::Line& prog1L1 = prog1->addNewLine();
    //reg[0] = in1[0] + reg[1];
    prog1L1.setDestinationIndex(0);
    prog1L1.setInstructionIndex(0);
    prog1L1.setOperand(0,1,0);
    prog1L1.setOperand(1,0,1);

    const std::shared_ptr<Program::Program> prog2(new Program::Program(*e));
    Program::Line& prog2L1 = prog2->addNewLine();
    //reg[0] = in1[0] - in1[1];
    prog2L1.setDestinationIndex(0);
    prog2L1.setInstructionIndex(1);
    prog2L1.setOperand(0,1,0);
    prog2L1.setOperand(1,1,1);

    //Version avec team intermédiaire
    TPG::TPGEdge edge1 = tpg.addNewEdge(*root, *T1, prog1);
    TPG::TPGEdge edge2 = tpg.addNewEdge(*T1, *leaf, prog2);

    ASSERT_EQ(tpg.getNbRootVertices(), 1)
        << "number of root is not 1 in OneTeamOneLeaf";

    ASSERT_EQ(tpg.getNbVertices(),3)
        << "bad number of vertices in OneTeamOneLeaf";

    ASSERT_EQ(tpg.getEdges().size(), 2)
        << "bad number of edges in OneTeamOneLeaf";

    CodeGen::TPGGenerationEngine tpgGen("OneTeamOneLeaf", tpg);
    tpgGen.generateTPGGraph();

}

TEST_F(TPGGenerationEngineTest, OneTeamTwoLeaves){
    TPG::TPGGraph tpg(*e);

    const TPG::TPGVertex* root = (&tpg.addNewTeam());
    const TPG::TPGVertex* T1 = (&tpg.addNewTeam());
    const TPG::TPGVertex* leaf = (&tpg.addNewAction(1));
    const TPG::TPGVertex* leaf2 = (&tpg.addNewAction(2));

    const std::shared_ptr<Program::Program> prog1(new Program::Program(*e));
    Program::Line& prog1L1 = prog1->addNewLine();
    //reg[0] = in1[0] - reg[1];
    prog1L1.setDestinationIndex(0);
    prog1L1.setInstructionIndex(0);
    prog1L1.setOperand(0,1,0);
    prog1L1.setOperand(1,0,1);

    const std::shared_ptr<Program::Program> prog2(new Program::Program(*e));
    Program::Line& prog2L1 = prog2->addNewLine();
    //reg[0] = in1[0] - in1[1];
    prog2L1.setDestinationIndex(0);
    prog2L1.setInstructionIndex(1);
    prog2L1.setOperand(0,1,0);
    prog2L1.setOperand(1,1,1);

    const std::shared_ptr<Program::Program> prog3(new Program::Program(*e));
    Program::Line& prog3L1 = prog3->addNewLine();
    //reg[0] = in1[0] + in1[1];
    prog3L1.setDestinationIndex(0);
    prog3L1.setInstructionIndex(0);
    prog3L1.setOperand(0,1,0);
    prog3L1.setOperand(1,1,1);

    //Version avec team intermédiaire
    tpg.addNewEdge(*root, *T1, prog1);
    tpg.addNewEdge(*T1, *leaf, prog2);
    tpg.addNewEdge(*T1, *leaf2, prog3);
    ASSERT_EQ(tpg.getNbRootVertices(), 1)
        << "number of root is not 1 in OneTeamTwoLeaves";

    ASSERT_EQ(tpg.getNbVertices(),4)
        << "bad number of vertices in OneTeamTwoLeaves";

    ASSERT_EQ(tpg.getEdges().size(), 2)
        << "bad number of edges in OneTeamTwoLeaves";

    CodeGen::TPGGenerationEngine tpgGen("OneTeamTwoLeaves", tpg);
    tpgGen.generateTPGGraph();

}

TEST_F(TPGGenerationEngineTest, TwoTeamsOneCycle){
    TPG::TPGGraph tpg(*e);

    const TPG::TPGVertex* root = (&tpg.addNewTeam());
    const TPG::TPGVertex* T1 = (&tpg.addNewTeam());
    const TPG::TPGVertex* T2 = (&tpg.addNewTeam());
    const TPG::TPGVertex* leaf = (&tpg.addNewAction(1));
    const TPG::TPGVertex* leaf2 = (&tpg.addNewAction(2));

    const std::shared_ptr<Program::Program> prog1(new Program::Program(*e));
    Program::Line& prog1L1 = prog1->addNewLine();
    //reg[0] = in1[0] + in1[1];
    prog1L1.setDestinationIndex(0);
    prog1L1.setInstructionIndex(0);
    prog1L1.setOperand(0,1,0);
    prog1L1.setOperand(1,1,1);

    const std::shared_ptr<Program::Program> prog2(new Program::Program(*e));
    Program::Line& prog2L1 = prog2->addNewLine();
    //reg[0] = in1[1] + in1[2];
    prog2L1.setDestinationIndex(0);
    prog2L1.setInstructionIndex(0);
    prog2L1.setOperand(0,1,1);
    prog2L1.setOperand(1,1,2);

    const std::shared_ptr<Program::Program> prog3(new Program::Program(*e));
    Program::Line& prog3L1 = prog3->addNewLine();
    //reg[0] = in1[1] - in1[2];
    prog3L1.setDestinationIndex(0);
    prog3L1.setInstructionIndex(1);
    prog3L1.setOperand(0,1,1);
    prog3L1.setOperand(1,1,2);

    const std::shared_ptr<Program::Program> prog4(new Program::Program(*e));
    Program::Line& prog4L1 = prog4->addNewLine();
    //reg[0] = in1[1] - in1[2];
    prog4L1.setDestinationIndex(0);
    prog4L1.setInstructionIndex(1);
    prog4L1.setOperand(0,1,1);
    prog4L1.setOperand(1,1,2);

    const std::shared_ptr<Program::Program> prog5(new Program::Program(*e));
    Program::Line& prog5L1 = prog5->addNewLine();
    //reg[0] = in1[1] + in1[2];
    prog5L1.setDestinationIndex(0);
    prog5L1.setInstructionIndex(0);
    prog5L1.setOperand(0,1,1);
    prog5L1.setOperand(1,1,2);

    //Version avec team intermédiaire
    tpg.addNewEdge(*root, *T1, prog1);
    tpg.addNewEdge(*T1, *leaf, prog2);
    tpg.addNewEdge(*T1, *T2, prog3);
    tpg.addNewEdge(*T2, *leaf2, prog4);
    tpg.addNewEdge(*T2, *T1, prog5);

    ASSERT_EQ(tpg.getNbRootVertices(), 1)
        << "number of root is not 1 in TwoTeamsOneCycle";

    ASSERT_EQ(tpg.getNbVertices(),5)
        << "bad number of vertices in TwoTeamsOneCycle";

    ASSERT_EQ(tpg.getEdges().size(), 2)
        << "bad number of edges in TwoTeamsOneCycle";

    CodeGen::TPGGenerationEngine tpgGen("TwoTeamsOneCycle", tpg);
    tpgGen.generateTPGGraph();

}

static void setProgLine(const std::shared_ptr<Program::Program> prog, int operand){
    Program::Line &line = prog->addNewLine();
    //reg[0] = in1[operand] + in1[8] (= 0)
    line.setDestinationIndex(0);
    line.setInstructionIndex(0);
    line.setOperand(0, 1, operand);
    line.setOperand(1, 1, 8);
}

TEST_F(TPGGenerationEngineTest, ThreeTeamsOneCycleThreeLeaf){
    TPG::TPGGraph tpg(*e);

    const TPG::TPGVertex *A1 = (&tpg.addNewAction(1));
    const TPG::TPGVertex *A2 = (&tpg.addNewAction(2));
    const TPG::TPGVertex *A0 = (&tpg.addNewAction(0));
    const TPG::TPGVertex *T1 = (&tpg.addNewTeam());
    const TPG::TPGVertex *T2 = (&tpg.addNewTeam());
    const TPG::TPGVertex *T3 = (&tpg.addNewTeam());

    const std::shared_ptr<Program::Program> prog1(new Program::Program(*e));
    const std::shared_ptr<Program::Program> prog2(new Program::Program(*e));
    const std::shared_ptr<Program::Program> prog3(new Program::Program(*e));
    const std::shared_ptr<Program::Program> prog4(new Program::Program(*e));
    const std::shared_ptr<Program::Program> prog5(new Program::Program(*e));
    const std::shared_ptr<Program::Program> prog6(new Program::Program(*e));
    const std::shared_ptr<Program::Program> prog7(new Program::Program(*e));
    const std::shared_ptr<Program::Program> prog8(new Program::Program(*e));

    setProgLine(prog1, 0);
    //reg[0] = in1[0] + in1[8] (= 0)
    setProgLine(prog2, 1);
    //reg[0] = in1[1] + in1[8] (= 0)
    setProgLine(prog3, 2);
    //reg[0] = in1[2] + in1[8] (= 0)
    setProgLine(prog4, 3);
    //reg[0] = in1[3] + in1[8] (= 0)
    setProgLine(prog5, 4);
    //reg[0] = in1[4] + in1[8] (= 0)
    setProgLine(prog6, 5);
    //reg[0] = in1[5] + in1[8] (= 0)
    setProgLine(prog7, 6);
    //reg[0] = in1[6] + in1[8] (= 0)
    setProgLine(prog8, 7);
    //reg[0] = in1[7] + in1[8] (= 0)


//Version avec team intermédiaire
    tpg.addNewEdge(*T1, *T2, prog1);
    tpg.addNewEdge(*T1, *A1, prog2);
    tpg.addNewEdge(*T1, *T3, prog3);

    tpg.addNewEdge(*T2, *A0, prog4);
    tpg.addNewEdge(*T2, *T3, prog2);

    tpg.addNewEdge(*T3, *A2, prog5);
    tpg.addNewEdge(*T3, *T2, prog6);

    ASSERT_EQ(tpg.getNbRootVertices(), 1)
        << "number of root is not 1 in ThreeTeamsOneCycleThreeLeaf";

    ASSERT_EQ(tpg.getNbVertices(),6)
        << "bad number of vertices in ThreeTeamsOneCycleThreeLeaf";

    ASSERT_EQ(tpg.getEdges().size(), 3)
        << "bad number of edges in ThreeTeamsOneCycleThreeLeaf";

    CodeGen::TPGGenerationEngine tpgGen("ThreeTeamsOneCycleThreeLeaf", tpg);
    tpgGen.generateTPGGraph();

}

TEST_F(TPGGenerationEngineTest, TicTacToeBestTPG){
    //TESTS_DAT_PATH;

}

TEST_F(TPGGenerationEngineTest, PendulumBestTPG){
//    TESTS_DAT_PATH;

}



