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
#include "parameters.h"

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
    Parameters params;
    ASSERT_EQ(params.representation.lgp.nbRegisters, 8);
    std::string key = "nbRegisters";
    Json::Value v(5);
    File::ParametersParser::setParameterFromString(params, key, v);
    ASSERT_EQ(params.representation.lgp.nbRegisters, 5);
}

TEST(LearningParametersTest, setAllParamsFrom)
{
    Parameters params;
    Json::Value root;

    File::ParametersParser::readConfigFile(TESTS_DAT_PATH "params.json", root);
    ASSERT_NO_THROW(File::ParametersParser::setAllParamsFrom(root, params));

    ASSERT_EQ(50, params.evaluation.nbIterationsPerPolicyEvaluation);
    ASSERT_EQ(50, params.evaluation.nbIterationsPerPolicyValidation);
    ASSERT_EQ(4, params.evaluation.stepValidation);
    ASSERT_EQ(5, params.evaluation.maxNbActionsPerEval);
    ASSERT_EQ(100, params.evaluation.maxNbEvaluationPerPolicy);
    ASSERT_EQ(2.0, params.evaluation.nbThreads);
    ASSERT_EQ(200, params.evaluation.nbGenerations);
    ASSERT_EQ(true, params.evaluation.doValidation);

    ASSERT_EQ("none", params.representation.activationFunction);
    ASSERT_EQ(100, params.representation.nbAgents);

    ASSERT_EQ(50, params.representation.tpg.archiveSize);
    ASSERT_EQ(0.5, params.representation.tpg.archivingProbability);
    ASSERT_EQ(3, params.representation.tpg.maxInitOutgoingEdges);
    ASSERT_EQ(60, params.representation.tpg.maxOutgoingEdges);
    ASSERT_EQ(0.8, params.representation.tpg.pEdgeDeletion);
    ASSERT_EQ(0.8, params.representation.tpg.pEdgeAddition);
    ASSERT_EQ(0.8, params.representation.tpg.pProgramMutation);
    ASSERT_TRUE(params.representation.lgp.forceProgramBehaviorChangeOnMutation);
    ASSERT_EQ(0.3, params.representation.tpg.pEdgeDestinationChange);
    ASSERT_EQ(0.6, params.representation.tpg.pEdgeDestinationIsAction);

    ASSERT_EQ(0.5, params.representation.atpg.probaContextOverActionProgram);

    ASSERT_EQ(0.3, params.representation.maple.pChangeActionClass);
    ASSERT_EQ(0.3, params.representation.maple.pActionEdgeDeletion);
    ASSERT_EQ(0.4, params.representation.maple.pActionEdgeAddition);
    ASSERT_EQ(0.5, params.representation.maple.pMutateActionProgram);
    ASSERT_EQ(0.9, params.representation.maple.pSwapActionProgram);
    ASSERT_EQ(3, params.representation.maple.nbActionEdgeInit);
    ASSERT_EQ(0.2, params.representation.maple.pCrossAgents);
    ASSERT_EQ(0.5, params.representation.maple.pCrossPrograms);

    ASSERT_EQ(3.0, params.representation.lgp.nbRegisters);
    ASSERT_EQ(5, params.representation.lgp.nbProgramConstant);
    ASSERT_EQ(40, params.representation.lgp.maxProgramSize);
    ASSERT_EQ(0.0, params.representation.lgp.pNewProgram);
    ASSERT_EQ(0.7, params.representation.lgp.pDelete);
    ASSERT_EQ(0.7, params.representation.lgp.pAdd);
    ASSERT_EQ(1.0, params.representation.lgp.pMutate);
    ASSERT_EQ(1.0, params.representation.lgp.pSwap);
    ASSERT_EQ(0.5, params.representation.lgp.pConstantMutation);
    ASSERT_EQ(-10, params.representation.lgp.minConstValue);
    ASSERT_EQ(10, params.representation.lgp.maxConstValue);

    ASSERT_EQ(0, params.representation.cgp.nbLayers);
    ASSERT_EQ(0, params.representation.cgp.nbNodesPerLayer);
    ASSERT_EQ(0, params.representation.cgp.pMutateNode);

    ASSERT_EQ(0, params.representation.tgp.maxDepth);
    ASSERT_EQ(0, params.representation.tgp.maxInitDepth);
    ASSERT_EQ(0, params.representation.tgp.maxNbEdgePerNode);

    ASSERT_EQ("tournament", params.selection._selectionMode);
    ASSERT_EQ(0.85, params.selection.truncation.ratioDeletedRoots);
    ASSERT_EQ(0.15, params.selection.tournament.ratioSavedRoots);
    ASSERT_EQ(3, params.selection.tournament.sizeTournament);
    ASSERT_EQ(true, params.selection.tournament.areElitesReproductible);

    // check default parameters
    Parameters params2;

    File::ParametersParser::readConfigFile(
        TESTS_DAT_PATH "paramsWithWrongOne.json", root);
    File::ParametersParser::setAllParamsFrom(root, params2);

    File::ParametersParser::readConfigFile(
        TESTS_DAT_PATH "paramsWithWrongOne.json", root);
    File::ParametersParser::setAllParamsFrom(root, params2);

    ASSERT_TRUE(params2.evaluation.nbThreads > 0)
        << "A default nbThreads value should be set when no one is specified";
    ASSERT_EQ(params2.evaluation.doValidation, false)
        << "Default validation should be false";
    ASSERT_EQ(params2.representation.lgp.nbRegisters, 8) << "Bad parameter should be ignored";
}

TEST(LearningParametersTest, loadParametersFromJson)
{
    Parameters params;
    ASSERT_NO_THROW(File::ParametersParser::loadParametersFromJson(
        TESTS_DAT_PATH "params.json", params));
    // only testing 1 parameter as readConfigFile was already tested
    ASSERT_EQ(params.representation.lgp.nbRegisters, 3.0)
        << "There should be 3 registers according to the params file";
}

TEST(LearningParametersTest, writeParametersToJson)
{
    Parameters params;
    // Load from file
    File::ParametersParser::loadParametersFromJson(TESTS_DAT_PATH "params.json",
                                                   params);

    // Write to file
    ASSERT_NO_THROW(File::ParametersParser::writeParametersToJson(
        "current_params.json", params))
        << "Failure while writing parameters to the file.";

    // Re-parse the written file
    Parameters params2;
    File::ParametersParser::loadParametersFromJson("current_params.json",
                                                   params2);

    // Check equality
    // Base parameters
    ASSERT_EQ(params.evaluation.doValidation, params2.evaluation.doValidation);
    ASSERT_EQ(params.evaluation.maxNbActionsPerEval, params2.evaluation.maxNbActionsPerEval);
    ASSERT_EQ(params.evaluation.maxNbEvaluationPerPolicy,
              params2.evaluation.maxNbEvaluationPerPolicy);
    ASSERT_EQ(params.evaluation.nbGenerations, params2.evaluation.nbGenerations);
    ASSERT_EQ(params.evaluation.nbIterationsPerPolicyEvaluation,
              params2.evaluation.nbIterationsPerPolicyEvaluation);
    ASSERT_EQ(params.evaluation.nbIterationsPerPolicyValidation,
              params2.evaluation.nbIterationsPerPolicyValidation);
    ASSERT_EQ(params.evaluation.stepValidation, params2.evaluation.stepValidation);
    ASSERT_EQ(params.evaluation.nbThreads, params2.evaluation.nbThreads);

    // Mutation lgp parameters
    ASSERT_EQ(params.representation.lgp.nbProgramConstant, params2.representation.lgp.nbProgramConstant);
    ASSERT_EQ(params.representation.lgp.nbRegisters, params2.representation.lgp.nbRegisters);
    ASSERT_EQ(params.representation.lgp.maxConstValue,
              params2.representation.lgp.maxConstValue);
    ASSERT_EQ(params.representation.lgp.maxProgramSize,
              params2.representation.lgp.maxProgramSize);
    ASSERT_EQ(params.representation.lgp.minConstValue,
              params2.representation.lgp.minConstValue);
    ASSERT_EQ(params.representation.lgp.pAdd, params2.representation.lgp.pAdd);
    ASSERT_EQ(params.representation.lgp.pConstantMutation,
              params2.representation.lgp.pConstantMutation);
    ASSERT_EQ(params.representation.lgp.pDelete, params2.representation.lgp.pDelete);
    ASSERT_EQ(params.representation.lgp.pMutate, params2.representation.lgp.pMutate);
    ASSERT_EQ(params.representation.lgp.pSwap, params2.representation.lgp.pSwap);

    ASSERT_EQ(params.representation.cgp.nbLayers, params2.representation.cgp.nbLayers);
    ASSERT_EQ(params.representation.cgp.nbNodesPerLayer, params2.representation.cgp.nbNodesPerLayer);
    ASSERT_EQ(params.representation.cgp.pMutateNode, params2.representation.cgp.pMutateNode);
    
    ASSERT_EQ(params.representation.tgp.maxDepth, params2.representation.tgp.maxDepth);
    ASSERT_EQ(params.representation.tgp.maxInitDepth, params2.representation.tgp.maxInitDepth);
    ASSERT_EQ(params.representation.tgp.maxNbEdgePerNode, params2.representation.tgp.maxNbEdgePerNode);
    

    // Mutation parameters tpg
    ASSERT_EQ(params.representation.activationFunction, params2.representation.activationFunction);
    ASSERT_EQ(params.representation.nbAgents, params2.representation.nbAgents);

    ASSERT_EQ(params.representation.tpg.archiveSize, params2.representation.tpg.archiveSize);
    ASSERT_EQ(params.representation.tpg.archivingProbability, params2.representation.tpg.archivingProbability);
    ASSERT_EQ(params.representation.lgp.forceProgramBehaviorChangeOnMutation,
              params2.representation.lgp.forceProgramBehaviorChangeOnMutation);
    ASSERT_EQ(params.representation.tpg.maxInitOutgoingEdges,
              params2.representation.tpg.maxInitOutgoingEdges);
    ASSERT_EQ(params.representation.tpg.maxOutgoingEdges,
              params2.representation.tpg.maxOutgoingEdges);
    ASSERT_EQ(params.representation.tpg.pEdgeAddition,
              params2.representation.tpg.pEdgeAddition);
    ASSERT_EQ(params.representation.tpg.pEdgeDeletion,
              params2.representation.tpg.pEdgeDeletion);
    ASSERT_EQ(params.representation.tpg.pEdgeDestinationChange,
              params2.representation.tpg.pEdgeDestinationChange);
    ASSERT_EQ(params.representation.tpg.pEdgeDestinationIsAction,
              params2.representation.tpg.pEdgeDestinationIsAction);
    ASSERT_EQ(params.representation.tpg.pProgramMutation,
              params2.representation.tpg.pProgramMutation);

    ASSERT_EQ(params.representation.atpg.probaContextOverActionProgram,
              params2.representation.atpg.probaContextOverActionProgram);
              
    ASSERT_EQ(params.representation.maple.pChangeActionClass,
              params2.representation.maple.pChangeActionClass);
    ASSERT_EQ(params.representation.maple.pActionEdgeAddition,
              params2.representation.maple.pActionEdgeAddition);
    ASSERT_EQ(params.representation.maple.pActionEdgeDeletion,
              params2.representation.maple.pActionEdgeDeletion);
    ASSERT_EQ(params.representation.maple.pMutateActionProgram,
              params2.representation.maple.pMutateActionProgram);
    ASSERT_EQ(params.representation.maple.pSwapActionProgram,
              params2.representation.maple.pSwapActionProgram);
    ASSERT_EQ(params.representation.maple.nbActionEdgeInit,
              params2.representation.maple.nbActionEdgeInit);
    ASSERT_EQ(params.representation.maple.pCrossAgents,
              params2.representation.maple.pCrossAgents);
    ASSERT_EQ(params.representation.maple.pCrossPrograms,
              params2.representation.maple.pCrossPrograms);

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
