/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2020 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2020 - 2021)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
 * Pierre-Yves Le Rolland-Raumer <plerolla@insa-rennes.fr> (2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2024 - 2025)
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

#include "../lib/JsonCpp/json.h"
#include "file/parametersParser.h"

TEST(LearningParametersTest, readConfigFile)
{
    Json::Value root;

    // name validity
    ASSERT_THROW(File::ParametersParser::readConfigFile(
                     TESTS_DAT_PATH "non_existing_file.json", root),
                 Json::Exception)
        << "An exception should be raised if file doesn't exist";
    ASSERT_NO_THROW(File::ParametersParser::readConfigFile(
        TESTS_DAT_PATH "params.json", root))
        << "An exception is raised in spite of existing file";

    // content validity
    File::ParametersParser::readConfigFile(
        TESTS_DAT_PATH "paramsNotConform.json", root);
    ASSERT_EQ(0, root.size())
        << "Ill-formed parameters file should result in no root filling";

    File::ParametersParser::readConfigFile(TESTS_DAT_PATH "params.json", root);
    ASSERT_EQ(15, root.size())
        << "Wrong number of elements in parsed json file";
    ASSERT_EQ(9, root["mutation"]["prog"].size())
        << "Wrong number of elements in parsed json file";
    ASSERT_EQ(22, root["mutation"]["tpg"].size())
        << "Wrong number of elements in parsed json file";
    ASSERT_EQ(3, root["selection"].size())
        << "Wrong number of elements in parsed json file";
    ASSERT_EQ(3, root["selection"]["tournament"].size())
        << "Wrong number of elements in parsed json file";
    ASSERT_EQ(1, root["selection"]["truncation"].size())
        << "Wrong number of elements in parsed json file";
}

TEST(LearningParametersTest, setParameterFromString)
{
    Learn::LearningParameters params;
    ASSERT_EQ(params.algorithm.lgp.nbRegisters, 8);
    std::string key = "nbRegisters";
    Json::Value v(5);
    File::ParametersParser::setParameterFromString(params, key, v);
    ASSERT_EQ(params.algorithm.lgp.nbRegisters, 5);
}

TEST(LearningParametersTest, setAllParamsFrom)
{
    Learn::LearningParameters params;
    Json::Value root;

    File::ParametersParser::readConfigFile(TESTS_DAT_PATH "params.json", root);
    ASSERT_NO_THROW(File::ParametersParser::setAllParamsFrom(root, params));

    ASSERT_EQ(50, params.nbIterationsPerPolicyEvaluation);
    ASSERT_EQ(50, params.nbIterationsPerPolicyValidation);
    ASSERT_EQ(4, params.stepValidation);
    ASSERT_EQ(5, params.maxNbActionsPerEval);
    ASSERT_EQ(100, params.maxNbEvaluationPerPolicy);
    ASSERT_EQ(2.0, params.nbThreads);
    ASSERT_EQ(200, params.nbGenerations);
    ASSERT_EQ(true, params.doValidation);

    ASSERT_EQ("none", params.algorithm.activationFunction);
    ASSERT_EQ(100, params.algorithm.nbAgents);

    ASSERT_EQ(50, params.algorithm.tpg.archiveSize);
    ASSERT_EQ(0.5, params.algorithm.tpg.archivingProbability);
    ASSERT_EQ(3, params.algorithm.tpg.maxInitOutgoingEdges);
    ASSERT_EQ(60, params.algorithm.tpg.maxOutgoingEdges);
    ASSERT_EQ(0.8, params.algorithm.tpg.pEdgeDeletion);
    ASSERT_EQ(0.8, params.algorithm.tpg.pEdgeAddition);
    ASSERT_EQ(0.8, params.algorithm.tpg.pProgramMutation);
    ASSERT_TRUE(params.algorithm.tpg.forceProgramBehaviorChangeOnMutation);
    ASSERT_EQ(0.3, params.algorithm.tpg.pEdgeDestinationChange);
    ASSERT_EQ(0.6, params.algorithm.tpg.pEdgeDestinationIsAction);

    ASSERT_EQ(0.5, params.algorithm.atpg.probaContextOverActionProgram);

    ASSERT_EQ(0.3, params.algorithm.maple.pChangeActionClass);
    ASSERT_EQ(0.3, params.algorithm.maple.pActionEdgeDeletion);
    ASSERT_EQ(0.4, params.algorithm.maple.pActionEdgeAddition);
    ASSERT_EQ(0.5, params.algorithm.maple.pMutateActionProgram);
    ASSERT_EQ(0.9, params.algorithm.maple.pSwapActionProgram);
    ASSERT_EQ(3, params.algorithm.maple.nbActionEdgeInit);
    ASSERT_EQ(0.2, params.algorithm.maple.pCrossAgents);
    ASSERT_EQ(0.5, params.algorithm.maple.pCrossPrograms);

    ASSERT_EQ(3.0, params.algorithm.lgp.nbRegisters);
    ASSERT_EQ(5, params.algorithm.lgp.nbProgramConstant);
    ASSERT_EQ(40, params.algorithm.lgp.maxProgramSize);
    ASSERT_EQ(0.0, params.algorithm.lgp.pNewProgram);
    ASSERT_EQ(0.7, params.algorithm.lgp.pDelete);
    ASSERT_EQ(0.7, params.algorithm.lgp.pAdd);
    ASSERT_EQ(1.0, params.algorithm.lgp.pMutate);
    ASSERT_EQ(1.0, params.algorithm.lgp.pSwap);
    ASSERT_EQ(0.5, params.algorithm.lgp.pConstantMutation);
    ASSERT_EQ(-10, params.algorithm.lgp.minConstValue);
    ASSERT_EQ(10, params.algorithm.lgp.maxConstValue);

    ASSERT_EQ("tournament", params.selection._selectionMode);
    ASSERT_EQ(0.85, params.selection.truncation.ratioDeletedRoots);
    ASSERT_EQ(0.15, params.selection.tournament.ratioSavedRoots);
    ASSERT_EQ(3, params.selection.tournament.sizeTournament);
    ASSERT_EQ(true, params.selection.tournament.areElitesReproductible);

    // check default parameters
    Learn::LearningParameters params2;

    File::ParametersParser::readConfigFile(
        TESTS_DAT_PATH "paramsWithWrongOne.json", root);
    File::ParametersParser::setAllParamsFrom(root, params2);

    File::ParametersParser::readConfigFile(
        TESTS_DAT_PATH "paramsWithWrongOne.json", root);
    File::ParametersParser::setAllParamsFrom(root, params2);

    ASSERT_TRUE(params2.nbThreads > 0)
        << "A default nbThreads value should be set when no one is specified";
    ASSERT_EQ(params2.doValidation, false)
        << "Default validation should be false";
    ASSERT_EQ(params2.algorithm.lgp.nbRegisters, 8) << "Bad parameter should be ignored";
}

TEST(LearningParametersTest, loadParametersFromJson)
{
    Learn::LearningParameters params;
    ASSERT_NO_THROW(File::ParametersParser::loadParametersFromJson(
        TESTS_DAT_PATH "params.json", params));
    // only testing 1 parameter as readConfigFile was already tested
    ASSERT_EQ(params.algorithm.lgp.nbRegisters, 3.0)
        << "There should be 3 registers according to the params file";
}

TEST(LearningParametersTest, writeParametersToJson)
{
    Learn::LearningParameters params;
    // Load from file
    File::ParametersParser::loadParametersFromJson(TESTS_DAT_PATH "params.json",
                                                   params);

    // Write to file
    ASSERT_NO_THROW(File::ParametersParser::writeParametersToJson(
        "current_params.json", params))
        << "Failure while writing parameters to the file.";

    // Re-parse the written file
    Learn::LearningParameters params2;
    File::ParametersParser::loadParametersFromJson("current_params.json",
                                                   params2);

    // Check equality
    // Base parameters
    ASSERT_EQ(params.doValidation, params2.doValidation);
    ASSERT_EQ(params.maxNbActionsPerEval, params2.maxNbActionsPerEval);
    ASSERT_EQ(params.maxNbEvaluationPerPolicy,
              params2.maxNbEvaluationPerPolicy);
    ASSERT_EQ(params.nbGenerations, params2.nbGenerations);
    ASSERT_EQ(params.nbIterationsPerPolicyEvaluation,
              params2.nbIterationsPerPolicyEvaluation);
    ASSERT_EQ(params.nbIterationsPerPolicyValidation,
              params2.nbIterationsPerPolicyValidation);
    ASSERT_EQ(params.stepValidation, params2.stepValidation);
    ASSERT_EQ(params.nbThreads, params2.nbThreads);

    // Mutation lgp parameters
    ASSERT_EQ(params.algorithm.lgp.nbProgramConstant, params2.algorithm.lgp.nbProgramConstant);
    ASSERT_EQ(params.algorithm.lgp.nbRegisters, params2.algorithm.lgp.nbRegisters);
    ASSERT_EQ(params.algorithm.lgp.maxConstValue,
              params2.algorithm.lgp.maxConstValue);
    ASSERT_EQ(params.algorithm.lgp.maxProgramSize,
              params2.algorithm.lgp.maxProgramSize);
    ASSERT_EQ(params.algorithm.lgp.minConstValue,
              params2.algorithm.lgp.minConstValue);
    ASSERT_EQ(params.algorithm.lgp.pAdd, params2.algorithm.lgp.pAdd);
    ASSERT_EQ(params.algorithm.lgp.pConstantMutation,
              params2.algorithm.lgp.pConstantMutation);
    ASSERT_EQ(params.algorithm.lgp.pDelete, params2.algorithm.lgp.pDelete);
    ASSERT_EQ(params.algorithm.lgp.pMutate, params2.algorithm.lgp.pMutate);
    ASSERT_EQ(params.algorithm.lgp.pSwap, params2.algorithm.lgp.pSwap);

    // Mutation parameters tpg
    ASSERT_EQ(params.algorithm.activationFunction, params2.algorithm.activationFunction);
    ASSERT_EQ(params.algorithm.nbAgents, params2.algorithm.nbAgents);

    ASSERT_EQ(params.algorithm.tpg.archiveSize, params2.algorithm.tpg.archiveSize);
    ASSERT_EQ(params.algorithm.tpg.archivingProbability, params2.algorithm.tpg.archivingProbability);
    ASSERT_EQ(params.algorithm.tpg.forceProgramBehaviorChangeOnMutation,
              params2.algorithm.tpg.forceProgramBehaviorChangeOnMutation);
    ASSERT_EQ(params.algorithm.tpg.maxInitOutgoingEdges,
              params2.algorithm.tpg.maxInitOutgoingEdges);
    ASSERT_EQ(params.algorithm.tpg.maxOutgoingEdges,
              params2.algorithm.tpg.maxOutgoingEdges);
    ASSERT_EQ(params.algorithm.tpg.pEdgeAddition,
              params2.algorithm.tpg.pEdgeAddition);
    ASSERT_EQ(params.algorithm.tpg.pEdgeDeletion,
              params2.algorithm.tpg.pEdgeDeletion);
    ASSERT_EQ(params.algorithm.tpg.pEdgeDestinationChange,
              params2.algorithm.tpg.pEdgeDestinationChange);
    ASSERT_EQ(params.algorithm.tpg.pEdgeDestinationIsAction,
              params2.algorithm.tpg.pEdgeDestinationIsAction);
    ASSERT_EQ(params.algorithm.tpg.pProgramMutation,
              params2.algorithm.tpg.pProgramMutation);

    ASSERT_EQ(params.algorithm.atpg.probaContextOverActionProgram,
              params2.algorithm.atpg.probaContextOverActionProgram);
              
    ASSERT_EQ(params.algorithm.maple.pChangeActionClass,
              params2.algorithm.maple.pChangeActionClass);
    ASSERT_EQ(params.algorithm.maple.pActionEdgeAddition,
              params2.algorithm.maple.pActionEdgeAddition);
    ASSERT_EQ(params.algorithm.maple.pActionEdgeDeletion,
              params2.algorithm.maple.pActionEdgeDeletion);
    ASSERT_EQ(params.algorithm.maple.pMutateActionProgram,
              params2.algorithm.maple.pMutateActionProgram);
    ASSERT_EQ(params.algorithm.maple.pSwapActionProgram,
              params2.algorithm.maple.pSwapActionProgram);
    ASSERT_EQ(params.algorithm.maple.nbActionEdgeInit,
              params2.algorithm.maple.nbActionEdgeInit);
    ASSERT_EQ(params.algorithm.maple.pCrossAgents,
              params2.algorithm.maple.pCrossAgents);
    ASSERT_EQ(params.algorithm.maple.pCrossPrograms,
              params2.algorithm.maple.pCrossPrograms);

    // Selection parameters
    ASSERT_EQ(params.selection._selectionMode,
              params2.selection._selectionMode);
    ASSERT_EQ(params.selection.tournament.areElitesReproductible,
              params2.selection.tournament.areElitesReproductible);
    ASSERT_EQ(params.selection.tournament.ratioSavedRoots,
              params2.selection.tournament.ratioSavedRoots);
    ASSERT_EQ(params.selection.tournament.sizeTournament,
              params2.selection.tournament.sizeTournament);
    ASSERT_EQ(params.selection.truncation.ratioDeletedRoots,
              params2.selection.truncation.ratioDeletedRoots);
}
