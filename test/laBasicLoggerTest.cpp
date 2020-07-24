/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2020)
 * Pierre-Yves Le Rolland-Raumer <plerolla@insa-rennes.fr> (2020)
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
#include <chrono>
#include <gtest/gtest.h>
#include <thread>

#include "instructions/addPrimitiveType.h"
#include "instructions/multByConstParam.h"
#include "learn/learningAgent.h"
#include "learn/stickGameWithOpponent.h"

#include "log/laBasicLogger.h"

class LABasicLoggerTest : public ::testing::Test
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

        params.archiveSize = 50;
        params.archivingProbability = 0.5;
        params.maxNbActionsPerEval = 11;
        params.nbIterationsPerPolicyEvaluation = 3;
        params.ratioDeletedRoots =
            0.95; // high number to force the apparition of root action.
        params.nbThreads = 1;

        set.add(*(new Instructions::AddPrimitiveType<double>()));
        set.add(*(new Instructions::MultByConstParam<double, float>()));

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

TEST_F(LABasicLoggerTest, Constructor)
{
    Log::LABasicLogger* l = nullptr;
    ASSERT_NO_THROW(l = new Log::LABasicLogger(*la));
    if (l != nullptr) {
        delete l;
    }
    ASSERT_NO_THROW(Log::LABasicLogger l(*la, std::cerr));
}

TEST_F(LABasicLoggerTest, logHeader)
{
    std::stringstream strStr;
    // basic header without validation
    Log::LABasicLogger l(*la, strStr);

    // we log a second header with validation column
    l.doValidation = true;
    l.logHeader();

    // now we will check the header logged correctly
    std::string s = strStr.str();
    // putting each element seperated by blanks in a tab
    std::vector<std::string> result;
    std::istringstream iss(s);
    for (std::string s2; iss >> s2;)
        result.push_back(s2);

    ASSERT_EQ("Gen", result[0]);
    ASSERT_EQ("NbVert", result[1]);
    ASSERT_EQ("Min", result[2]);
    ASSERT_EQ("Avg", result[3]);
    ASSERT_EQ("Max", result[4]);
    ASSERT_EQ("T_mutat", result[5]);
    ASSERT_EQ("T_eval", result[6]);
    ASSERT_EQ("T_total", result[7]);
    ASSERT_EQ("T_valid", result[15]);
}

TEST_F(LABasicLoggerTest, logNewGeneration)
{
    std::stringstream strStr;
    Log::LABasicLogger l(*la, strStr);
    uint64_t nbGen = 42;

    l.logNewGeneration(nbGen);

    std::string s = strStr.str();
    // putting each element seperated by blanks in a tab
    std::vector<std::string> result;
    std::istringstream iss(s);
    for (std::string s2; iss >> s2;)
        result.push_back(s2);

    // index 8 because we skip the header
    ASSERT_EQ("42", result[8]);
    ASSERT_EQ(result.size(), 8 + 1);
}

TEST_F(LABasicLoggerTest, logAfterPopulateTPG)
{
    la->init();
    std::stringstream strStr;
    Log::LABasicLogger l(*la, strStr);

    l.logAfterPopulateTPG();
    std::string s = strStr.str();
    // putting each element seperated by blanks in a tab
    std::vector<std::string> result;
    std::istringstream iss(s);
    for (std::string s2; iss >> s2;)
        result.push_back(s2);

    // index 8 because we skip the header
    ASSERT_EQ("6", result[8])
        << "Unexpected number of vertices was printed in the log.";
}

TEST_F(LABasicLoggerTest, logAfterEvaluate)
{
    std::stringstream strStr;
    Log::LABasicLogger l(*la, strStr);

    l.logAfterEvaluate(results);
    std::string s = strStr.str();
    // putting each element seperated by blanks in a tab
    std::vector<std::string> result;
    std::istringstream iss(s);
    for (std::string s2; iss >> s2;)
        result.push_back(s2);

    // index 8 because we skip the header
    ASSERT_DOUBLE_EQ(5.00, std::stod(result[8]));
    ASSERT_DOUBLE_EQ(7.50, std::stod(result[9]));
    ASSERT_DOUBLE_EQ(10.00, std::stod(result[10]));
}

TEST_F(LABasicLoggerTest, logAfterValidate)
{
    std::stringstream strStr;
    Log::LABasicLogger l(*la, strStr);

    l.logAfterValidate(results);
    std::string s = strStr.str();
    // putting each element seperated by blanks in a tab
    std::vector<std::string> result;
    std::istringstream iss(s);
    for (std::string s2; iss >> s2;)
        result.push_back(s2);

    // index 8+ because we skip the header
    ASSERT_DOUBLE_EQ(5.00, std::stod(result[8]));
    ASSERT_DOUBLE_EQ(7.50, std::stod(result[9]));
    ASSERT_DOUBLE_EQ(10.00, std::stod(result[10]));
}

TEST_F(LABasicLoggerTest, logAfterDecimate)
{
    la->init();
    std::stringstream strStr;
    Log::LABasicLogger l(*la, strStr);
    ASSERT_NO_THROW(l.logAfterDecimate());
}

TEST_F(LABasicLoggerTest, logEndOfTraining)
{
    // To test chrono, we will wait, use chronoFromNow() which resets the
    // "checkpoint" time, then call logAfterEvaluate() which will register
    // evalTime and call logEndOfTraining() which shall log the duration from
    // checkpoint and from start.
    // The total duration should be larger than the evalTime

    std::stringstream strStr;
    Log::LABasicLogger l(*la, strStr);

    // little sleep to delay the total_time value (while the "checkpoint" of the
    // logger will be reset)
    size_t timeToWaitMili = 10;
    std::this_thread::sleep_for(std::chrono::milliseconds(timeToWaitMili));

    // resets "checkpoint" so that the first displayed time shall be lower than
    // the second which is the time from start
    l.chronoFromNow();
    l.doValidation = true; // to avoid logging eval statistics
    l.logAfterEvaluate(results);
    l.logEndOfTraining();
    // then, we can test the method when there is no validation
    l.doValidation = false; // to avoid logging eval statistics
    l.logEndOfTraining();

    std::string s = strStr.str();
    // putting each element seperated by blanks in a tab
    std::cout << s;
    std::vector<std::string> result;
    std::istringstream iss(s);
    for (std::string s2; iss >> s2;)
        result.push_back(s2);

    // index 8+ because we skip the headers
    double mutatTime = std::stod(result[8]);
    double evalTime = std::stod(result[9]);
    double validTime = std::stod(result[10]);
    double totTime = std::stod(result[11]);
    ASSERT_GE(mutatTime, 0) << "Eval duration should be positive";
    ASSERT_GE(evalTime, 0) << "Eval duration should be positive";
    ASSERT_GE(validTime, 0) << "Valid duration should be positive";
    ASSERT_GE(totTime, evalTime)
        << "Total time should be the largest duration !";
    ASSERT_GE(totTime, timeToWaitMili / 1000)
        << "Total time should be larger than the time we waited !";

    // Size is headerSize (8) + log size (4 + 3)
    ASSERT_EQ(result.size(), 8 + 7)
        << "logEndOfTraining with and without valid should have 4+3=7 elements";
}
