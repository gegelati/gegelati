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
        auto v1(new TPG::TPGAction(0));
        auto v2(new TPG::TPGAction(0));
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