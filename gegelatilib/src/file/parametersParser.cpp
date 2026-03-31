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
                                              Parameters& params)
{
    for (std::string const& key : root.getMemberNames()) {
        if (root[key].size() == 0) {
            // we have a parameter without subtree (as a leaf)
            Json::Value value = root[key];
            setParameterFromString(params, key, value);
        } else {
            for (std::string const& key2 : root[key].getMemberNames()) {
                if (root[key][key2].size() == 0) {
                    // we have a parameter without subtree (as a leaf)
                    Json::Value value = root[key][key2];
                    setParameterFromString(params, key2, value);
                } else {
                    for (std::string const& key3 : root[key][key2].getMemberNames()) {
                        if (root[key][key2][key3].size() == 0) {
                            // we have a parameter without subtree (as a leaf)
                            Json::Value value = root[key][key2][key3];
                            setParameterFromString(params, key3, value);
                        }
                    }
                }
            }
        }
    }
}

void File::ParametersParser::setParameterFromString(
    Parameters& params, const std::string& param,
    Json::Value const& value)
{

    if (param == "doValidation") {
        params.evaluation.doValidation = value.asBool();
        return;
    }
    if (param == "nbIterationsPerPolicyEvaluation") {
        params.evaluation.nbIterationsPerPolicyEvaluation = value.asUInt64();
        return;
    }
    if (param == "nbIterationsPerPolicyValidation") {
        params.evaluation.nbIterationsPerPolicyValidation = value.asUInt64();
        return;
    }
    if (param == "nbThreads") {
        params.evaluation.nbThreads = (size_t)value.asUInt();
        return;
    }
    if (param == "nbGenerations") {
        params.evaluation.nbGenerations = value.asUInt64();
        return;
    }
    if (param == "maxNbActionsPerEval") {
        params.evaluation.maxNbActionsPerEval = value.asUInt64();
        return;
    }
    if (param == "maxNbEvaluationPerPolicy") {
        params.evaluation.maxNbEvaluationPerPolicy = (size_t)value.asUInt();
        return;
    }
    if (param == "stepValidation") {
        params.evaluation.stepValidation = (size_t)value.asUInt();
        return;
    }

    if (param == "nbAgents") {
        params.algorithm.nbAgents = (size_t)value.asUInt();
        return;
    }
    if (param == "activationFunction") {
        params.algorithm.activationFunction = value.asString();
        return;
    }


    if (param == "archiveSize") {
        params.algorithm.tpg.archiveSize = (size_t)value.asUInt();
        return;
    }
    if (param == "archivingProbability") {
        params.algorithm.tpg.archivingProbability = value.asDouble();
        return;
    }
    if (param == "maxInitOutgoingEdges") {
        params.algorithm.tpg.maxInitOutgoingEdges = (size_t)value.asUInt();
        return;
    }
    if (param == "maxOutgoingEdges") {
        params.algorithm.tpg.maxOutgoingEdges = (size_t)value.asUInt();
        return;
    }
    if (param == "pEdgeDeletion") {
        params.algorithm.tpg.pEdgeDeletion = value.asDouble();
        return;
    }
    if (param == "pEdgeAddition") {
        params.algorithm.tpg.pEdgeAddition = value.asDouble();
        return;
    }
    if (param == "pProgramMutation") {
        params.algorithm.tpg.pProgramMutation = value.asDouble();
        return;
    }
    if (param == "forceProgramBehaviorChangeOnMutation") {
        params.algorithm.tpg.forceProgramBehaviorChangeOnMutation =
            value.asBool();
        return;
    }
    if (param == "pEdgeDestinationChange") {
        params.algorithm.tpg.pEdgeDestinationChange = value.asDouble();
        return;
    }
    if (param == "pEdgeDestinationIsAction") {
        params.algorithm.tpg.pEdgeDestinationIsAction = value.asDouble();
        return;
    }


    if (param == "probaContextOverActionProgram") {
        params.algorithm.atpg.probaContextOverActionProgram =
            (double)value.asDouble();
        return;
    }


    
    if (param == "nbActionEdgeInit") {
        params.algorithm.maple.nbActionEdgeInit = (size_t)value.asUInt();
        return;
    }
    if (param == "pChangeActionClass") {
        params.algorithm.maple.pChangeActionClass = (double)value.asDouble();
        return;
    }
    if (param == "pActionEdgeDeletion") {
        params.algorithm.maple.pActionEdgeDeletion = value.asDouble();
        return;
    }
    if (param == "pActionEdgeAddition") {
        params.algorithm.maple.pActionEdgeAddition = value.asDouble();
        return;
    }
    if (param == "pMutateActionProgram") {
        params.algorithm.maple.pMutateActionProgram = value.asDouble();
        return;
    }
    if (param == "pSwapActionProgram") {
        params.algorithm.maple.pSwapActionProgram = value.asDouble();
        return;
    }
    if (param == "pCrossAgents") {
        params.algorithm.maple.pCrossAgents = (double)value.asDouble();
        return;
    }
    if (param == "pCrossPrograms") {
        params.algorithm.maple.pCrossPrograms = (double)value.asDouble();
        return;
    }



    if (param == "nbRegisters") {
        params.algorithm.lgp.nbRegisters = (size_t)value.asUInt();
        return;
    }
    if (param == "maxProgramSize") {
        params.algorithm.lgp.maxProgramSize = (size_t)value.asUInt();
        return;
    }
    if (param == "initMinProgramSize") {
        params.algorithm.lgp.initMinProgramSize = (size_t)value.asUInt();
        return;
    }
    if (param == "initMaxProgramSize") {
        params.algorithm.lgp.initMaxProgramSize = (size_t)value.asUInt();
        return;
    }
    if (param == "pDelete") {
        params.algorithm.lgp.pDelete = value.asDouble();
        return;
    }
    if (param == "pAdd") {
        params.algorithm.lgp.pAdd = value.asDouble();
        return;
    }
    if (param == "pMutate") {
        params.algorithm.lgp.pMutate = value.asDouble();
        return;
    }
    if (param == "pSwap") {
        params.algorithm.lgp.pSwap = value.asDouble();
        return;
    }
    if (param == "nbProgramConstant") {
        params.algorithm.lgp.nbProgramConstant = (size_t)value.asUInt();
        return;
    }
    if (param == "pConstantMutation") {
        params.algorithm.lgp.pConstantMutation = value.asDouble();
        return;
    }
    if (param == "pNewProgram") {
        params.algorithm.lgp.pNewProgram = value.asDouble();
        return;
    }
    if (param == "minConstValue") {
        params.algorithm.lgp.minConstValue = value.asDouble();
        return;
    }
    if (param == "maxConstValue") {
        params.algorithm.lgp.maxConstValue = value.asDouble();
        return;
    }
    if (param == "pMutateOutput") {
        params.algorithm.lgp.pMutateOutput = value.asDouble();
        return;
    }


    if (param == "nbLayers") {
        params.algorithm.cgp.nbLayers = (size_t)value.asUInt();
        return;
    }
    if (param == "nbNodesPerLayer") {
        params.algorithm.cgp.nbNodesPerLayer = (size_t)value.asUInt();
        return;
    }
    if (param == "pMutateNode") {
        params.algorithm.cgp.pMutateNode = value.asDouble();
        return;
    }


    if (param == "maxDepth") {
        params.algorithm.tgp.maxDepth = (size_t)value.asUInt();
        return;
    }
    if (param == "maxInitDepth") {
        params.algorithm.tgp.maxInitDepth = (size_t)value.asUInt();
        return;
    }
    if (param == "maxNbEdgePerNode") {
        params.algorithm.tgp.maxNbEdgePerNode= (size_t)value.asUInt();
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
    const char* path, Parameters& params)
{
    Json::Value root;
    readConfigFile(path, root);

    setAllParamsFrom(root, params);
}

void File::ParametersParser::writeParametersToJson(
    const char* path, const Parameters& params)
{
    Parameters defaultParams;
    // Create the JSON Structure
    Json::Value root;

    // Fill it
    // base parameters

    root["doValidation"] = params.evaluation.doValidation;
    root["doValidation"].setComment(
        Learn::LearningParameters::doValidationComment, Json::commentBefore);

    root["maxNbActionsPerEval"] = params.evaluation.maxNbActionsPerEval;
    root["maxNbActionsPerEval"].setComment(
        Learn::LearningParameters::maxNbActionsPerEvalComment,
        Json::commentBefore);

    root["maxNbEvaluationPerPolicy"] = params.evaluation.maxNbEvaluationPerPolicy;
    root["maxNbEvaluationPerPolicy"].setComment(
        Learn::LearningParameters::maxNbEvaluationPerPolicyComment,
        Json::commentBefore);

    root["nbGenerations"] = params.evaluation.nbGenerations;
    root["nbGenerations"].setComment(
        Learn::LearningParameters::nbGenerationsComment, Json::commentBefore);

    root["nbIterationsPerPolicyEvaluation"] =
        params.evaluation.nbIterationsPerPolicyEvaluation;
    root["nbIterationsPerPolicyEvaluation"].setComment(
        Learn::LearningParameters::nbIterationsPerPolicyEvaluationComment,
        Json::commentBefore);
    root["nbIterationsPerPolicyValidation"] =
        params.evaluation.nbIterationsPerPolicyValidation;
    root["nbIterationsPerPolicyValidation"].setComment(
        Learn::LearningParameters::nbIterationsPerPolicyValidationComment,
        Json::commentBefore);
    root["stepValidation"] = params.evaluation.stepValidation;
    root["stepValidation"].setComment(
        Learn::LearningParameters::stepValidationComment, Json::commentBefore);

    root["nbThreads"] = params.evaluation.nbThreads;
    root["nbThreads"].setComment(Learn::LearningParameters::nbThreadsComment,
                                 Json::commentBefore);




    root["algorithm"]["nbAgents"] = params.algorithm.nbAgents;
    root["algorithm"]["nbAgents"].setComment(
        Algorithm::AlgorithmParameters::nbAgentsComment, Json::commentBefore);

    root["algorithm"]["activationFunction"] = params.algorithm.activationFunction;
    root["algorithm"]["activationFunction"].setComment(
        Algorithm::AlgorithmParameters::activationFunctionComment,
        Json::commentBefore);

    root["algorithm"]["tpg"]["archiveSize"] = params.algorithm.tpg.archiveSize;
    root["algorithm"]["tpg"]["archiveSize"].setComment(
        Algorithm::TPG::TPGParameters::archiveSizeComment, Json::commentBefore);

    root["algorithm"]["tpg"]["archivingProbability"] = params.algorithm.tpg.archivingProbability;
    root["algorithm"]["tpg"]["archivingProbability"].setComment(
        Algorithm::TPG::TPGParameters::archivingProbabilityComment,
        Json::commentBefore);

    // algorithm.tpg parameters
    root["algorithm"]["tpg"]["forceProgramBehaviorChangeOnMutation"] =
        params.algorithm.tpg.forceProgramBehaviorChangeOnMutation;
    root["algorithm"]["tpg"]["forceProgramBehaviorChangeOnMutation"].setComment(
        Algorithm::TPG::TPGParameters::forceProgramBehaviorChangeOnMutationComment,
        Json::commentBefore);

    root["algorithm"]["tpg"]["maxInitOutgoingEdges"] =
        params.algorithm.tpg.maxInitOutgoingEdges;
    root["algorithm"]["tpg"]["maxInitOutgoingEdges"].setComment(
        Algorithm::TPG::TPGParameters::maxInitOutgoingEdgesComment,
        Json::commentBefore);

    root["algorithm"]["tpg"]["maxOutgoingEdges"] =
        params.algorithm.tpg.maxOutgoingEdges;
    root["algorithm"]["tpg"]["maxOutgoingEdges"].setComment(
        Algorithm::TPG::TPGParameters::maxOutgoingEdgesComment, Json::commentBefore);

    root["algorithm"]["tpg"]["pEdgeAddition"] =
        params.algorithm.tpg.pEdgeAddition;
    root["algorithm"]["tpg"]["pEdgeAddition"].setComment(
        Algorithm::TPG::TPGParameters::pEdgeAdditionComment, Json::commentBefore);

    root["algorithm"]["tpg"]["pEdgeDeletion"] =
        params.algorithm.tpg.pEdgeDeletion;
    root["algorithm"]["tpg"]["pEdgeDeletion"].setComment(
        Algorithm::TPG::TPGParameters::pEdgeDeletionComment, Json::commentBefore);

    root["algorithm"]["tpg"]["pEdgeDestinationChange"] =
        params.algorithm.tpg.pEdgeDestinationChange;
    root["algorithm"]["tpg"]["pEdgeDestinationChange"].setComment(
        Algorithm::TPG::TPGParameters::pEdgeDestinationChangeComment,
        Json::commentBefore);

    root["algorithm"]["tpg"]["pEdgeDestinationIsAction"] =
        params.algorithm.tpg.pEdgeDestinationIsAction;
    root["algorithm"]["tpg"]["pEdgeDestinationIsAction"].setComment(
        Algorithm::TPG::TPGParameters::pEdgeDestinationIsActionComment,
        Json::commentBefore);
    root["algorithm"]["tpg"]["pProgramMutation"] =
        params.algorithm.tpg.pProgramMutation;
    root["algorithm"]["tpg"]["pProgramMutation"].setComment(
        Algorithm::TPG::TPGParameters::pProgramMutationComment, Json::commentBefore);



    root["algorithm"]["atpg"]["probaContextOverActionProgram"] =
        params.algorithm.atpg.probaContextOverActionProgram;
    root["algorithm"]["atpg"]["probaContextOverActionProgram"].setComment(
        Algorithm::ATPG::ATPGParameters::probaContextOverActionProgramComment,
        Json::commentBefore);



    root["algorithm"]["maple"]["nbActionEdgeInit"] =
        params.algorithm.maple.nbActionEdgeInit;
    root["algorithm"]["maple"]["nbActionEdgeInit"].setComment(
        Algorithm::Maple::MapleParameters::nbActionEdgeInitComment, Json::commentBefore);
    root["algorithm"]["maple"]["pChangeActionClass"] =
        params.algorithm.maple.pChangeActionClass;
    root["algorithm"]["maple"]["pChangeActionClass"].setComment(
        Algorithm::Maple::MapleParameters::pChangeActionClassComment, Json::commentBefore);
    root["algorithm"]["maple"]["pActionEdgeAddition"] =
        params.algorithm.maple.pActionEdgeAddition;
    root["algorithm"]["maple"]["pActionEdgeAddition"].setComment(
        Algorithm::Maple::MapleParameters::pActionEdgeAdditionComment,
        Json::commentBefore);
    root["algorithm"]["maple"]["pActionEdgeDeletion"] =
        params.algorithm.maple.pActionEdgeDeletion;
    root["algorithm"]["maple"]["pActionEdgeDeletion"].setComment(
        Algorithm::Maple::MapleParameters::pActionEdgeDeletionComment,
        Json::commentBefore);
    root["algorithm"]["maple"]["pMutateActionProgram"] =
        params.algorithm.maple.pMutateActionProgram;
    root["algorithm"]["maple"]["pMutateActionProgram"].setComment(
        Algorithm::Maple::MapleParameters::pMutateActionProgramComment,
        Json::commentBefore);
    root["algorithm"]["maple"]["pSwapActionProgram"] =
        params.algorithm.maple.pSwapActionProgram;
    root["algorithm"]["maple"]["pSwapActionProgram"].setComment(
        Algorithm::Maple::MapleParameters::pSwapActionProgramComment, Json::commentBefore);
    root["algorithm"]["maple"]["pCrossAgents"] = params.algorithm.maple.pCrossAgents;
    root["algorithm"]["maple"]["pCrossAgents"].setComment(
        Algorithm::Maple::MapleParameters::pCrossAgentsComment, Json::commentBefore);
    root["algorithm"]["maple"]["pCrossPrograms"] =
        params.algorithm.maple.pCrossPrograms;
    root["algorithm"]["maple"]["pCrossPrograms"].setComment(
        Algorithm::Maple::MapleParameters::pCrossProgramsComment, Json::commentBefore);



    // Mutation.program parameters
    root["algorithm"]["lgp"]["nbProgramConstant"] = params.algorithm.lgp.nbProgramConstant;
    root["algorithm"]["lgp"]["nbProgramConstant"].setComment(
        Algorithm::LGP::LGPParameters::nbProgramConstantComment,
        Json::commentBefore);
    root["algorithm"]["lgp"]["nbRegisters"] = params.algorithm.lgp.nbRegisters;
    root["algorithm"]["lgp"]["nbRegisters"].setComment(
        Algorithm::LGP::LGPParameters::nbRegistersComment, Json::commentBefore);
    root["algorithm"]["lgp"]["maxConstValue"] =
        params.algorithm.lgp.maxConstValue;
    root["algorithm"]["lgp"]["maxConstValue"].setComment(
        Algorithm::LGP::LGPParameters::maxConstValueComment, Json::commentBefore);
    root["algorithm"]["lgp"]["maxProgramSize"] =
        params.algorithm.lgp.maxProgramSize;
    root["algorithm"]["lgp"]["maxProgramSize"].setComment(
        Algorithm::LGP::LGPParameters::maxProgramSizeComment, Json::commentBefore);
    root["algorithm"]["lgp"]["initMinProgramSize"] =
        params.algorithm.lgp.initMinProgramSize;
    root["algorithm"]["lgp"]["initMinProgramSize"].setComment(
        Algorithm::LGP::LGPParameters::initMinProgramSizeComment,
        Json::commentBefore);
    root["algorithm"]["lgp"]["initMaxProgramSize"] =
        params.algorithm.lgp.initMaxProgramSize;
    root["algorithm"]["lgp"]["initMaxProgramSize"].setComment(
        Algorithm::LGP::LGPParameters::initMaxProgramSizeComment,
        Json::commentBefore);
    root["algorithm"]["lgp"]["minConstValue"] =
        params.algorithm.lgp.minConstValue;
    root["algorithm"]["lgp"]["minConstValue"].setComment(
        Algorithm::LGP::LGPParameters::minConstValueComment, Json::commentBefore);
    root["algorithm"]["lgp"]["pAdd"] = params.algorithm.lgp.pAdd;
    root["algorithm"]["lgp"]["pAdd"].setComment(
        Algorithm::LGP::LGPParameters::pAddComment, Json::commentBefore);
    root["algorithm"]["lgp"]["pConstantMutation"] =
        params.algorithm.lgp.pConstantMutation;
    root["algorithm"]["lgp"]["pConstantMutation"].setComment(
        Algorithm::LGP::LGPParameters::pConstantMutationComment,
        Json::commentBefore);
    root["algorithm"]["lgp"]["pNewProgram"] = params.algorithm.lgp.pNewProgram;
    root["algorithm"]["lgp"]["pNewProgram"].setComment(
        Algorithm::LGP::LGPParameters::pNewProgramComment, Json::commentBefore);
    root["algorithm"]["lgp"]["pDelete"] = params.algorithm.lgp.pDelete;
    root["algorithm"]["lgp"]["pDelete"].setComment(
        Algorithm::LGP::LGPParameters::pDeleteComment, Json::commentBefore);
    root["algorithm"]["lgp"]["pMutate"] = params.algorithm.lgp.pMutate;
    root["algorithm"]["lgp"]["pMutate"].setComment(
        Algorithm::LGP::LGPParameters::pMutateComment, Json::commentBefore);
    root["algorithm"]["lgp"]["pSwap"] = params.algorithm.lgp.pSwap;
    root["algorithm"]["lgp"]["pSwap"].setComment(
        Algorithm::LGP::LGPParameters::pSwapComment, Json::commentBefore);
    root["algorithm"]["lgp"]["pMutateOutput"] = params.algorithm.lgp.pMutateOutput;
    root["algorithm"]["lgp"]["pMutateOutput"].setComment(
        Algorithm::LGP::LGPParameters::pMutateOutputComment, Json::commentBefore);


    root["algorithm"]["cgp"]["nbLayers"] = params.algorithm.cgp.nbLayers;
    root["algorithm"]["cgp"]["nbLayers"].setComment(
        Algorithm::CGP::CGPParameters::nbLayersComment, Json::commentBefore);
    root["algorithm"]["cgp"]["nbNodesPerLayer"] = params.algorithm.cgp.nbNodesPerLayer;
    root["algorithm"]["cgp"]["nbNodesPerLayer"].setComment(
        Algorithm::CGP::CGPParameters::nbNodesPerLayerComment, Json::commentBefore);
    root["algorithm"]["cgp"]["pMutateNode"] = params.algorithm.cgp.pMutateNode;
    root["algorithm"]["cgp"]["pMutateNode"].setComment(
        Algorithm::CGP::CGPParameters::pMutateNodeComment, Json::commentBefore);
    

    root["algorithm"]["tgp"]["nbLayers"] = params.algorithm.tgp.maxDepth;
    root["algorithm"]["tgp"]["nbLayers"].setComment(
        Algorithm::TGP::TGPParameters::maxDepthComment, Json::commentBefore);
    root["algorithm"]["tgp"]["nbNodesPerLayer"] = params.algorithm.tgp.maxInitDepth;
    root["algorithm"]["tgp"]["nbNodesPerLayer"].setComment(
        Algorithm::TGP::TGPParameters::maxInitDepthComment, Json::commentBefore);
    root["algorithm"]["tgp"]["pMutateNode"] = params.algorithm.tgp.maxNbEdgePerNode;
    root["algorithm"]["tgp"]["pMutateNode"].setComment(
        Algorithm::TGP::TGPParameters::maxNbEdgePerNodeComment, Json::commentBefore);
    


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
