/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2020 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2020 - 2021)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
 * Pierre-Yves Le Rolland-Raumer <plerolla@insa-rennes.fr> (2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
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
#include "instructions/multByConstant.h"
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
        params.mutation.prog.minConstValue = 0;
        params.mutation.prog.maxConstValue = 3;
        params.nbProgramConstant = 0;

        params.archiveSize = 50;
        params.archivingProbability = 0.5;
        params.maxNbActionsPerEval = 11;
        params.nbIterationsPerPolicyEvaluation = 3;
        params.selection.truncation.ratioDeletedRoots =
            0.95; // high number to force the apparition of root action.
        params.nbThreads = 1;
        params.nbProgramConstant = 5;

        set.add(*(new Instructions::AddPrimitiveType<double>()));
        set.add(*(new Instructions::MultByConstant<double>()));

        auto res1 = new Learn::EvaluationResult(5, 2, 2);
        auto res2 = new Learn::EvaluationResult(10, 2, 4);
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

    // we log a third header with validation column
    l.useUtility = true;
    l.logHeader();

    // now we will check the header logged correctly
    std::string s = strStr.str();
    // putting each element seperated by blanks in a tab
    std::vector<std::string> result;
    std::istringstream iss(s);
    for (std::string s2; iss >> s2;)
        result.push_back(s2);

    ASSERT_EQ("Train", result[0]);
    ASSERT_EQ("Gen", result[1]);
    ASSERT_EQ("NbVert", result[2]);
    ASSERT_EQ("NbActR", result[3]);
    ASSERT_EQ("NbTeamR", result[4]);
    ASSERT_EQ("Min", result[5]);
    ASSERT_EQ("Avg", result[6]);
    ASSERT_EQ("Max", result[7]);
    ASSERT_EQ("T_eval", result[8]);
    ASSERT_EQ("T_decim", result[9]);
    ASSERT_EQ("T_mutat", result[10]);
    ASSERT_EQ("T_total", result[11]);
    ASSERT_EQ("Valid", result[13]);
    ASSERT_EQ("Avg", result[22]);
    ASSERT_EQ("T_valid", result[25]);
    ASSERT_EQ("U_Min", result[35]);
    ASSERT_EQ("U_Avg", result[36]);
    ASSERT_EQ("U_Max", result[37]);
    ASSERT_EQ("R_Min", result[38]);
    ASSERT_EQ("R_Avg", result[39]);
    ASSERT_EQ("R_Max", result[40]);
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

    // index 12 because we skip the header
    ASSERT_EQ("42", result[12]);
    ASSERT_EQ(result.size(), 12 + 1);
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

    // index 12 because we skip the header
    ASSERT_EQ("18", result[12])
        << "Unexpected number of vertices was printed in the log.";
    ASSERT_EQ("0", result[13])
        << "Unexpected number of vertices was printed in the log.";
    ASSERT_EQ("15", result[14])
        << "Unexpected number of vertices was printed in the log.";
}

TEST_F(LABasicLoggerTest, logAfterEvaluate)
{
    std::stringstream strStr;
    Log::LABasicLogger l(*la, strStr);

    l.logAfterEvaluate(results);

    l.useUtility = true;
    l.logAfterEvaluate(results);
    std::string s = strStr.str();
    // putting each element seperated by blanks in a tab
    std::vector<std::string> result;
    std::istringstream iss(s);
    for (std::string s2; iss >> s2;)
        result.push_back(s2);

    // index 12+ because we skip the header
    ASSERT_DOUBLE_EQ(5.00, std::stod(result[12]));
    ASSERT_DOUBLE_EQ(7.50, std::stod(result[13]));
    ASSERT_DOUBLE_EQ(10.00, std::stod(result[14]));

    // Utility then reward values
    ASSERT_DOUBLE_EQ(2.00, std::stod(result[15]));
    ASSERT_DOUBLE_EQ(3.00, std::stod(result[16]));
    ASSERT_DOUBLE_EQ(4.00, std::stod(result[17]));
    ASSERT_DOUBLE_EQ(5.00, std::stod(result[18]));
    ASSERT_DOUBLE_EQ(7.50, std::stod(result[19]));
    ASSERT_DOUBLE_EQ(10.00, std::stod(result[20]));
}

TEST_F(LABasicLoggerTest, logAfterValidate)
{
    std::stringstream strStr;
    Log::LABasicLogger l(*la, strStr);

    l.logAfterValidate(results);

    auto r(results);
    r.clear();
    l.logAfterValidate(r);
    l.useUtility = true;
    l.logAfterValidate(r);

    std::string s = strStr.str();

    // putting each element seperated by blanks in a tab
    std::vector<std::string> result;
    std::istringstream iss(s);
    for (std::string s2; iss >> s2;)
        result.push_back(s2);

    // index 12+ because we skip the header
    ASSERT_DOUBLE_EQ(5.00, std::stod(result[12]));
    ASSERT_DOUBLE_EQ(7.50, std::stod(result[13]));
    ASSERT_DOUBLE_EQ(10.00, std::stod(result[14]));

    // Result size should not have increased with cleared results.
    ASSERT_EQ(result.size(), 15);
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
    l.doValidation = true;
    l.logAfterEvaluate(results);
    l.logEndOfTraining();
    // then, we can test the method when there is no validation
    l.doValidation = false;
    l.logEndOfTraining();

    std::string s = strStr.str();
    // putting each element seperated by blanks in a tab
    std::cout << s;
    std::vector<std::string> result;
    std::istringstream iss(s);
    for (std::string s2; iss >> s2;)
        result.push_back(s2);

    // index 15+ because we skip the headers and the eval stats
    double mutatTime = std::stod(result[15]);
    double evalTime = std::stod(result[16]);
    double decimTime = std::stod(result[17]);
    double validTime = std::stod(result[18]);
    double totTime = std::stod(result[19]);
    ASSERT_GE(mutatTime, 0) << "Eval duration should be positive";
    ASSERT_GE(evalTime, 0) << "Eval duration should be positive";
    ASSERT_GE(decimTime, 0) << "Valid duration should be positive";
    ASSERT_GE(validTime, 0) << "Valid duration should be positive";
    ASSERT_GE(totTime, evalTime)
        << "Total time should be the largest duration !";
    ASSERT_GE(totTime, timeToWaitMili / 1000)
        << "Total time should be larger than the time we waited !";

    // Size is headerSize (12) + log size (8 + 4)
    ASSERT_EQ(result.size(), 12 + 12) << "logEndOfTraining with and without "
                                         "valid should have 8+4=12 elements";
}
