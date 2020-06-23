/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
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
#include "file/parametersParser.h"
#include "../lib/JsonCpp/json.h"

TEST(LearningParametersTest, readConfigFile) {
    Json::Value root;

    // name validity
    ASSERT_THROW(ParametersParser::readConfigFile(TESTS_DAT_PATH "non_existing_file.json", root), Json::Exception)
                                << "An exception should be raised if file doesn't exist";
    ASSERT_NO_THROW(ParametersParser::readConfigFile(TESTS_DAT_PATH "params.json", root))
                                << "An exception is raised in spite of existing file";

    // content validity
    ParametersParser::readConfigFile(TESTS_DAT_PATH "paramsNotConform.json", root);
    ASSERT_EQ(0, root.size()) << "Ill-formed parameters file should result in no root filling";

    ParametersParser::readConfigFile(TESTS_DAT_PATH "params.json", root);
    ASSERT_EQ(10, root.size()) << "Wrong number of elements in parsed json file";
    ASSERT_EQ(9, root["mutation"]["tpg"].size()) << "Wrong number of elements in parsed json file";
    ASSERT_EQ(5, root["mutation"]["prog"].size()) << "Wrong number of elements in parsed json file";
}

TEST(LearningParametersTest, setParameterFromString) {
    Learn::LearningParameters params;
    ASSERT_EQ(params.nbRegisters, 8);
    std::string key = "nbRegisters";
    ParametersParser::setParameterFromString(&params,key,5);
    ASSERT_EQ(params.nbRegisters, 5);
}


TEST(LearningParametersTest, setAllParamsFrom) {
    Learn::LearningParameters params;
    Json::Value root;

    ParametersParser::readConfigFile(TESTS_DAT_PATH "params.json", root);
    ASSERT_NO_THROW(ParametersParser::setAllParamsFrom(root, &params));

    ASSERT_EQ(50, params.archiveSize);
    ASSERT_EQ(0.5, params.archivingProbability);
    ASSERT_EQ(50, params.nbIterationsPerPolicyEvaluation);
    ASSERT_EQ(5, params.maxNbActionsPerEval);
    ASSERT_EQ(0.85, params.ratioDeletedRoots);
    ASSERT_EQ(100, params.maxNbEvaluationPerPolicy);
    ASSERT_EQ(3.0, params.nbRegisters);
    ASSERT_EQ(2.0, params.nbThreads);
    ASSERT_EQ(200, params.nbGenerations);
    ASSERT_EQ(100, params.mutation.tpg.nbRoots);
    ASSERT_EQ(5, params.mutation.tpg.nbActions);
    ASSERT_EQ(3, params.mutation.tpg.maxInitOutgoingEdges);
    ASSERT_EQ(60, params.mutation.tpg.maxOutgoingEdges);
    ASSERT_EQ(0.8, params.mutation.tpg.pEdgeDeletion);
    ASSERT_EQ(0.8, params.mutation.tpg.pEdgeAddition);
    ASSERT_EQ(0.8, params.mutation.tpg.pProgramMutation);
    ASSERT_EQ(0.3, params.mutation.tpg.pEdgeDestinationChange);
    ASSERT_EQ(0.6, params.mutation.tpg.pEdgeDestinationIsAction);
    ASSERT_EQ(40, params.mutation.prog.maxProgramSize);
    ASSERT_EQ(0.7, params.mutation.prog.pDelete);
    ASSERT_EQ(0.7, params.mutation.prog.pAdd);
    ASSERT_EQ(1.0, params.mutation.prog.pMutate);
    ASSERT_EQ(1.0, params.mutation.prog.pSwap);

    // check default parameters
    Learn::LearningParameters params2;

    ParametersParser::readConfigFile(TESTS_DAT_PATH "paramsWithWrongOne.json", root);
    ParametersParser::setAllParamsFrom(root, &params2);

    ASSERT_TRUE(params2.nbThreads > 0) << "A default nbThreads value should be set when no one is specified";
    ASSERT_EQ(params2.nbRegisters, 8) << "Bad parameter should be ignored";
}

TEST(LearningParametersTest, loadParametersFromJson) {
    Learn::LearningParameters params;
    ASSERT_NO_THROW(ParametersParser::loadParametersFromJson(TESTS_DAT_PATH "params.json", &params));
    // only testing 1 parameter as readConfigFile was already tested
    ASSERT_EQ(params.nbRegisters, 3.0) << "There should be 3 registers according to the params file";
}
