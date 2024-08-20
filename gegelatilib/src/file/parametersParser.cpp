/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2020 - 2021) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2020 - 2021)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
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
            continue;
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
    if (param == "nbActions") {
        params.mutation.tpg.nbActions = (size_t)value.asUInt();
        return;
    }
    if (param == "nbRoots") {
        params.mutation.tpg.nbRoots = (size_t)value.asUInt();
        return;
    }
    if (param == "initNbRoots") {
        params.mutation.tpg.initNbRoots = (size_t)value.asUInt();
        return;
    }
    if (param == "maxInitOutgoingEdges") {
        params.mutation.tpg.maxInitOutgoingEdges = (size_t)value.asUInt();
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
    if (param == "maxProgramSize") {
        params.mutation.prog.maxProgramSize = (size_t)value.asUInt();
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
        params.mutation.prog.minConstValue = value.asInt();
        return;
    }
    if (param == "maxConstValue") {
        params.mutation.prog.maxConstValue = value.asInt();
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
    if (param == "nbIterationsPerPolicyEvaluation") {
        params.nbIterationsPerPolicyEvaluation = value.asUInt64();
        return;
    }
    if (param == "maxNbActionsPerEval") {
        params.maxNbActionsPerEval = value.asUInt64();
        return;
    }
    if (param == "ratioDeletedRoots") {
        params.ratioDeletedRoots = value.asDouble();
        return;
    }
    if (param == "nbGenerations") {
        params.nbGenerations = value.asUInt64();
        return;
    }
    if (param == "nbIterationsPerJob") {
        params.nbIterationsPerJob = value.asUInt64();
        return;
    }
    if (param == "maxNbEvaluationPerPolicy") {
        params.maxNbEvaluationPerPolicy = (size_t)value.asUInt();
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
    if (param == "doValidation") {
        params.doValidation = value.asBool();
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

    root["nbIterationsPerJob"] = params.nbIterationsPerJob;
    root["nbIterationsPerJob"].setComment(
        Learn::LearningParameters::nbIterationsPerJobComment,
        Json::commentBefore);

    root["nbIterationsPerPolicyEvaluation"] =
        params.nbIterationsPerPolicyEvaluation;
    root["nbIterationsPerPolicyEvaluation"].setComment(
        Learn::LearningParameters::nbIterationsPerPolicyEvaluationComment,
        Json::commentBefore);

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

    root["ratioDeletedRoots"] = params.ratioDeletedRoots;
    root["ratioDeletedRoots"].setComment(
        Learn::LearningParameters::ratioDeletedRootsComment,
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

    root["mutation"]["tpg"]["maxOutgoingEdges"] =
        params.mutation.tpg.maxOutgoingEdges;
    root["mutation"]["tpg"]["maxOutgoingEdges"].setComment(
        Mutator::TPGParameters::maxOutgoingEdgesComment, Json::commentBefore);

    root["mutation"]["tpg"]["nbActions"] = params.mutation.tpg.nbActions;
    root["mutation"]["tpg"]["nbActions"].setComment(
        Mutator::TPGParameters::nbActionsComment, Json::commentBefore);

    root["mutation"]["tpg"]["nbRoots"] = params.mutation.tpg.nbRoots;
    root["mutation"]["tpg"]["nbRoots"].setComment(
        Mutator::TPGParameters::nbRootsComment, Json::commentBefore);

    root["mutation"]["tpg"]["initNbRoots"] = params.mutation.tpg.initNbRoots;
    root["mutation"]["tpg"]["initNbRoots"].setComment(
        Mutator::TPGParameters::initNbRootsComment, Json::commentBefore);

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

    // Mutation.program parameters
    root["mutation"]["prog"]["maxConstValue"] =
        params.mutation.prog.maxConstValue;
    root["mutation"]["prog"]["maxConstValue"].setComment(
        Mutator::ProgramParameters::maxConstValueComment, Json::commentBefore);

    root["mutation"]["prog"]["maxProgramSize"] =
        params.mutation.prog.maxProgramSize;
    root["mutation"]["prog"]["maxProgramSize"].setComment(
        Mutator::ProgramParameters::maxProgramSizeComment, Json::commentBefore);

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

    root["mutation"]["prog"]["pNewProgram"] =
        params.mutation.prog.pNewProgram;
    root["mutation"]["prog"]["pConstantMutation"].setComment(
        Mutator::ProgramParameters::pNewProgramComment,
        Json::commentBefore);

    root["mutation"]["prog"]["pDelete"] = params.mutation.prog.pDelete;
    root["mutation"]["prog"]["pDelete"].setComment(
        Mutator::ProgramParameters::pDeleteComment, Json::commentBefore);

    root["mutation"]["prog"]["pMutate"] = params.mutation.prog.pMutate;
    root["mutation"]["prog"]["pMutate"].setComment(
        Mutator::ProgramParameters::pMutateComment, Json::commentBefore);

    root["mutation"]["prog"]["pSwap"] = params.mutation.prog.pSwap;
    root["mutation"]["prog"]["pSwap"].setComment(
        Mutator::ProgramParameters::pSwapComment, Json::commentBefore);

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
