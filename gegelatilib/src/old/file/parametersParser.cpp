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

    if (param == "nbIndividuals") {
        params.representation.nbIndividuals = (size_t)value.asUInt();
        return;
    }
    if (param == "activationFunction") {
        params.representation.activationFunction = value.asString();
        return;
    }


    if (param == "archiveSize") {
        params.representation.tpg.archiveSize = (size_t)value.asUInt();
        return;
    }
    if (param == "archivingProbability") {
        params.representation.tpg.archivingProbability = value.asDouble();
        return;
    }
    if (param == "maxInitOutgoingEdges") {
        params.representation.tpg.maxInitOutgoingEdges = (size_t)value.asUInt();
        return;
    }
    if (param == "maxOutgoingEdges") {
        params.representation.tpg.maxOutgoingEdges = (size_t)value.asUInt();
        return;
    }
    if (param == "pEdgeDeletion") {
        params.representation.tpg.pEdgeDeletion = value.asDouble();
        return;
    }
    if (param == "pEdgeAddition") {
        params.representation.tpg.pEdgeAddition = value.asDouble();
        return;
    }
    if (param == "pProgramMutation") {
        params.representation.tpg.pProgramMutation = value.asDouble();
        return;
    }
    if (param == "pEdgeDestinationChange") {
        params.representation.tpg.pEdgeDestinationChange = value.asDouble();
        return;
    }
    if (param == "pEdgeDestinationIsAction") {
        params.representation.tpg.pEdgeDestinationIsAction = value.asDouble();
        return;
    }


    if (param == "probaContextOverActionProgram") {
        params.representation.atpg.probaContextOverActionProgram =
            (double)value.asDouble();
        return;
    }


    
    if (param == "nbActionEdgeInit") {
        params.representation.maple.nbActionEdgeInit = (size_t)value.asUInt();
        return;
    }
    if (param == "pChangeActionClass") {
        params.representation.maple.pChangeActionClass = (double)value.asDouble();
        return;
    }
    if (param == "pActionEdgeDeletion") {
        params.representation.maple.pActionEdgeDeletion = value.asDouble();
        return;
    }
    if (param == "pActionEdgeAddition") {
        params.representation.maple.pActionEdgeAddition = value.asDouble();
        return;
    }
    if (param == "pMutateActionProgram") {
        params.representation.maple.pMutateActionProgram = value.asDouble();
        return;
    }
    if (param == "pSwapActionProgram") {
        params.representation.maple.pSwapActionProgram = value.asDouble();
        return;
    }
    if (param == "pCrossIndividuals") {
        params.representation.maple.pCrossIndividuals = (double)value.asDouble();
        return;
    }
    if (param == "pCrossPrograms") {
        params.representation.maple.pCrossPrograms = (double)value.asDouble();
        return;
    }



    if (param == "forceProgramBehaviorChangeOnMutation") {
        params.representation.lgp.forceProgramBehaviorChangeOnMutation =
            value.asBool();
        return;
    }
    if (param == "nbRegisters") {
        params.representation.lgp.nbRegisters = (size_t)value.asUInt();
        return;
    }
    if (param == "maxProgramSize") {
        params.representation.lgp.maxProgramSize = (size_t)value.asUInt();
        return;
    }
    if (param == "initMinProgramSize") {
        params.representation.lgp.initMinProgramSize = (size_t)value.asUInt();
        return;
    }
    if (param == "initMaxProgramSize") {
        params.representation.lgp.initMaxProgramSize = (size_t)value.asUInt();
        return;
    }
    if (param == "pDelete") {
        params.representation.lgp.pDelete = value.asDouble();
        return;
    }
    if (param == "pAdd") {
        params.representation.lgp.pAdd = value.asDouble();
        return;
    }
    if (param == "pMutate") {
        params.representation.lgp.pMutate = value.asDouble();
        return;
    }
    if (param == "pSwap") {
        params.representation.lgp.pSwap = value.asDouble();
        return;
    }
    if (param == "nbProgramConstant") {
        params.representation.lgp.nbProgramConstant = (size_t)value.asUInt();
        return;
    }
    if (param == "pConstantMutation") {
        params.representation.lgp.pConstantMutation = value.asDouble();
        return;
    }
    if (param == "pNewProgram") {
        params.representation.lgp.pNewProgram = value.asDouble();
        return;
    }
    if (param == "minConstValue") {
        params.representation.lgp.minConstValue = value.asDouble();
        return;
    }
    if (param == "maxConstValue") {
        params.representation.lgp.maxConstValue = value.asDouble();
        return;
    }
    if (param == "pMutateOutput") {
        params.representation.lgp.pMutateOutput = value.asDouble();
        return;
    }


    if (param == "nbLayers") {
        params.representation.cgp.nbLayers = (size_t)value.asUInt();
        return;
    }
    if (param == "nbNodesPerLayer") {
        params.representation.cgp.nbNodesPerLayer = (size_t)value.asUInt();
        return;
    }
    if (param == "pMutateNode") {
        params.representation.cgp.pMutateNode = value.asDouble();
        return;
    }


    if (param == "maxDepth") {
        params.representation.tgp.maxDepth = (size_t)value.asUInt();
        return;
    }
    if (param == "maxInitDepth") {
        params.representation.tgp.maxInitDepth = (size_t)value.asUInt();
        return;
    }
    if (param == "maxNbEdgePerNode") {
        params.representation.tgp.maxNbEdgePerNode= (size_t)value.asUInt();
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

    root["evaluation"]["doValidation"] = params.evaluation.doValidation;
    root["evaluation"]["doValidation"].setComment(
        Learn::LearningParameters::doValidationComment, Json::commentBefore);

    root["evaluation"]["maxNbActionsPerEval"] = params.evaluation.maxNbActionsPerEval;
    root["evaluation"]["maxNbActionsPerEval"].setComment(
        Learn::LearningParameters::maxNbActionsPerEvalComment,
        Json::commentBefore);

    root["evaluation"]["maxNbEvaluationPerPolicy"] = params.evaluation.maxNbEvaluationPerPolicy;
    root["evaluation"]["maxNbEvaluationPerPolicy"].setComment(
        Learn::LearningParameters::maxNbEvaluationPerPolicyComment,
        Json::commentBefore);

    root["evaluation"]["nbGenerations"] = params.evaluation.nbGenerations;
    root["evaluation"]["nbGenerations"].setComment(
        Learn::LearningParameters::nbGenerationsComment, Json::commentBefore);

    root["evaluation"]["nbIterationsPerPolicyEvaluation"] =
        params.evaluation.nbIterationsPerPolicyEvaluation;
    root["evaluation"]["nbIterationsPerPolicyEvaluation"].setComment(
        Learn::LearningParameters::nbIterationsPerPolicyEvaluationComment,
        Json::commentBefore);
    root["evaluation"]["nbIterationsPerPolicyValidation"] =
        params.evaluation.nbIterationsPerPolicyValidation;
    root["evaluation"]["nbIterationsPerPolicyValidation"].setComment(
        Learn::LearningParameters::nbIterationsPerPolicyValidationComment,
        Json::commentBefore);
    root["evaluation"]["stepValidation"] = params.evaluation.stepValidation;
    root["evaluation"]["stepValidation"].setComment(
        Learn::LearningParameters::stepValidationComment, Json::commentBefore);

    root["evaluation"]["nbThreads"] = params.evaluation.nbThreads;
    root["evaluation"]["nbThreads"].setComment(Learn::LearningParameters::nbThreadsComment,
                                 Json::commentBefore);




    root["representation"]["nbIndividuals"] = params.representation.nbIndividuals;
    root["representation"]["nbIndividuals"].setComment(
        Representation::RepresentationParameters::nbIndividualsComment, Json::commentBefore);

    root["representation"]["activationFunction"] = params.representation.activationFunction;
    root["representation"]["activationFunction"].setComment(
        Representation::RepresentationParameters::activationFunctionComment,
        Json::commentBefore);

    root["representation"]["tpg"]["archiveSize"] = params.representation.tpg.archiveSize;
    root["representation"]["tpg"]["archiveSize"].setComment(
        Representation::TPG::TPGParameters::archiveSizeComment, Json::commentBefore);

    root["representation"]["tpg"]["archivingProbability"] = params.representation.tpg.archivingProbability;
    root["representation"]["tpg"]["archivingProbability"].setComment(
        Representation::TPG::TPGParameters::archivingProbabilityComment,
        Json::commentBefore);

    root["representation"]["tpg"]["maxInitOutgoingEdges"] =
        params.representation.tpg.maxInitOutgoingEdges;
    root["representation"]["tpg"]["maxInitOutgoingEdges"].setComment(
        Representation::TPG::TPGParameters::maxInitOutgoingEdgesComment,
        Json::commentBefore);

    root["representation"]["tpg"]["maxOutgoingEdges"] =
        params.representation.tpg.maxOutgoingEdges;
    root["representation"]["tpg"]["maxOutgoingEdges"].setComment(
        Representation::TPG::TPGParameters::maxOutgoingEdgesComment, Json::commentBefore);

    root["representation"]["tpg"]["pEdgeAddition"] =
        params.representation.tpg.pEdgeAddition;
    root["representation"]["tpg"]["pEdgeAddition"].setComment(
        Representation::TPG::TPGParameters::pEdgeAdditionComment, Json::commentBefore);

    root["representation"]["tpg"]["pEdgeDeletion"] =
        params.representation.tpg.pEdgeDeletion;
    root["representation"]["tpg"]["pEdgeDeletion"].setComment(
        Representation::TPG::TPGParameters::pEdgeDeletionComment, Json::commentBefore);

    root["representation"]["tpg"]["pEdgeDestinationChange"] =
        params.representation.tpg.pEdgeDestinationChange;
    root["representation"]["tpg"]["pEdgeDestinationChange"].setComment(
        Representation::TPG::TPGParameters::pEdgeDestinationChangeComment,
        Json::commentBefore);

    root["representation"]["tpg"]["pEdgeDestinationIsAction"] =
        params.representation.tpg.pEdgeDestinationIsAction;
    root["representation"]["tpg"]["pEdgeDestinationIsAction"].setComment(
        Representation::TPG::TPGParameters::pEdgeDestinationIsActionComment,
        Json::commentBefore);
    root["representation"]["tpg"]["pProgramMutation"] =
        params.representation.tpg.pProgramMutation;
    root["representation"]["tpg"]["pProgramMutation"].setComment(
        Representation::TPG::TPGParameters::pProgramMutationComment, Json::commentBefore);



    root["representation"]["atpg"]["probaContextOverActionProgram"] =
        params.representation.atpg.probaContextOverActionProgram;
    root["representation"]["atpg"]["probaContextOverActionProgram"].setComment(
        Representation::ATPG::ATPGParameters::probaContextOverActionProgramComment,
        Json::commentBefore);



    root["representation"]["maple"]["nbActionEdgeInit"] =
        params.representation.maple.nbActionEdgeInit;
    root["representation"]["maple"]["nbActionEdgeInit"].setComment(
        Representation::Maple::MapleParameters::nbActionEdgeInitComment, Json::commentBefore);
    root["representation"]["maple"]["pChangeActionClass"] =
        params.representation.maple.pChangeActionClass;
    root["representation"]["maple"]["pChangeActionClass"].setComment(
        Representation::Maple::MapleParameters::pChangeActionClassComment, Json::commentBefore);
    root["representation"]["maple"]["pActionEdgeAddition"] =
        params.representation.maple.pActionEdgeAddition;
    root["representation"]["maple"]["pActionEdgeAddition"].setComment(
        Representation::Maple::MapleParameters::pActionEdgeAdditionComment,
        Json::commentBefore);
    root["representation"]["maple"]["pActionEdgeDeletion"] =
        params.representation.maple.pActionEdgeDeletion;
    root["representation"]["maple"]["pActionEdgeDeletion"].setComment(
        Representation::Maple::MapleParameters::pActionEdgeDeletionComment,
        Json::commentBefore);
    root["representation"]["maple"]["pMutateActionProgram"] =
        params.representation.maple.pMutateActionProgram;
    root["representation"]["maple"]["pMutateActionProgram"].setComment(
        Representation::Maple::MapleParameters::pMutateActionProgramComment,
        Json::commentBefore);
    root["representation"]["maple"]["pSwapActionProgram"] =
        params.representation.maple.pSwapActionProgram;
    root["representation"]["maple"]["pSwapActionProgram"].setComment(
        Representation::Maple::MapleParameters::pSwapActionProgramComment, Json::commentBefore);
    root["representation"]["maple"]["pCrossIndividuals"] = params.representation.maple.pCrossIndividuals;
    root["representation"]["maple"]["pCrossIndividuals"].setComment(
        Representation::Maple::MapleParameters::pCrossIndividualsComment, Json::commentBefore);
    root["representation"]["maple"]["pCrossPrograms"] =
        params.representation.maple.pCrossPrograms;
    root["representation"]["maple"]["pCrossPrograms"].setComment(
        Representation::Maple::MapleParameters::pCrossProgramsComment, Json::commentBefore);



    // Mutation.program parameters

    root["representation"]["lgp"]["forceProgramBehaviorChangeOnMutation"] =
        params.representation.lgp.forceProgramBehaviorChangeOnMutation;
    root["representation"]["lgp"]["forceProgramBehaviorChangeOnMutation"].setComment(
        Representation::LGP::LGPParameters::forceProgramBehaviorChangeOnMutationComment,
        Json::commentBefore);
    root["representation"]["lgp"]["nbProgramConstant"] = params.representation.lgp.nbProgramConstant;
    root["representation"]["lgp"]["nbProgramConstant"].setComment(
        Representation::LGP::LGPParameters::nbProgramConstantComment,
        Json::commentBefore);
    root["representation"]["lgp"]["nbRegisters"] = params.representation.lgp.nbRegisters;
    root["representation"]["lgp"]["nbRegisters"].setComment(
        Representation::LGP::LGPParameters::nbRegistersComment, Json::commentBefore);
    root["representation"]["lgp"]["maxConstValue"] =
        params.representation.lgp.maxConstValue;
    root["representation"]["lgp"]["maxConstValue"].setComment(
        Representation::LGP::LGPParameters::maxConstValueComment, Json::commentBefore);
    root["representation"]["lgp"]["maxProgramSize"] =
        params.representation.lgp.maxProgramSize;
    root["representation"]["lgp"]["maxProgramSize"].setComment(
        Representation::LGP::LGPParameters::maxProgramSizeComment, Json::commentBefore);
    root["representation"]["lgp"]["initMinProgramSize"] =
        params.representation.lgp.initMinProgramSize;
    root["representation"]["lgp"]["initMinProgramSize"].setComment(
        Representation::LGP::LGPParameters::initMinProgramSizeComment,
        Json::commentBefore);
    root["representation"]["lgp"]["initMaxProgramSize"] =
        params.representation.lgp.initMaxProgramSize;
    root["representation"]["lgp"]["initMaxProgramSize"].setComment(
        Representation::LGP::LGPParameters::initMaxProgramSizeComment,
        Json::commentBefore);
    root["representation"]["lgp"]["minConstValue"] =
        params.representation.lgp.minConstValue;
    root["representation"]["lgp"]["minConstValue"].setComment(
        Representation::LGP::LGPParameters::minConstValueComment, Json::commentBefore);
    root["representation"]["lgp"]["pAdd"] = params.representation.lgp.pAdd;
    root["representation"]["lgp"]["pAdd"].setComment(
        Representation::LGP::LGPParameters::pAddComment, Json::commentBefore);
    root["representation"]["lgp"]["pConstantMutation"] =
        params.representation.lgp.pConstantMutation;
    root["representation"]["lgp"]["pConstantMutation"].setComment(
        Representation::LGP::LGPParameters::pConstantMutationComment,
        Json::commentBefore);
    root["representation"]["lgp"]["pNewProgram"] = params.representation.lgp.pNewProgram;
    root["representation"]["lgp"]["pNewProgram"].setComment(
        Representation::LGP::LGPParameters::pNewProgramComment, Json::commentBefore);
    root["representation"]["lgp"]["pDelete"] = params.representation.lgp.pDelete;
    root["representation"]["lgp"]["pDelete"].setComment(
        Representation::LGP::LGPParameters::pDeleteComment, Json::commentBefore);
    root["representation"]["lgp"]["pMutate"] = params.representation.lgp.pMutate;
    root["representation"]["lgp"]["pMutate"].setComment(
        Representation::LGP::LGPParameters::pMutateComment, Json::commentBefore);
    root["representation"]["lgp"]["pSwap"] = params.representation.lgp.pSwap;
    root["representation"]["lgp"]["pSwap"].setComment(
        Representation::LGP::LGPParameters::pSwapComment, Json::commentBefore);
    root["representation"]["lgp"]["pMutateOutput"] = params.representation.lgp.pMutateOutput;
    root["representation"]["lgp"]["pMutateOutput"].setComment(
        Representation::LGP::LGPParameters::pMutateOutputComment, Json::commentBefore);


    root["representation"]["cgp"]["nbLayers"] = params.representation.cgp.nbLayers;
    root["representation"]["cgp"]["nbLayers"].setComment(
        Representation::CGP::CGPParameters::nbLayersComment, Json::commentBefore);
    root["representation"]["cgp"]["nbNodesPerLayer"] = params.representation.cgp.nbNodesPerLayer;
    root["representation"]["cgp"]["nbNodesPerLayer"].setComment(
        Representation::CGP::CGPParameters::nbNodesPerLayerComment, Json::commentBefore);
    root["representation"]["cgp"]["pMutateNode"] = params.representation.cgp.pMutateNode;
    root["representation"]["cgp"]["pMutateNode"].setComment(
        Representation::CGP::CGPParameters::pMutateNodeComment, Json::commentBefore);
    

    root["representation"]["tgp"]["maxDepth"] = params.representation.tgp.maxDepth;
    root["representation"]["tgp"]["maxDepth"].setComment(
        Representation::TGP::TGPParameters::maxDepthComment, Json::commentBefore);
    root["representation"]["tgp"]["maxInitDepth"] = params.representation.tgp.maxInitDepth;
    root["representation"]["tgp"]["maxInitDepth"].setComment(
        Representation::TGP::TGPParameters::maxInitDepthComment, Json::commentBefore);
    root["representation"]["tgp"]["maxNbEdgePerNode"] = params.representation.tgp.maxNbEdgePerNode;
    root["representation"]["tgp"]["maxNbEdgePerNode"].setComment(
        Representation::TGP::TGPParameters::maxNbEdgePerNodeComment, Json::commentBefore);
    


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
