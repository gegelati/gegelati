#include <gtest/gtest.h>
#include <vector>

#include "instructions/set.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/lambdaInstruction.h"
#include "instructions/multByConstant.h"
#include "tpg/tpgGraph.h"
#include "environment.h"
#include "tpg/instrumented/tpgInstrumentedFactory.h"
#include "tpg/instrumented/tpgExecutionEngineInstrumented.h"
#include "data/dataHandler.h"
#include "data/primitiveTypeArray.h"

#include "tpg/instrumented/executionStats.h"



class ExecutionStatsTest : public ::testing::Test
{
  protected:

    Instructions::Set set;
    Data::PrimitiveTypeArray<double>* data;
    std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
    Environment* e = nullptr;
    std::vector<std::shared_ptr<Program::Program>> progPointers;

    TPG::TPGGraph* tpg;
    std::vector<const TPG::TPGEdge*> edges;

    TPG::TPGExecutionEngineInstrumented* execEngine;

    std::vector<std::vector<const TPG::TPGVertex*>> executionTraces;


    virtual void SetUp() override{

        // Based on policyStats tests

        // Instructions
        std::function<double(double, const double[3])> mac =
            [](double a, const double b[3]) {
                return a * (b[0] + b[1] + b[2]);
            };
        std::function<double(double, double)> minus =
            [](double a, double b) -> double { return a - b; };

        set.add(*(new Instructions::AddPrimitiveType<double>()));
        set.add(*(
            new Instructions::LambdaInstruction<double, const double[3]>(mac)));
        set.add(*(new Instructions::LambdaInstruction<double, double>(minus)));
        set.add(*(new Instructions::MultByConstant<double>()));

        // Data handler
        // Setup environment
        data = new Data::PrimitiveTypeArray<double>((unsigned int)25);
        vect.emplace_back(*data);

        // Environment
        e = new Environment(set, vect, 8, 5);

        // Setup execution engine
        execEngine = new TPG::TPGExecutionEngineInstrumented(*e);

        // Create 9 programs
        for (int i = 0; i < 9; i++) {
            progPointers.push_back(
                std::make_shared<Program::Program>(*e));
        }

        // Create a TPG
        // (T= Team, A= Action)
        // T0 will be considered the main team root
        // T3 will also be considered a team root, but we will never start an
        // inference from it, so his nbVisited attribute will stay at 0
        // and the number of inferences will correctly be deduced by the
        // executionStats class.
        //
        //        .------.
        //        v      |
        // T0---->T1---->T2<----T3
        // |     /| \    |       |
        // v    / v  \   v       v
        // A0<-'  A1  `->A2     A3
        //
        // With four action and four teams
        // All Edges have a unique Program, except T1->A0 and T0->A0 which
        // share the same program: progPointers.at(0)

        // The TPG is given a TPGInstrumentedFactory to enable instrumentation
        tpg = new TPG::TPGGraph(*e, std::make_unique<TPG::TPGInstrumentedFactory>());
        for (int i = 0; i < 4; i++) {
            tpg->addNewTeam();
        }
        for (int i = 0; i < 4; i++) {
            // Each action is linked to a team (and vice-versa)
            tpg->addNewAction(i);
            edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(i),
                                             *tpg->getVertices().back(),
                                             progPointers.at(i)));
        }

        // Add new Edges between teams
        edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(0),
                                         *tpg->getVertices().at(1),
                                         progPointers.at(4)));
        edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(1),
                                         *tpg->getVertices().at(2),
                                         progPointers.at(5)));
        edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(3),
                                         *tpg->getVertices().at(2),
                                         progPointers.at(6)));

        // Add a cyclic edge
        edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(2),
                                         *tpg->getVertices().at(1),
                                         progPointers.at(7)));

        // Add new outgoing edge to one team
        edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(1),
                                         *tpg->getVertices().at(4),
                                         progPointers.at(0)));
        edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(1),
                                         *tpg->getVertices().at(6),
                                         progPointers.at(8)));

        // Check the characteristics
        ASSERT_EQ(tpg->getNbVertices(), 8);
        ASSERT_EQ(tpg->getEdges().size(), 10);
        ASSERT_EQ(tpg->getRootVertices().size(), 2);

        // Add instructions to some programs

        // Program 0 (referenced by two edges)
        Program::Line* l = &progPointers.at(0).get()->addNewLine();
        // Intron
        l->setInstructionIndex(3); // MultByConst
        l->setDestinationIndex(4); // Register[4]
        l->setOperand(0, 0, 0);    // Array[0]
        l->setOperand(1, 1, 0);    // Constant[0]

        l = &progPointers.at(0).get()->addNewLine();
        l->setInstructionIndex(2); // Minus
        l->setDestinationIndex(1); // Register[1]
        l->setOperand(0, 2, 5);    // Register[5]
        l->setOperand(1, 0, 2);   // Array[2]

        l = &progPointers.at(0).get()->addNewLine();
        l->setInstructionIndex(2); // Minus
        l->setDestinationIndex(0); // Register[0]
        l->setOperand(0, 2, 2);    // Array[2]
        l->setOperand(1, 0, 1);    // Register[1]

        progPointers.at(0).get()->identifyIntrons();

        // Program 1 (referenced by one edge)
        l = &progPointers.at(1).get()->addNewLine();
        l->setInstructionIndex(2); // Minus
        l->setDestinationIndex(0); // Register[0]
        l->setOperand(0, 2, 10);   // Array[10]
        l->setOperand(1, 2, 12);   // Array[12]

        progPointers.at(1).get()->identifyIntrons();


        // Program 8 (T1 -> A2)
        l = &progPointers.at(8).get()->addNewLine();
        l->setInstructionIndex(1);  // mac
        l->setDestinationIndex(0);
        l->setOperand(0, 2, 6); // Array[6]
        l->setOperand(1, 2, 7); // Array[7..9]

        // Program 2 (T2 -> A2)
        l = &progPointers.at(2).get()->addNewLine();
        l->setInstructionIndex(0);  // Add
        l->setDestinationIndex(0);
        l->setOperand(0, 2, 3); // Array[3]
        l->setOperand(1, 2, 4); // Array[4]

        // TPG execution

        // Four graph executions to do :
        //  - T0 -> T1 -> A2
        //  - T0 -> T1 -> A1
        //  - T0 -> T1 -> T2 -> A2
        //  - T0 -> T1 -> T2 -> A2 with possible cycle


        //  - T0 -> T1 -> A2
        data->setDataAt(typeid(double), 2, -6);
        data->setDataAt(typeid(double), 3, 10);
        data->setDataAt(typeid(double), 6, 2);
        data->setDataAt(typeid(double), 7, 2);
        data->setDataAt(typeid(double), 8, 2);
        data->setDataAt(typeid(double), 9, 2);
        // P0 = -12
        // P1 = 0
        // P2 = 10
        // P8 = 12
        ASSERT_NO_THROW(executionTraces.push_back(
            execEngine->executeFromRoot(*tpg->getVertices().at(0))
            ));

        //  - T0 -> T1 -> A1
        data->setDataAt(typeid(double), 10, 10);
        data->setDataAt(typeid(double), 12, -3);
        // P0 = -12
        // P1 = 13
        // P2 = 10
        // P8 = 12
        ASSERT_NO_THROW(executionTraces.push_back(
            execEngine->executeFromRoot(*tpg->getVertices().at(0))
            ));

        //  - T0 -> T1 -> T2 -> A2
        data->setDataAt(typeid(double), 12, 13);
        data->setDataAt(typeid(double), 6, -3);
        // P0 = -12
        // P1 = -3
        // P2 = 10
        // P8 = -18
        ASSERT_NO_THROW(executionTraces.push_back(
            execEngine->executeFromRoot(*tpg->getVertices().at(0))
            ));

        //  - T0 -> T1 -> T2 -> A2 with possible cycle
        data->setDataAt(typeid(double), 3, -10);
        // P0 = -12
        // P1 = -3
        // P2 = -10
        // P8 = -18
        ASSERT_NO_THROW(executionTraces.push_back(
            execEngine->executeFromRoot(*tpg->getVertices().at(0))
            ));

    }

    virtual void TearDown() override{

        delete tpg;
        delete execEngine;
        delete e;
        delete data;
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
        delete (&set.getInstruction(2));
        delete (&set.getInstruction(3));

    }

};

TEST_F(ExecutionStatsTest, AnalyzeExecution)
{
    TPG::ExecutionStats executionStats;
    ASSERT_NO_THROW(executionStats.analyzeExecution(tpg))
        << "Analysis of a valid tpg execution failed unexpectedly.";

    ASSERT_EQ(executionStats.getAvgEvaluatedTeams(), 10.0/4.0)
        << "Incorrect attribute value after analyzing execution.";
    ASSERT_EQ(executionStats.getAvgEvaluatedPrograms(), 26.0/4.0)
        << "Incorrect attribute value after analyzing execution.";
    ASSERT_EQ(executionStats.getAvgExecutedLines(), 34.0/4.0)
        << "Incorrect attribute value after analyzing execution.";
    // Add
    ASSERT_EQ(executionStats.getAvgNbExecutionPerInstruction().at(0), 2.0/4.0)
        << "Incorrect attribute value after analyzing execution.";
    // mac
    ASSERT_EQ(executionStats.getAvgNbExecutionPerInstruction().at(1), 4.0/4.0)
        << "Incorrect attribute value after analyzing execution.";
    // Minus
    ASSERT_EQ(executionStats.getAvgNbExecutionPerInstruction().at(2), 20.0/4.0)
        << "Incorrect attribute value after analyzing execution.";
    // MultByConst
    ASSERT_EQ(executionStats.getAvgNbExecutionPerInstruction().at(3), 8.0/4.0)
        << "Incorrect attribute value after analyzing execution.";

}

TEST_F(ExecutionStatsTest, AnalyzeNotInstrumented){

    TPG::TPGGraph notInstrumented(*e);
    notInstrumented.addNewTeam();

    TPG::ExecutionStats executionStats;
    ASSERT_THROW(executionStats.analyzeExecution(&notInstrumented), std::bad_cast)
        << "Analysis of not instrumented TPG didn't failed or did with an unexpected error.";

}

TEST_F(ExecutionStatsTest, AnalyzeInferenceTrace){

    TPG::ExecutionStats executionStats;

    ASSERT_EQ(executionStats.getExecutionTracesStats().size(), 0)
        << "Attribute executionTracesStats isn't empty at initialisation.";

    ASSERT_NO_THROW(executionStats.analyzeInferenceTrace(executionTraces[3]))
        << "Analysis of execution trace failed unexpectedly.";

    ASSERT_EQ(executionStats.getExecutionTracesStats().size(), 1)
        << "Attribute executionTraceStats doesn't have just the analyzed trace statistics.";
    ASSERT_EQ(executionStats.getExecutionTracesStats()[0].trace, executionTraces[3])
        << "Wrong analyzed execution trace in executionStats.";


    const TPG::TraceStats& stats = executionStats.getExecutionTracesStats()[0];

    ASSERT_EQ(stats.nbEvaluatedTeams, 3)
        << "Wrong number of evaluated teams.";
    ASSERT_EQ(stats.nbEvaluatedPrograms, 7)
        << "Wrong number of evaluated programs.";
    ASSERT_EQ(stats.nbExecutedLines, 9)
        << "Wrong number of executed lines.";
    // Add
    ASSERT_EQ(stats.nbExecutionPerInstruction.at(0), 1)
        << "Wrong number of executed instruction.";
    // mac
    ASSERT_EQ(stats.nbExecutionPerInstruction.at(1), 1)
        << "Wrong number of executed instruction.";
    // Minus
    ASSERT_EQ(stats.nbExecutionPerInstruction.at(2), 5)
        << "Wrong number of executed instruction.";
    // MultByConst
    ASSERT_EQ(stats.nbExecutionPerInstruction.at(3), 2)
        << "Wrong number of executed instruction.";

}