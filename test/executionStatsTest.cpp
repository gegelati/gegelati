/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2022) :
 *
 * Elinor Montmasson <elinor.montmasson@gmail.com> (2022)
 * Karol Desnos <kdesnos@insa-rennes.fr> (2022)
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
#include "environment.h"
#include "goldenReferenceComparison.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/lambdaInstruction.h"
#include "instructions/multByConstant.h"
#include "instructions/set.h"
#include "tpg/instrumented/tpgExecutionEngineInstrumented.h"
#include "tpg/instrumented/tpgInstrumentedFactory.h"
#include "tpg/tpgGraph.h"

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

    std::vector<std::vector<const TPG::TPGVertex*>> inferenceTraces;

    virtual void SetUp() override
    {

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

        // Create 8 programs
        for (int i = 0; i < 8; i++) {
            progPointers.push_back(std::make_shared<Program::Program>(*e));
        }

        // Create a TPG
        // (T= Team, A= Action)
        // T0 will be considered the main team root
        // T3 will also be considered a team root, but we will never start an
        // inference from it, so his nbVisited attribute will stay at 0
        // and the number of inferences will correctly be deduced by the
        // executionStats class.
        //
        // T0---->T1---->T2<----T3
        // |     /| \    |       |
        // v    / v  \   v       v
        // A0<-'  A1  `->A2     A3
        //
        // With four action and four teams
        // All Edges have a unique Program, except T1->A0 and T0->A0 which
        // share the same program: progPointers.at(0)

        // The TPG is given a TPGInstrumentedFactory to enable instrumentation
        tpg = new TPG::TPGGraph(
            *e, std::make_unique<TPG::TPGInstrumentedFactory>());
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

        // Add new outgoing edge to one team
        edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(1),
                                         *tpg->getVertices().at(4),
                                         progPointers.at(0)));
        edges.push_back(&tpg->addNewEdge(*tpg->getVertices().at(1),
                                         *tpg->getVertices().at(6),
                                         progPointers.at(7)));

        // Check the characteristics
        ASSERT_EQ(tpg->getNbVertices(), 8);
        ASSERT_EQ(tpg->getEdges().size(), 9);
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
        l->setOperand(1, 0, 2);    // Array[2]

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

        // Program 7 (T1 -> A2)
        l = &progPointers.at(7).get()->addNewLine();
        l->setInstructionIndex(1); // mac
        l->setDestinationIndex(0);
        l->setOperand(0, 2, 6); // Array[6]
        l->setOperand(1, 2, 7); // Array[7..9]

        // Program 2 (T2 -> A2)
        l = &progPointers.at(2).get()->addNewLine();
        l->setInstructionIndex(0); // Add
        l->setDestinationIndex(0);
        l->setOperand(0, 2, 3); // Array[3]
        l->setOperand(1, 2, 4); // Array[4]

        // TPG execution

        // Three graph executions to do :
        //  - T0 -> T1 -> A2
        //  - T0 -> T1 -> A1
        //  - T0 -> T1 -> T2 -> A2

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
        ASSERT_NO_THROW(inferenceTraces.push_back(
            execEngine->executeFromRoot(*tpg->getVertices().at(0), {0}, 1)
                .first));

        //  - T0 -> T1 -> A1
        data->setDataAt(typeid(double), 10, 10);
        data->setDataAt(typeid(double), 12, -3);
        // P0 = -12
        // P1 = 13
        // P2 = 10
        // P8 = 12
        ASSERT_NO_THROW(inferenceTraces.push_back(
            execEngine->executeFromRoot(*tpg->getVertices().at(0), {0}, 1)
                .first));

        //  - T0 -> T1 -> T2 -> A2
        data->setDataAt(typeid(double), 12, 13);
        data->setDataAt(typeid(double), 6, -3);
        // P0 = -12
        // P1 = -3
        // P2 = 10
        // P8 = -18
        ASSERT_NO_THROW(inferenceTraces.push_back(
            execEngine->executeFromRoot(*tpg->getVertices().at(0), {0}, 1)
                .first));
    }

    virtual void TearDown() override
    {

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

TEST_F(ExecutionStatsTest, AnalyzeInstrumentedGraph)
{
    TPG::ExecutionStats executionStats;
    ASSERT_NO_THROW(executionStats.analyzeInstrumentedGraph(tpg))
        << "Analysis of a valid tpg execution failed unexpectedly.";

    ASSERT_EQ(executionStats.getAvgEvaluatedTeams(), 7.0 / 3.0)
        << "Incorrect attribute value after analyzing execution.";
    ASSERT_EQ(executionStats.getAvgEvaluatedPrograms(), 19.0 / 3.0)
        << "Incorrect attribute value after analyzing execution.";
    ASSERT_EQ(executionStats.getAvgExecutedLines(), 25.0 / 3.0)
        << "Incorrect attribute value after analyzing execution.";
    // Add
    ASSERT_EQ(executionStats.getAvgNbExecutionPerInstruction().at(0), 1.0 / 3.0)
        << "Incorrect attribute value after analyzing execution.";
    // mac
    ASSERT_EQ(executionStats.getAvgNbExecutionPerInstruction().at(1), 3.0 / 3.0)
        << "Incorrect attribute value after analyzing execution.";
    // Minus
    ASSERT_EQ(executionStats.getAvgNbExecutionPerInstruction().at(2),
              15.0 / 3.0)
        << "Incorrect attribute value after analyzing execution.";
    // MultByConst
    ASSERT_EQ(executionStats.getAvgNbExecutionPerInstruction().at(3), 6.0 / 3.0)
        << "Incorrect attribute value after analyzing execution.";
}

TEST_F(ExecutionStatsTest, AnalyzeNotInstrumented)
{

    TPG::TPGGraph notInstrumented(*e);
    notInstrumented.addNewTeam();

    TPG::ExecutionStats executionStats;
    ASSERT_THROW(executionStats.analyzeInstrumentedGraph(&notInstrumented),
                 std::bad_cast)
        << "Analysis of not instrumented TPG didn't failed or did with an "
           "unexpected error.";
}

TEST_F(ExecutionStatsTest, AnalyzeInferenceTrace)
{

    TPG::ExecutionStats executionStats;

    ASSERT_EQ(executionStats.getInferenceTracesStats().size(), 0)
        << "Attribute inferenceTracesStats isn't empty at initialisation.";

    ASSERT_NO_THROW(executionStats.analyzeInferenceTrace(inferenceTraces[2]))
        << "Analysis of execution trace failed unexpectedly.";

    ASSERT_EQ(executionStats.getInferenceTracesStats().size(), 1)
        << "Attribute executionTraceStats doesn't have just the analyzed trace "
           "statistics.";
    ASSERT_EQ(executionStats.getInferenceTracesStats()[0].trace,
              inferenceTraces[2])
        << "Wrong analyzed execution trace in executionStats.";

    const TPG::TraceStats& stats = executionStats.getInferenceTracesStats()[0];

    ASSERT_EQ(stats.nbEvaluatedTeams, 3) << "Wrong number of evaluated teams.";
    ASSERT_EQ(stats.nbEvaluatedPrograms, 7)
        << "Wrong number of evaluated programs.";
    ASSERT_EQ(stats.nbExecutedLines, 9) << "Wrong number of executed lines.";
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

    /* Distributions */

    std::map<size_t, size_t> expectedDistribEvaluatedTeams = {{3, 1}};
    std::map<size_t, size_t> expectedDistribEvaluatedPrograms = {{7, 1}};
    std::map<size_t, size_t> expectedDistribExecutedLines = {{9, 1}};
    std::map<size_t, std::map<size_t, size_t>>
        expectedDistribNbExecutionPerInstruction = {
            {0, {{1, 1}}},
            {1, {{1, 1}}},
            {2, {{5, 1}}},
            {3, {{2, 1}}},
        };
    std::map<const TPG::TPGVertex*, size_t> expectedDistribUsedVertices = {
        {tpg->getVertices()[0], 1},
        {tpg->getVertices()[1], 1},
        {tpg->getVertices()[2], 1},
        {tpg->getVertices()[6], 1}};

    ASSERT_EQ(expectedDistribEvaluatedTeams,
              executionStats.getDistribEvaluatedTeams())
        << "Wrong evaluated teams distribution.";
    ASSERT_EQ(expectedDistribEvaluatedPrograms,
              executionStats.getDistribEvaluatedPrograms())
        << "Wrong evaluated programs distribution.";
    ASSERT_EQ(expectedDistribExecutedLines,
              executionStats.getDistribExecutedLines())
        << "Wrong executed lines distribution.";
    ASSERT_EQ(expectedDistribNbExecutionPerInstruction,
              executionStats.getDistribNbExecutionPerInstruction())
        << "Wrong executions per instruction distributions.";
    ASSERT_EQ(expectedDistribUsedVertices,
              executionStats.getDistribUsedVertices())
        << "Wrong used vertices distribution.";
}

TEST_F(ExecutionStatsTest, ClearTracesStats)
{

    TPG::ExecutionStats executionStats;
    executionStats.analyzeInferenceTrace(inferenceTraces[2]);
    executionStats.analyzeInferenceTrace(inferenceTraces[1]);

    ASSERT_EQ(executionStats.getInferenceTracesStats().size(), 2)
        << "Not enough TraceStats after trace analysis.";
    ASSERT_NO_THROW(executionStats.clearInferenceTracesStats())
        << "Clearing inferenceTracesStats failed unexpectedly.";
    ASSERT_EQ(executionStats.getInferenceTracesStats().size(), 0)
        << "inferenceTracesStats is not empty after clearing.";

    ASSERT_EQ(executionStats.getDistribEvaluatedTeams().size(), 0)
        << "distribEvaluatedTeams is not empty after clearing.";

    ASSERT_EQ(executionStats.getDistribEvaluatedPrograms().size(), 0)
        << "distribEvaluatedPrograms is not empty after clearing.";

    ASSERT_EQ(executionStats.getDistribExecutedLines().size(), 0)
        << "distribExecutedLines is not empty after clearing.";

    ASSERT_EQ(executionStats.getDistribNbExecutionPerInstruction().size(), 0)
        << "distribNbExecutionPerInstruction is not empty after clearing.";

    ASSERT_EQ(executionStats.getDistribUsedVertices().size(), 0)
        << "distribUsedVertices is not empty after clearing.";
}

TEST_F(ExecutionStatsTest, AnalyzeExecution)
{

    TPG::ExecutionStats executionStats;

    // This analyzed traces must be cleared by analyzeExecution()
    executionStats.analyzeInferenceTrace(inferenceTraces[1]);

    ASSERT_NO_THROW(executionStats.analyzeExecution(*execEngine, tpg))
        << "Analysing execution failed unexpectedly.";

    ASSERT_EQ(executionStats.getInferenceTracesStats().size(), 3)
        << "Incorrect number of analyzed traces.";

    /* Average graph execution statistics */
    ASSERT_EQ(executionStats.getAvgEvaluatedTeams(), 7.0 / 3.0)
        << "Incorrect attribute value after analyzing execution.";
    ASSERT_EQ(executionStats.getAvgEvaluatedPrograms(), 19.0 / 3.0)
        << "Incorrect attribute value after analyzing execution.";
    ASSERT_EQ(executionStats.getAvgExecutedLines(), 25.0 / 3.0)
        << "Incorrect attribute value after analyzing execution.";
    // Add
    ASSERT_EQ(executionStats.getAvgNbExecutionPerInstruction().at(0), 1.0 / 3.0)
        << "Incorrect attribute value after analyzing execution.";
    // mac
    ASSERT_EQ(executionStats.getAvgNbExecutionPerInstruction().at(1), 3.0 / 3.0)
        << "Incorrect attribute value after analyzing execution.";
    // Minus
    ASSERT_EQ(executionStats.getAvgNbExecutionPerInstruction().at(2),
              15.0 / 3.0)
        << "Incorrect attribute value after analyzing execution.";
    // MultByConst
    ASSERT_EQ(executionStats.getAvgNbExecutionPerInstruction().at(3), 6.0 / 3.0)
        << "Incorrect attribute value after analyzing execution.";

    const TPG::TraceStats& stats = executionStats.getInferenceTracesStats()[2];

    ASSERT_EQ(stats.nbEvaluatedTeams, 3) << "Wrong number of evaluated teams.";
    ASSERT_EQ(stats.nbEvaluatedPrograms, 7)
        << "Wrong number of evaluated programs.";
    ASSERT_EQ(stats.nbExecutedLines, 9) << "Wrong number of executed lines.";
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

    /* Distributions */

    std::map<size_t, size_t> expectedDistribEvaluatedTeams = {{2, 2}, {3, 1}};
    std::map<size_t, size_t> expectedDistribEvaluatedPrograms = {{6, 2},
                                                                 {7, 1}};
    std::map<size_t, size_t> expectedDistribExecutedLines = {{8, 2}, {9, 1}};
    std::map<size_t, std::map<size_t, size_t>>
        expectedDistribNbExecutionPerInstruction = {
            {0, {{1, 1}}},
            {1, {{1, 3}}},
            {2, {{5, 3}}},
            {3, {{2, 3}}},
        };
    std::map<const TPG::TPGVertex*, size_t> expectedDistribUsedVertices = {
        {tpg->getVertices()[0], 3},
        {tpg->getVertices()[1], 3},
        {tpg->getVertices()[2], 1},
        {tpg->getVertices()[5], 1},
        {tpg->getVertices()[6], 2}};

    ASSERT_EQ(expectedDistribEvaluatedTeams,
              executionStats.getDistribEvaluatedTeams())
        << "Wrong evaluated teams distribution.";
    ASSERT_EQ(expectedDistribEvaluatedPrograms,
              executionStats.getDistribEvaluatedPrograms())
        << "Wrong evaluated programs distribution.";
    ASSERT_EQ(expectedDistribExecutedLines,
              executionStats.getDistribExecutedLines())
        << "Wrong executed lines distribution.";
    ASSERT_EQ(expectedDistribNbExecutionPerInstruction,
              executionStats.getDistribNbExecutionPerInstruction())
        << "Wrong executions per instruction distributions.";
    ASSERT_EQ(expectedDistribUsedVertices,
              executionStats.getDistribUsedVertices())
        << "Wrong used vertices distribution.";
}

TEST_F(ExecutionStatsTest, WriteStatsToJson)
{

    TPG::ExecutionStats executionStats;
    executionStats.analyzeExecution(*execEngine, tpg);

    ASSERT_NO_THROW(executionStats.writeStatsToJson("execution_stats.json"))
        << "Exporting execution statistics to file failed unexpectedly.";

    ASSERT_TRUE(compare_files("execution_stats.json",
                              TESTS_DAT_PATH "execution_stats_ref.json"))
        << "Generated json file is different from the reference file.";
}
