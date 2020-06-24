/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
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
#include <chrono>
#include <thread>


#include "instructions/addPrimitiveType.h"
#include "instructions/multByConstParam.h"
#include "log/LABasicLogger.h"

class LABasicLoggerTest : public ::testing::Test {
protected:
    const size_t size1{24};
    const size_t size2{32};
    std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
    Instructions::Set set;
    Environment *e = NULL;
    TPG::TPGGraph *tpg = NULL;
    uint64_t generation = 0;
    std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex *> *results =
            new std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex *>();


    void SetUp() override {
        vect.emplace_back(*(new Data::PrimitiveTypeArray<double>((unsigned int) size1)));
        vect.emplace_back(*(new Data::PrimitiveTypeArray<float>((unsigned int) size2)));

        set.add(*(new Instructions::AddPrimitiveType<float>()));
        set.add(*(new Instructions::MultByConstParam<double, float>()));

        e = new Environment(set, vect, 8);
        tpg = new TPG::TPGGraph(*e);

        auto res1 = new Learn::EvaluationResult(5, 2);
        auto res2 = new Learn::EvaluationResult(10, 2);
        auto v1(new TPG::TPGAction(0));
        auto v2(new TPG::TPGAction(0));
        results->insert(std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex *>(res1, v1));
        results->insert(std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex *>(res2, v2));
    }

    void TearDown() override {
        delete tpg;
        delete e;
        delete (&(vect.at(0).get()));
        delete (&(vect.at(1).get()));
        delete (&set.getInstruction(0));
        delete (&set.getInstruction(1));
        auto it = results->begin();
        delete it->second;
        it++;
        delete it->second;
        delete results;
    }
};


TEST_F(LABasicLoggerTest, ConstructorAndlogHeader) {
    ASSERT_NO_THROW(Log::LABasicLogger l);
    ASSERT_NO_THROW(Log::LABasicLogger l(std::cerr));

    std::stringstream strStr;
    Log::LABasicLogger l(strStr);

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
    ASSERT_EQ("Duration(eval)", result[5]);
    ASSERT_EQ("Duration(decim)", result[6]);
    ASSERT_EQ("Total_time", result[7]);
}

TEST_F(LABasicLoggerTest, logAfterPopulateTPG) {
    std::stringstream strStr;
    Log::LABasicLogger l(strStr);

    l.logAfterPopulateTPG(generation, *tpg);
    std::string s = strStr.str();
    // putting each element seperated by blanks in a tab
    std::vector<std::string> result;
    std::istringstream iss(s);
    for (std::string s2; iss >> s2;)
        result.push_back(s2);

    ASSERT_EQ("0", result[8]);
    ASSERT_EQ("0", result[9]);
}

TEST_F(LABasicLoggerTest, logAfterEvaluate) {
    std::stringstream strStr;
    Log::LABasicLogger l(strStr);

    l.logAfterEvaluate(*results);
    std::string s = strStr.str();
    // putting each element seperated by blanks in a tab
    std::vector<std::string> result;
    std::istringstream iss(s);
    for (std::string s2; iss >> s2;)
        result.push_back(s2);

    ASSERT_DOUBLE_EQ(5.00, std::stod(result[8]));
    ASSERT_DOUBLE_EQ(7.50, std::stod(result[9]));
    ASSERT_DOUBLE_EQ(10.00, std::stod(result[10]));
    ASSERT_TRUE(std::stod(result[11]) >= 0.0) << "eval duration should be positive";
}


TEST_F(LABasicLoggerTest, logAfterDecimate) {
    std::stringstream strStr;
    Log::LABasicLogger l(strStr);

    // little sleep to delay the total_time value (while the "checkpoint" of the logger will be reset)
    double timeToWaitMili = 10;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    l.logAfterDecimate(*tpg);
    std::string s = strStr.str();
    // putting each element seperated by blanks in a tab
    std::vector<std::string> result;
    std::istringstream iss(s);
    for (std::string s2; iss >> s2;)
        result.push_back(s2);

    double decimTime = std::stod(result[8]);
    double totTime = std::stod(result[9]);
    ASSERT_TRUE(decimTime >= 0.0) << "decimation duration should be positive";
    ASSERT_TRUE(totTime >= 0.0) << "total elapsed time should be positive";
}

TEST_F(LABasicLoggerTest, ChronoFromNow){
    // to test chrono, we will wait, use chronoFromNow() which resets the "checkpoint" time
    // and call logAfterDecimate() which shall print the duration from checkpoint and from start

    std::stringstream strStr;
    Log::LABasicLogger l(strStr);

    // little sleep to delay the total_time value (while the "checkpoint" of the logger will be reset)
    double timeToWaitMili = 10;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // resets "checkpoint" so that the first displayed time shall be lower than the second which is the time from start
    l.chronoFromNow();

    l.logAfterDecimate(*tpg);
    std::string s = strStr.str();
    // putting each element seperated by blanks in a tab
    std::vector<std::string> result;
    std::istringstream iss(s);
    for (std::string s2; iss >> s2;)
        result.push_back(s2);

    double decimTime = std::stod(result[8]);
    double totTime = std::stod(result[9]);
    ASSERT_TRUE(totTime > decimTime) << "Total time should be the largest duration !";
    ASSERT_TRUE(totTime >= timeToWaitMili / 1000) << "Total time should be larger than the time we waited !";

}