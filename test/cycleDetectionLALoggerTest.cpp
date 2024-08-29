/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2022) :
 *
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

#include "instructions/addPrimitiveType.h"
#include "instructions/multByConstant.h"
#include "learn/learningAgent.h"
#include "learn/stickGameWithOpponent.h"

#include "log/cycleDetectionLALogger.h"

class CycleDetectionLoggerTest : public ::testing::Test
{
  protected:
    Instructions::Set set;

    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  const TPG::TPGVertex*>
        results;

    StickGameWithOpponent le;
    Learn::LearningParameters params;
    Learn::LearningAgent* la;

    void SetUp() override
    {
        // Proba as in Kelly's paper
        params.mutation.tpg.maxInitOutgoingEdges = 3;
        params.mutation.prog.maxProgramSize = 96;
        params.mutation.tpg.nbRoots = 15;
        params.mutation.tpg.pEdgeDeletion = 0.7;
        params.mutation.tpg.pEdgeAddition = 0.7;
        params.mutation.tpg.pProgramMutation = 0.2;
        params.mutation.tpg.pEdgeDestinationChange = 0.1;
        params.mutation.tpg.pEdgeDestinationIsAction = 0.5;
        params.mutation.tpg.maxOutgoingEdges = 4;
        params.mutation.prog.pAdd = 0.5;
        params.mutation.prog.pDelete = 0.5;
        params.mutation.prog.pMutate = 1.0;
        params.mutation.prog.pSwap = 1.0;
        params.mutation.prog.minConstValue = 0;
        params.mutation.prog.maxConstValue = 3;
        params.nbProgramConstant = 0;

        params.archiveSize = 50;
        params.archivingProbability = 0.5;
        params.maxNbActionsPerEval = 11;
        params.nbIterationsPerPolicyEvaluation = 3;
        params.ratioDeletedRoots =
            0.95; // high number to force the apparition of root action.
        params.nbThreads = 1;
        params.nbProgramConstant = 5;

        set.add(*(new Instructions::AddPrimitiveType<double>()));
        set.add(*(new Instructions::MultByConstant<double>()));

        auto res1 = new Learn::EvaluationResult(5, 2);
        auto res2 = new Learn::EvaluationResult(10, 2);
        auto v1(new TPG::TPGAction(0, 0));
        auto v2(new TPG::TPGAction(0, 0));
        results.insert(std::pair<std::shared_ptr<Learn::EvaluationResult>,
                                 const TPG::TPGVertex*>(res1, v1));
        results.insert(std::pair<std::shared_ptr<Learn::EvaluationResult>,
                                 const TPG::TPGVertex*>(res2, v2));

        la = new Learn::LearningAgent(le, set, params);
    }

    void TearDown() override
    {
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
        auto it = results.begin();
        delete it->second;
        it++;
        delete it->second;
        delete la;
    }
};

TEST_F(CycleDetectionLoggerTest, Constructor)
{
    Log::CycleDetectionLALogger* l = nullptr;
    ASSERT_NO_THROW(l = new Log::CycleDetectionLALogger(*la));
    if (l != nullptr) {
        delete l;
    }
    ASSERT_NO_THROW(Log::CycleDetectionLALogger l(*la, std::cerr));
}

TEST_F(CycleDetectionLoggerTest, EmptyMethods)
{
    la->init();
    std::stringstream strStr;
    Log::CycleDetectionLALogger l(*la, strStr);

    // Call to all empty methods
    l.logAfterDecimate();
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                  const TPG::TPGVertex*>
        results;
    l.logAfterEvaluate(results);
    l.logAfterValidate(results);
    l.logEndOfTraining();
    l.logHeader();
    uint64_t generation;
    l.logNewGeneration(generation);

    ASSERT_EQ(strStr.str().length(), 0)
        << "Empty methods should not generate any log.";
}

TEST_F(CycleDetectionLoggerTest, logAfterPopulateTPG)
{
    la->init();
    std::stringstream strStr;
    Log::CycleDetectionLALogger l(*la, strStr);

    l.logAfterPopulateTPG();
    std::string s = strStr.str();

    ASSERT_EQ(s.length(), 0)
        << "TPG after initialization should not contain any cycle.";

    // Change the TPG
    //
    //  T0-->T1   T4
    //   |   |     |
    //   v   v     v
    //  T2-->T3   T5
    //       |     |
    //       v     v
    //       A0    A1

    auto tpg = la->getTPGGraph();
    tpg->clear();
    std::vector<const TPG::TPGTeam*> teams;
    for (auto idx = 0; idx < 6; idx++) {
        teams.push_back(&(tpg->addNewTeam()));
    }

    std::vector<const TPG::TPGAction*> actions;
    for (auto idx = 0; idx < 2; idx++) {
        actions.push_back(&tpg->addNewAction(0, 0));
    }

    // tree 1
    tpg->addNewEdge(*teams[0], *teams[1],
                    std::make_shared<Program::Program>(la->getEnvironment()));
    tpg->addNewEdge(*teams[0], *teams[2],
                    std::make_shared<Program::Program>(la->getEnvironment()));
    tpg->addNewEdge(*teams[1], *teams[3],
                    std::make_shared<Program::Program>(la->getEnvironment()));
    tpg->addNewEdge(*teams[2], *teams[3],
                    std::make_shared<Program::Program>(la->getEnvironment()));
    tpg->addNewEdge(*teams[3], *actions[0],
                    std::make_shared<Program::Program>(la->getEnvironment()));

    tpg->addNewEdge(*teams[4], *teams[5],
                    std::make_shared<Program::Program>(la->getEnvironment()));
    tpg->addNewEdge(*teams[5], *actions[1],
                    std::make_shared<Program::Program>(la->getEnvironment()));

    // Check cycle detection again
    strStr.str(std::string()); // clear the string
    l.logAfterPopulateTPG();
    s = strStr.str();

    ASSERT_EQ(s.length(), 0) << "Custom TPG does not contain any cycle.";

    // Check with a "positive" detection for this case
    std::stringstream strStr2;
    Log::CycleDetectionLALogger l2(*la, strStr2, true);
    l2.logAfterPopulateTPG();
    ASSERT_EQ(strStr2.str(), "No cycle detected in this TPG.")
        << "Logging from \"logOnDetection\" logger in incorrect.";

    // Add a cycle to the graph
    // A subgraph won't be traversed by the DFS because it has no root.
    //
    // .>T0-->T1   T4
    // |  |   |     |
    // |  v   v     v
    // | T2-->T3   T5
    // |_____/|     |
    //        v     v
    //        A0    A1

    tpg->addNewEdge(*teams[3], *teams[0],
                    std::make_shared<Program::Program>(la->getEnvironment()));

    // Check cycle detection again
    strStr.str(std::string()); // clear the string
    l.logAfterPopulateTPG();
    s = strStr.str();

    ASSERT_GT(s.length(), 0) << "Cycle in custom TPG is not detected.";

    // Add a root to the cycle.
    //        T6
    //         |
    //         v
    // .>T0-->T1   T4
    // |  |   |     |
    // |  v   v     v
    // | T2-->T3   T5
    // |_____/|     |
    //        v     v
    //        A0    A1

    // Add a new team and edge
    teams.push_back(&(tpg->addNewTeam()));
    tpg->addNewEdge(*teams[6], *teams[1],
                    std::make_shared<Program::Program>(la->getEnvironment()));

    // Check cycle detection again
    strStr.str(std::string()); // clear the string
    l.logAfterPopulateTPG();
    s = strStr.str();

    ASSERT_GT(s.length(), 0) << "Cycle in custom TPG is not detected.";
}
