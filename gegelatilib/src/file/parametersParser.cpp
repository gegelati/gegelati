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

#include <fstream>
#include <iostream>
#include <json.h>

#include "file/parametersParser.h"

void File::ParametersParser::readConfigFile(const char* path, Json::Value& root)
{
    std::ifstream ifs;
    ifs.open(path);

    if (!ifs.is_open()) {
        std::cerr << "Error : specified param file doesn't exist : " << path
                  << std::endl;
        throw Json::Exception("aborting");
    }

    Json::CharReaderBuilder builder;
    builder["collectComments"] = true;
    JSONCPP_STRING errs;
    if (!parseFromStream(builder, ifs, &root, &errs)) {
        std::cout << errs << std::endl;
        std::cerr << "Ignoring ill-formed config file " << path << std::endl;
    }
}

void File::ParametersParser::setAllParamsFrom(const Json::Value& root,
                                              Learn::LearningParameters& params)
{
    for (std::string const& key : root.getMemberNames()) {
        if (key == "mutation") {
            // we have a subtree of mutation : parameters like mutation.xxx.xxx
            for (std::string const& key2 : root[key].getMemberNames()) {
                if (key2 == "tpg") {
                    // we're on a mutation.tpg.xxx parameter
                    for (std::string const& key3 :
                         root[key][key2].getMemberNames()) {
                        Json::Value value = root[key][key2][key3];
                        setParameterFromString(params, key3, value);
                    }
                }
                else {
                    if (key2 == "prog") {
                        // we're on a mutation.prog.xxx parameter
                        for (std::string const& key3 :
                             root[key][key2].getMemberNames()) {
                            Json::Value value = root[key][key2][key3];
                            setParameterFromString(params, key3, value);
                        }
                    }
                }
            }
        }
        else if (key == "selection") {
            // we have a subtree of selection : parameters like
            // selection.xxx.xxx
            for (std::string const& key2 : root[key].getMemberNames()) {
                if (key2 == "truncation") {
                    // we're on a selection.truncation.xxx parameter
                    for (std::string const& key3 :
                         root[key][key2].getMemberNames()) {
                        Json::Value value = root[key][key2][key3];
                        setParameterFromString(params, key3, value);
                    }
                }
                else if (key2 == "tournament") {
                    // we're on a selection.tournament.xxx parameter
                    for (std::string const& key3 :
                         root[key][key2].getMemberNames()) {
                        Json::Value value = root[key][key2][key3];
                        setParameterFromString(params, key3, value);
                    }
                }
                if (root[key][key2].size() == 0) {
                    // we're on a selection.xxx paramter.
                    Json::Value value = root[key][key2];
                    setParameterFromString(params, key2, value);
                }
            }
        }
        if (root[key].size() == 0) {
            // we have a parameter without subtree (as a leaf)
            Json::Value value = root[key];
            setParameterFromString(params, key, value);
        }
    }
}

void File::ParametersParser::setParameterFromString(
    Learn::LearningParameters& params, const std::string& param,
    Json::Value const& value)
{

    if (param == "activationFunction") {
        params.activationFunction = value.asString();
        return;
    }
    if (param == "archiveSize") {
        params.archiveSize = (size_t)value.asUInt();
        return;
    }
    if (param == "archivingProbability") {
        params.archivingProbability = value.asDouble();
        return;
    }
    if (param == "doValidation") {
        params.doValidation = value.asBool();
        return;
    }
    if (param == "nbIterationsPerPolicyEvaluation") {
        params.nbIterationsPerPolicyEvaluation = value.asUInt64();
        return;
    }
    if (param == "nbIterationsPerPolicyValidation") {
        params.nbIterationsPerPolicyValidation = value.asUInt64();
        return;
    }
    if (param == "nbRegisters") {
        params.nbRegisters = (size_t)value.asUInt();
        return;
    }
    if (param == "nbThreads") {
        params.nbThreads = (size_t)value.asUInt();
        return;
    }
    if (param == "nbGenerations") {
        params.nbGenerations = value.asUInt64();
        return;
    }
    if (param == "maxNbActionsPerEval") {
        params.maxNbActionsPerEval = value.asUInt64();
        return;
    }
    if (param == "maxNbEvaluationPerPolicy") {
        params.maxNbEvaluationPerPolicy = (size_t)value.asUInt();
        return;
    }
    if (param == "stepValidation") {
        params.stepValidation = (size_t)value.asUInt();
        return;
    }

    if (param == "nbRoots") {
        params.mutation.tpg.nbRoots = (size_t)value.asUInt();
        return;
    }
    if (param == "ratioTeamsOverActions") {
        params.mutation.tpg.ratioTeamsOverActions = (double)value.asDouble();
        return;
    }
    if (param == "maxInitOutgoingEdges") {
        params.mutation.tpg.maxInitOutgoingEdges = (size_t)value.asUInt();
        return;
    }
    if (param == "nbActionEdgeInit") {
        params.mutation.tpg.nbActionEdgeInit = (size_t)value.asUInt();
        return;
    }
    if (param == "useMultiActionProgram") {
        params.mutation.tpg.useMultiActionProgram = value.asBool();
        return;
    }
    if (param == "teamAccessAllActions") {
        params.mutation.tpg.teamAccessAllActions = value.asBool();
        return;
    }
    if (param == "pChangeActionClass") {
        params.mutation.tpg.pChangeActionClass = (double)value.asDouble();
        return;
    }
    if (param == "pActionEdgeDeletion") {
        params.mutation.tpg.pActionEdgeDeletion = value.asDouble();
        return;
    }
    if (param == "pActionEdgeAddition") {
        params.mutation.tpg.pActionEdgeAddition = value.asDouble();
        return;
    }
    if (param == "pMutateActionProgram") {
        params.mutation.tpg.pMutateActionProgram = value.asDouble();
        return;
    }
    if (param == "pSwapActionProgram") {
        params.mutation.tpg.pSwapActionProgram = value.asDouble();
        return;
    }
    if (param == "maxOutgoingEdges") {
        params.mutation.tpg.maxOutgoingEdges = (size_t)value.asUInt();
        return;
    }
    if (param == "pEdgeDeletion") {
        params.mutation.tpg.pEdgeDeletion = value.asDouble();
        return;
    }
    if (param == "pEdgeAddition") {
        params.mutation.tpg.pEdgeAddition = value.asDouble();
        return;
    }
    if (param == "pProgramMutation") {
        params.mutation.tpg.pProgramMutation = value.asDouble();
        return;
    }
    if (param == "forceProgramBehaviorChangeOnMutation") {
        params.mutation.tpg.forceProgramBehaviorChangeOnMutation =
            value.asBool();
        return;
    }
    if (param == "pEdgeDestinationChange") {
        params.mutation.tpg.pEdgeDestinationChange = value.asDouble();
        return;
    }
    if (param == "pEdgeDestinationIsAction") {
        params.mutation.tpg.pEdgeDestinationIsAction = value.asDouble();
        return;
    }
    if (param == "useActionProgram") {
        params.mutation.tpg.useActionProgram = value.asBool();
        return;
    }
    if (param == "probaContextOverActionProgram") {
        params.mutation.tpg.probaContextOverActionProgram =
            (double)value.asDouble();
        return;
    }
    if (param == "pCrossAgents") {
        params.mutation.tpg.pCrossAgents = (double)value.asDouble();
        return;
    }
    if (param == "pCrossPrograms") {
        params.mutation.tpg.pCrossPrograms = (double)value.asDouble();
        return;
    }

    if (param == "nbBestAgents") {
        params.mutation.tpg.nbBestAgents = (size_t)value.asUInt();
        return;
    }
    if (param == "nbLastGenPlateau") {
        params.mutation.tpg.nbLastGenPlateau = (size_t)value.asUInt();
        return;
    }
    if (param == "alphaEMALinearFactor") {
        params.mutation.tpg.alphaEMALinearFactor = (double)value.asDouble();
        return;
    }
    if (param == "epsilonPlateauThreshold") {
        params.mutation.tpg.epsilonPlateauThreshold = (double)value.asDouble();
        return;
    }
    if (param == "speciesProbaDupplicateNextVertex") {
        params.mutation.tpg.speciesProbaDupplicateNextVertex = (double)value.asDouble();
        return;
    }
    if (param == "speciesMutateSpecificFirst") {
        params.mutation.tpg.speciesMutateSpecificFirst = value.asBool();
        return;
    }
    if (param == "speciesProbaAddContext") {
        params.mutation.tpg.speciesProbaAddContext = (double)value.asDouble();
        return;
    }
    if (param == "speciesProbaAddActivation") {
        params.mutation.tpg.speciesProbaAddActivation = (double)value.asDouble();
        return;
    }
    if (param == "speciesProbaDelete") {
        params.mutation.tpg.speciesProbaDelete = (double)value.asDouble();
        return;
    }
    if (param == "speciesProbaExtension") {
        params.mutation.tpg.speciesProbaExtension = (double)value.asDouble();
        return;
    }
    if (param == "speciesProbaCrossProgram") {
        params.mutation.tpg.speciesProbaCrossProgram = (double)value.asDouble();
        return;
    }
    if (param == "speciesProbaCrossAgent") {
        params.mutation.tpg.speciesProbaCrossAgent = (double)value.asDouble();
        return;
    }
    if (param == "speciesProbaSwapContext") {
        params.mutation.tpg.speciesProbaSwapContext = (double)value.asDouble();
        return;
    }
    if (param == "speciesProbaSwapActionValues") {
        params.mutation.tpg.speciesProbaSwapActionValues = (double)value.asDouble();
        return;
    }
    if (param == "speciesProbaSwapPrograms") {
        params.mutation.tpg.speciesProbaSwapPrograms = (double)value.asDouble();
        return;
    }
    if (param == "speciesProbaMutateProgram") {
        params.mutation.tpg.speciesProbaMutateProgram = (double)value.asDouble();
        return;
    }




    if (param == "maxProgramSize") {
        params.mutation.prog.maxProgramSize = (size_t)value.asUInt();
        return;
    }
    if (param == "initMinProgramSize") {
        params.mutation.prog.initMinProgramSize = (size_t)value.asUInt();
        return;
    }
    if (param == "initMaxProgramSize") {
        params.mutation.prog.initMaxProgramSize = (size_t)value.asUInt();
        return;
    }
    if (param == "pDelete") {
        params.mutation.prog.pDelete = value.asDouble();
        return;
    }
    if (param == "pAdd") {
        params.mutation.prog.pAdd = value.asDouble();
        return;
    }
    if (param == "pMutate") {
        params.mutation.prog.pMutate = value.asDouble();
        return;
    }
    if (param == "pSwap") {
        params.mutation.prog.pSwap = value.asDouble();
        return;
    }
    if (param == "nbProgramConstant") {
        params.nbProgramConstant = (size_t)value.asUInt();
        return;
    }
    if (param == "pConstantMutation") {
        params.mutation.prog.pConstantMutation = value.asDouble();
        return;
    }
    if (param == "pNewProgram") {
        params.mutation.prog.pNewProgram = value.asDouble();
        return;
    }
    if (param == "minConstValue") {
        params.mutation.prog.minConstValue = value.asDouble();
        return;
    }
    if (param == "maxConstValue") {
        params.mutation.prog.maxConstValue = value.asDouble();
        return;
    }

    if (param == "_selectionMode") {
        params.selection._selectionMode = value.asString();
        return;
    }
    if (param == "ratioDeletedRoots") {
        params.selection.truncation.ratioDeletedRoots = value.asDouble();
        return;
    }
    if (param == "ratioSavedRoots") {
        params.selection.tournament.ratioSavedRoots = value.asDouble();
        return;
    }
    if (param == "sizeTournament") {
        params.selection.tournament.sizeTournament = value.asUInt64();
        return;
    }
    if (param == "areElitesReproductible") {
        params.selection.tournament.areElitesReproductible = value.asBool();
        return;
    }

    // we didn't recognize the symbol
    std::cerr << "Ignoring unknown parameter " << param << std::endl;
}

void File::ParametersParser::loadParametersFromJson(
    const char* path, Learn::LearningParameters& params)
{
    Json::Value root;
    readConfigFile(path, root);

    setAllParamsFrom(root, params);
}

void File::ParametersParser::writeParametersToJson(
    const char* path, const Learn::LearningParameters& params)
{
    Learn::LearningParameters defaultParams;
    // Create the JSON Structure
    Json::Value root;

    // Fill it
    // base parameters
    root["archiveSize"] = params.archiveSize;
    root["archiveSize"].setComment(
        Learn::LearningParameters::archiveSizeComment, Json::commentBefore);

    root["archivingProbability"] = params.archivingProbability;
    root["archivingProbability"].setComment(
        Learn::LearningParameters::archivingProbabilityComment,
        Json::commentBefore);

    root["doValidation"] = params.doValidation;
    root["doValidation"].setComment(
        Learn::LearningParameters::doValidationComment, Json::commentBefore);

    root["activationFunction"] = params.activationFunction;
    root["activationFunction"].setComment(
        Learn::LearningParameters::activationFunctionComment,
        Json::commentBefore);

    root["maxNbActionsPerEval"] = params.maxNbActionsPerEval;
    root["maxNbActionsPerEval"].setComment(
        Learn::LearningParameters::maxNbActionsPerEvalComment,
        Json::commentBefore);

    root["maxNbEvaluationPerPolicy"] = params.maxNbEvaluationPerPolicy;
    root["maxNbEvaluationPerPolicy"].setComment(
        Learn::LearningParameters::maxNbEvaluationPerPolicyComment,
        Json::commentBefore);

    root["nbGenerations"] = params.nbGenerations;
    root["nbGenerations"].setComment(
        Learn::LearningParameters::nbGenerationsComment, Json::commentBefore);

    root["nbIterationsPerPolicyEvaluation"] =
        params.nbIterationsPerPolicyEvaluation;
    root["nbIterationsPerPolicyEvaluation"].setComment(
        Learn::LearningParameters::nbIterationsPerPolicyEvaluationComment,
        Json::commentBefore);
    root["nbIterationsPerPolicyValidation"] =
        params.nbIterationsPerPolicyValidation;
    root["nbIterationsPerPolicyValidation"].setComment(
        Learn::LearningParameters::nbIterationsPerPolicyValidationComment,
        Json::commentBefore);
    root["stepValidation"] = params.stepValidation;
    root["stepValidation"].setComment(
        Learn::LearningParameters::stepValidationComment, Json::commentBefore);

    root["nbProgramConstant"] = params.nbProgramConstant;
    root["nbProgramConstant"].setComment(
        Learn::LearningParameters::nbProgramConstantComment,
        Json::commentBefore);

    root["nbRegisters"] = params.nbRegisters;
    root["nbRegisters"].setComment(
        Learn::LearningParameters::nbRegistersComment, Json::commentBefore);

    root["nbThreads"] = params.nbThreads;
    root["nbThreads"].setComment(Learn::LearningParameters::nbThreadsComment,
                                 Json::commentBefore);

    // Mutation.tpg parameters
    root["mutation"]["tpg"]["forceProgramBehaviorChangeOnMutation"] =
        params.mutation.tpg.forceProgramBehaviorChangeOnMutation;
    root["mutation"]["tpg"]["forceProgramBehaviorChangeOnMutation"].setComment(
        Mutator::TPGParameters::forceProgramBehaviorChangeOnMutationComment,
        Json::commentBefore);

    root["mutation"]["tpg"]["maxInitOutgoingEdges"] =
        params.mutation.tpg.maxInitOutgoingEdges;
    root["mutation"]["tpg"]["maxInitOutgoingEdges"].setComment(
        Mutator::TPGParameters::maxInitOutgoingEdgesComment,
        Json::commentBefore);

    root["mutation"]["tpg"]["nbActionEdgeInit"] =
        params.mutation.tpg.nbActionEdgeInit;
    root["mutation"]["tpg"]["nbActionEdgeInit"].setComment(
        Mutator::TPGParameters::nbActionEdgeInitComment, Json::commentBefore);

    root["mutation"]["tpg"]["useMultiActionProgram"] =
        params.mutation.tpg.useMultiActionProgram;
    root["mutation"]["tpg"]["useMultiActionProgram"].setComment(
        Mutator::TPGParameters::useMultiActionProgramComment,
        Json::commentBefore);
    root["mutation"]["tpg"]["teamAccessAllActions"] =
        params.mutation.tpg.teamAccessAllActions;
    root["mutation"]["tpg"]["teamAccessAllActions"].setComment(
        Mutator::TPGParameters::teamAccessAllActionsComment,
        Json::commentBefore);

    root["mutation"]["tpg"]["pChangeActionClass"] =
        params.mutation.tpg.pChangeActionClass;
    root["mutation"]["tpg"]["pChangeActionClass"].setComment(
        Mutator::TPGParameters::pChangeActionClassComment, Json::commentBefore);

    root["mutation"]["tpg"]["pActionEdgeAddition"] =
        params.mutation.tpg.pActionEdgeAddition;
    root["mutation"]["tpg"]["pActionEdgeAddition"].setComment(
        Mutator::TPGParameters::pActionEdgeAdditionComment,
        Json::commentBefore);

    root["mutation"]["tpg"]["pActionEdgeDeletion"] =
        params.mutation.tpg.pActionEdgeDeletion;
    root["mutation"]["tpg"]["pActionEdgeDeletion"].setComment(
        Mutator::TPGParameters::pActionEdgeDeletionComment,
        Json::commentBefore);

    root["mutation"]["tpg"]["pMutateActionProgram"] =
        params.mutation.tpg.pMutateActionProgram;
    root["mutation"]["tpg"]["pMutateActionProgram"].setComment(
        Mutator::TPGParameters::pMutateActionProgramComment,
        Json::commentBefore);
    root["mutation"]["tpg"]["pSwapActionProgram"] =
        params.mutation.tpg.pSwapActionProgram;
    root["mutation"]["tpg"]["pSwapActionProgram"].setComment(
        Mutator::TPGParameters::pSwapActionProgramComment, Json::commentBefore);

    root["mutation"]["tpg"]["maxOutgoingEdges"] =
        params.mutation.tpg.maxOutgoingEdges;
    root["mutation"]["tpg"]["maxOutgoingEdges"].setComment(
        Mutator::TPGParameters::maxOutgoingEdgesComment, Json::commentBefore);

    root["mutation"]["tpg"]["nbRoots"] = params.mutation.tpg.nbRoots;
    root["mutation"]["tpg"]["nbRoots"].setComment(
        Mutator::TPGParameters::nbRootsComment, Json::commentBefore);

    root["mutation"]["tpg"]["ratioTeamsOverActions"] =
        params.mutation.tpg.ratioTeamsOverActions;
    root["mutation"]["tpg"]["ratioTeamsOverActions"].setComment(
        Mutator::TPGParameters::ratioTeamsOverActionsComment,
        Json::commentBefore);

    root["mutation"]["tpg"]["pEdgeAddition"] =
        params.mutation.tpg.pEdgeAddition;
    root["mutation"]["tpg"]["pEdgeAddition"].setComment(
        Mutator::TPGParameters::pEdgeAdditionComment, Json::commentBefore);

    root["mutation"]["tpg"]["pEdgeDeletion"] =
        params.mutation.tpg.pEdgeDeletion;
    root["mutation"]["tpg"]["pEdgeDeletion"].setComment(
        Mutator::TPGParameters::pEdgeDeletionComment, Json::commentBefore);

    root["mutation"]["tpg"]["pEdgeDestinationChange"] =
        params.mutation.tpg.pEdgeDestinationChange;
    root["mutation"]["tpg"]["pEdgeDestinationChange"].setComment(
        Mutator::TPGParameters::pEdgeDestinationChangeComment,
        Json::commentBefore);

    root["mutation"]["tpg"]["pEdgeDestinationIsAction"] =
        params.mutation.tpg.pEdgeDestinationIsAction;
    root["mutation"]["tpg"]["pEdgeDestinationIsAction"].setComment(
        Mutator::TPGParameters::pEdgeDestinationIsActionComment,
        Json::commentBefore);

    root["mutation"]["tpg"]["pProgramMutation"] =
        params.mutation.tpg.pProgramMutation;
    root["mutation"]["tpg"]["pProgramMutation"].setComment(
        Mutator::TPGParameters::pProgramMutationComment, Json::commentBefore);

    root["mutation"]["tpg"]["useActionProgram"] =
        params.mutation.tpg.useActionProgram;
    root["mutation"]["tpg"]["useActionProgram"].setComment(
        Mutator::TPGParameters::useActionProgramComment, Json::commentBefore);

    root["mutation"]["tpg"]["probaContextOverActionProgram"] =
        params.mutation.tpg.probaContextOverActionProgram;
    root["mutation"]["tpg"]["probaContextOverActionProgram"].setComment(
        Mutator::TPGParameters::probaContextOverActionProgramComment,
        Json::commentBefore);

    root["mutation"]["tpg"]["pCrossAgents"] = params.mutation.tpg.pCrossAgents;
    root["mutation"]["tpg"]["pCrossAgents"].setComment(
        Mutator::TPGParameters::pCrossAgentsComment, Json::commentBefore);

    root["mutation"]["tpg"]["pCrossPrograms"] =
        params.mutation.tpg.pCrossPrograms;
    root["mutation"]["tpg"]["pCrossPrograms"].setComment(
        Mutator::TPGParameters::pCrossProgramsComment, Json::commentBefore);


    root["mutation"]["tpg"]["nbBestAgents"] = params.mutation.tpg.nbBestAgents;
    root["mutation"]["tpg"]["nbBestAgents"].setComment(
        Mutator::TPGParameters::nbBestAgentsComment, Json::commentBefore);

    root["mutation"]["tpg"]["nbLastGenPlateau"] = params.mutation.tpg.nbLastGenPlateau;
    root["mutation"]["tpg"]["nbLastGenPlateau"].setComment(
        Mutator::TPGParameters::nbLastGenPlateauComment, Json::commentBefore);

    root["mutation"]["tpg"]["alphaEMALinearFactor"] = params.mutation.tpg.alphaEMALinearFactor;
    root["mutation"]["tpg"]["alphaEMALinearFactor"].setComment(
        Mutator::TPGParameters::alphaEMALinearFactorComment, Json::commentBefore);

    root["mutation"]["tpg"]["epsilonPlateauThreshold"] = params.mutation.tpg.epsilonPlateauThreshold;
    root["mutation"]["tpg"]["epsilonPlateauThreshold"].setComment(
        Mutator::TPGParameters::epsilonPlateauThresholdComment, Json::commentBefore);

    root["mutation"]["tpg"]["speciesProbaDupplicateNextVertex"] = params.mutation.tpg.speciesProbaDupplicateNextVertex;
    root["mutation"]["tpg"]["speciesProbaDupplicateNextVertex"].setComment(
        Mutator::TPGParameters::speciesProbaDupplicateNextVertexComment, Json::commentBefore);

    root["mutation"]["tpg"]["speciesMutateSpecificFirst"] = params.mutation.tpg.speciesMutateSpecificFirst;
    root["mutation"]["tpg"]["speciesMutateSpecificFirst"].setComment(
        Mutator::TPGParameters::speciesMutateSpecificFirstComment, Json::commentBefore);

    root["mutation"]["tpg"]["speciesProbaAddContext"] = params.mutation.tpg.speciesProbaAddContext;
    root["mutation"]["tpg"]["speciesProbaAddContext"].setComment(
        Mutator::TPGParameters::speciesProbaAddContextComment, Json::commentBefore);

    root["mutation"]["tpg"]["speciesProbaAddActivation"] = params.mutation.tpg.speciesProbaAddActivation;
    root["mutation"]["tpg"]["speciesProbaAddActivation"].setComment(
        Mutator::TPGParameters::speciesProbaAddActivationComment, Json::commentBefore);

    root["mutation"]["tpg"]["speciesProbaDelete"] = params.mutation.tpg.speciesProbaDelete;
    root["mutation"]["tpg"]["speciesProbaDelete"].setComment(
        Mutator::TPGParameters::speciesProbaDeleteComment, Json::commentBefore);

    root["mutation"]["tpg"]["speciesProbaExtension"] = params.mutation.tpg.speciesProbaExtension;
    root["mutation"]["tpg"]["speciesProbaExtension"].setComment(
        Mutator::TPGParameters::speciesProbaExtensionComment, Json::commentBefore);

    root["mutation"]["tpg"]["speciesProbaCrossProgram"] = params.mutation.tpg.speciesProbaCrossProgram;
    root["mutation"]["tpg"]["speciesProbaCrossProgram"].setComment(
        Mutator::TPGParameters::speciesProbaCrossProgramComment, Json::commentBefore);

    root["mutation"]["tpg"]["speciesProbaCrossAgent"] = params.mutation.tpg.speciesProbaCrossAgent;
    root["mutation"]["tpg"]["speciesProbaCrossAgent"].setComment(
        Mutator::TPGParameters::speciesProbaCrossAgentComment, Json::commentBefore);

    root["mutation"]["tpg"]["speciesProbaSwapContext"] = params.mutation.tpg.speciesProbaSwapContext;
    root["mutation"]["tpg"]["speciesProbaSwapContext"].setComment(
        Mutator::TPGParameters::speciesProbaSwapContextComment, Json::commentBefore);

    root["mutation"]["tpg"]["speciesProbaSwapActionValues"] = params.mutation.tpg.speciesProbaSwapActionValues;
    root["mutation"]["tpg"]["speciesProbaSwapActionValues"].setComment(
        Mutator::TPGParameters::speciesProbaSwapActionValuesComment, Json::commentBefore);

    root["mutation"]["tpg"]["speciesProbaSwapPrograms"] = params.mutation.tpg.speciesProbaSwapPrograms;
    root["mutation"]["tpg"]["speciesProbaSwapPrograms"].setComment(
        Mutator::TPGParameters::speciesProbaSwapProgramsComment, Json::commentBefore);

    root["mutation"]["tpg"]["speciesProbaMutateProgram"] = params.mutation.tpg.speciesProbaMutateProgram;
    root["mutation"]["tpg"]["speciesProbaMutateProgram"].setComment(
        Mutator::TPGParameters::speciesProbaMutateProgramComment, Json::commentBefore);

    // Mutation.program parameters
    root["mutation"]["prog"]["maxConstValue"] =
        params.mutation.prog.maxConstValue;
    root["mutation"]["prog"]["maxConstValue"].setComment(
        Mutator::ProgramParameters::maxConstValueComment, Json::commentBefore);

    root["mutation"]["prog"]["maxProgramSize"] =
        params.mutation.prog.maxProgramSize;
    root["mutation"]["prog"]["maxProgramSize"].setComment(
        Mutator::ProgramParameters::maxProgramSizeComment, Json::commentBefore);

    root["mutation"]["prog"]["initMinProgramSize"] =
        params.mutation.prog.initMinProgramSize;
    root["mutation"]["prog"]["initMinProgramSize"].setComment(
        Mutator::ProgramParameters::initMinProgramSizeComment,
        Json::commentBefore);

    root["mutation"]["prog"]["initMaxProgramSize"] =
        params.mutation.prog.initMaxProgramSize;
    root["mutation"]["prog"]["initMaxProgramSize"].setComment(
        Mutator::ProgramParameters::initMaxProgramSizeComment,
        Json::commentBefore);

    root["mutation"]["prog"]["minConstValue"] =
        params.mutation.prog.minConstValue;
    root["mutation"]["prog"]["minConstValue"].setComment(
        Mutator::ProgramParameters::minConstValueComment, Json::commentBefore);

    root["mutation"]["prog"]["pAdd"] = params.mutation.prog.pAdd;
    root["mutation"]["prog"]["pAdd"].setComment(
        Mutator::ProgramParameters::pAddComment, Json::commentBefore);

    root["mutation"]["prog"]["pConstantMutation"] =
        params.mutation.prog.pConstantMutation;
    root["mutation"]["prog"]["pConstantMutation"].setComment(
        Mutator::ProgramParameters::pConstantMutationComment,
        Json::commentBefore);

    root["mutation"]["prog"]["pNewProgram"] = params.mutation.prog.pNewProgram;
    root["mutation"]["prog"]["pNewProgram"].setComment(
        Mutator::ProgramParameters::pNewProgramComment, Json::commentBefore);

    root["mutation"]["prog"]["pDelete"] = params.mutation.prog.pDelete;
    root["mutation"]["prog"]["pDelete"].setComment(
        Mutator::ProgramParameters::pDeleteComment, Json::commentBefore);

    root["mutation"]["prog"]["pMutate"] = params.mutation.prog.pMutate;
    root["mutation"]["prog"]["pMutate"].setComment(
        Mutator::ProgramParameters::pMutateComment, Json::commentBefore);

    root["mutation"]["prog"]["pSwap"] = params.mutation.prog.pSwap;
    root["mutation"]["prog"]["pSwap"].setComment(
        Mutator::ProgramParameters::pSwapComment, Json::commentBefore);

    root["selection"]["_selectionMode"] = params.selection._selectionMode;
    root["selection"]["_selectionMode"].setComment(
        Selector::SelectionParameters::selectionModeComment,
        Json::commentBefore);
    root["selection"]["truncation"]["ratioDeletedRoots"] =
        params.selection.truncation.ratioDeletedRoots;
    root["selection"]["truncation"]["ratioDeletedRoots"].setComment(
        Selector::TruncationParameters::ratioDeletedRootsComment,
        Json::commentBefore);
    root["selection"]["tournament"]["ratioSavedRoots"] =
        params.selection.tournament.ratioSavedRoots;
    root["selection"]["tournament"]["ratioSavedRoots"].setComment(
        Selector::TournamentParameters::ratioSavedRootsComment,
        Json::commentBefore);
    root["selection"]["tournament"]["sizeTournament"] =
        params.selection.tournament.sizeTournament;
    root["selection"]["tournament"]["sizeTournament"].setComment(
        Selector::TournamentParameters::sizeTournamentComment,
        Json::commentBefore);
    root["selection"]["tournament"]["areElitesReproductible"] =
        params.selection.tournament.areElitesReproductible;
    root["selection"]["tournament"]["areElitesReproductible"].setComment(
        Selector::TournamentParameters::areElitesReproductibleComment,
        Json::commentBefore);

    // Write to the output stream
    std::ofstream writtenFile(path);
    Json::StreamWriterBuilder writerFactory;
    // Set a precision to 6 digits after the point.
    writerFactory.settings_["precision"] = 6U;
    Json::StreamWriter* writer = writerFactory.newStreamWriter();
    writer->write(root, &writtenFile);
    delete writer;
    writtenFile.close();
}
