
#include <gtest/gtest.h>

#include "instructions/addPrimitiveType.h"
#include "learn/fakeMultiContinuousLearningEnvironment.h"
#include "learn/learningAgent.h"
#include "learn/stickGameWithOpponent.h"
#include "instructions/set.h"

#include "log/mapElitesArchiveLogger.h"
#include "selector/mapElites/mapElitesDefaultDescriptors.h"
#include "parameters.h"

class MapElitesLoggerTest : public ::testing::Test
{
  protected:
    Instructions::Set set;
    FakeMultiContinuousLearningEnvironment le;
    Parameters params;
    Learn::LearningAgent* la;

    Selector::MapElites::MapElitesArchive* archive;

    std::shared_ptr<Selector::MapElitesSelector> selector;
    std::shared_ptr<Selector::MapElites::DefaultDescriptors::ActionValues>
        descriptor;

    void SetUp() override
    {
        // Proba as in Kelly's paper
        params.representation.tpg.maxInitOutgoingEdges = 3;
        params.representation.lgp.maxProgramSize = 96;
        params.representation.nbAgents = 15;
        params.representation.tpg.pEdgeDeletion = 0.7;
        params.representation.tpg.pEdgeAddition = 0.7;
        params.representation.tpg.pProgramMutation = 0.2;
        params.representation.tpg.pEdgeDestinationChange = 0.1;
        params.representation.tpg.pEdgeDestinationIsAction = 0.5;
        params.representation.tpg.maxOutgoingEdges = 4;
        params.representation.lgp.pAdd = 0.5;
        params.representation.lgp.pDelete = 0.5;
        params.representation.lgp.pMutate = 1.0;
        params.representation.lgp.pSwap = 1.0;
        params.representation.lgp.nbProgramConstant = 0;

        params.representation.tpg.archiveSize = 50;
        params.representation.tpg.archivingProbability = 0.5;
        params.evaluation.maxNbActionsPerEval = 11;
        params.evaluation.nbIterationsPerPolicyEvaluation = 3;
        params.selection.truncation.ratioDeletedRoots =
            0.95; // high number to force the apparition of root action.
        params.evaluation.nbThreads = 1;

        params.selection._selectionMode = "mapElites";

        set.add(*(new Instructions::AddPrimitiveType<int>()));
        set.add(*(new Instructions::AddPrimitiveType<double>()));

        /*la = new Learn::LearningAgent(le, set, params);

        selector = std::dynamic_pointer_cast<Selector::MapElitesSelector>(
            la->getSelector());
*/
        descriptor = std::make_shared<
            Selector::MapElites::DefaultDescriptors::ActionValues>();
        descriptor->initDescriptor(la->getGraph(), le);
    }

    void TearDown() override
    {
        delete la;
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
    }
};

#if 0
TEST_F(MapElitesLoggerTest, Constructor)
{
    auto archive = selector->addArchiveFromDescriptor(2, descriptor, le);
    Log::MapElitesArchiveLogger* log = nullptr;
    ASSERT_NO_THROW(log = new Log::MapElitesArchiveLogger(*archive, *la));
    if (log != nullptr) {
        delete log;
    }

    la->init(3);
    auto cvtArchive =
        selector->addCvtArchiveFromDescriptor(4, descriptor, le, la->getRNG());
    Log::MapElitesArchiveLogger* cvtLog = nullptr;
    ASSERT_NO_THROW(cvtLog = new Log::MapElitesArchiveLogger(*cvtArchive, *la));
    if (cvtLog != nullptr) {
        delete cvtLog;
    }
}

TEST_F(MapElitesLoggerTest, logHeader)
{

    auto archive = selector->addArchiveFromDescriptor(2, descriptor, le);

    // add the Logger
    std::stringstream strStr;
    Log::MapElitesArchiveLogger log(*archive, *la, strStr);
    ASSERT_EQ(strStr.str(), "generation,0_0_0,0_0_1,0_1_0,0_1_1,1_0_0,1_0_1,1_"
                            "1_0,1_1_1,archiveRange\n")
        << "Output of the archive is not as expected.";

    la->init(3);
    auto cvtArchive =
        selector->addCvtArchiveFromDescriptor(4, descriptor, le, la->getRNG());

    // add the Logger, plot the deterministic values
    std::stringstream cvtStrStr;
    Log::MapElitesArchiveLogger cvtLog(*cvtArchive, *la, cvtStrStr);
    ASSERT_EQ(
        cvtStrStr.str(),
        "generation,0(0.476939;0.776662;0.737521),1(0.5028;0.231641;0.730523),"
        "2(0.779164;0.519662;0.275792),3(0.229133;0.462147;0.256459)\n")
        << "Output of the archive is not as expected.";
}

TEST_F(MapElitesLoggerTest, logNewGeneration)
{

    auto archive = selector->addArchiveFromDescriptor(2, descriptor, le);

    // add the Logger
    std::stringstream strStr;
    Log::MapElitesArchiveLogger log(*archive, *la, strStr);

    uint64_t generationValue = 0;
    log.logNewGeneration(generationValue);

    ASSERT_EQ(strStr.str().back(), '0') << "Generation value should be 0";
}

TEST_F(MapElitesLoggerTest, logEndOfTraining)
{

    auto archive = selector->addArchiveFromDescriptor(2, descriptor, le);

    // add the Logger
    std::stringstream strStr;
    Log::MapElitesArchiveLogger log(*archive, *la, strStr);

    la->init(3);
    la->trainOneGeneration(0);

    std::string content = strStr.str();
    content.erase(content.length() - 1);
    std::string lastLine = content.substr(content.rfind('\n') + 1);
    ASSERT_EQ(lastLine, "0,0,10,10,nan,10,nan,nan,nan,0.5;1");

    la->trainOneGeneration(1);

    content = strStr.str();
    content.erase(content.length() - 1);
    lastLine = content.substr(content.rfind('\n') + 1);
    ASSERT_EQ(lastLine, "1,0,10,10,20,10,20,20,30");
}
TEST_F(MapElitesLoggerTest, logEndOfTrainingCvt)
{

    auto archive =
        selector->addCvtArchiveFromDescriptor(4, descriptor, le, la->getRNG());

    // add the Logger
    std::stringstream strStr;
    Log::MapElitesArchiveLogger log(*archive, *la, strStr);

    la->init(3);
    la->trainOneGeneration(0);

    std::string content = strStr.str();
    content.erase(content.length() - 1);
    std::string lastLine = content.substr(content.rfind('\n') + 1);
    ASSERT_EQ(lastLine, "0,10,10,10,nan");

    la->trainOneGeneration(1);

    content = strStr.str();
    content.erase(content.length() - 1);
    lastLine = content.substr(content.rfind('\n') + 1);
    ASSERT_EQ(lastLine, "1,20,10,10,30");
}
#endif