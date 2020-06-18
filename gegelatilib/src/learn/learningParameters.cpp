/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019)
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

#include <iostream>
#include <fstream>
#include <json.h>
#include "learn/learningParameters.h"

void Learn::readConfigFile(const char* path, Json::Value &root) {
    std::ifstream ifs;
    ifs.open(path);

    if (!ifs.is_open()) {
        std::cerr << "Error : specified param file doesn't exist : " << path << std::endl;
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

void Learn::setAllParamsFrom(const Json::Value& root, LearningParameters *params) {
    for (std::string const &key : root.getMemberNames()) {
        if (key == "mutation") {
            // we have a subtree of mutation : parameters like mutation.xxx.xxx
            for (std::string const &key2 : root[key].getMemberNames()) {
                if (key2 == "tpg") {
                    // we're on a mutation.tpg.xxx parameter
                    for (std::string const &key3 : root[key][key2].getMemberNames()) {
                        double param = root[key][key2][key3].asDouble();
                        if (key3 == "nbActions") {
                            params->mutation.tpg.nbActions = param;
                            continue;
                        }
                        if (key3 == "nbRoots") {
                            params->mutation.tpg.nbRoots = param;
                            continue;
                        }
                        if (key3 == "maxInitOutgoingEdges") {
                            params->mutation.tpg.maxInitOutgoingEdges = param;
                            continue;
                        }
                        if (key3 == "maxOutgoingEdges") {
                            params->mutation.tpg.maxOutgoingEdges = param;
                            continue;
                        }
                        if (key3 == "pEdgeDeletion") {
                            params->mutation.tpg.pEdgeDeletion = param;
                            continue;
                        }
                        if (key3 == "pEdgeAddition") {
                            params->mutation.tpg.pEdgeAddition = param;
                            continue;
                        }
                        if (key3 == "pProgramMutation") {
                            params->mutation.tpg.pProgramMutation = param;
                            continue;
                        }
                        if (key3 == "pEdgeDestinationChange") {
                            params->mutation.tpg.pEdgeDestinationChange = param;
                            continue;
                        }
                        if (key3 == "pEdgeDestinationIsAction") {
                            params->mutation.tpg.pEdgeDestinationIsAction = param;
                            continue;
                        }
                    }
                } else {
                    if (key2 == "prog") {
                        // we're on a mutation.prog.xxx parameter
                        for (std::string const &key3 : root[key][key2].getMemberNames()) {
                            double param = root[key][key2][key3].asDouble();
                            if (key3 == "maxProgramSize") {
                                params->mutation.prog.maxProgramSize = param;
                                continue;
                            }
                            if (key3 == "pDelete") {
                                params->mutation.prog.pDelete = param;
                                continue;
                            }
                            if (key3 == "pAdd") {
                                params->mutation.prog.pAdd = param;
                                continue;
                            }
                            if (key3 == "pMutate") {
                                params->mutation.prog.pMutate = param;
                                continue;
                            }
                            if (key3 == "pSwap") {
                                params->mutation.prog.pSwap = param;
                                continue;
                            }
                        }
                    }
                }
            }
            continue;
        }
        if (root[key].size() == 0) {
            // we have a parameter without subtree (as a leaf)
            double param = root[key].asDouble();
            if (key == "archiveSize") {
                params->archiveSize = param;
                continue;
            }
            if (key == "archivingProbability") {
                params->archivingProbability = param;
                continue;
            }
            if (key == "nbIterationsPerPolicyEvaluation") {
                params->nbIterationsPerPolicyEvaluation = param;
                continue;
            }
            if (key == "maxNbActionsPerEval") {
                params->maxNbActionsPerEval = param;
                continue;
            }
            if (key == "ratioDeletedRoots") {
                params->ratioDeletedRoots = param;
                continue;
            }
            if (key == "nbGenerations") {
                params->nbGenerations = param;
                continue;
            }
            if (key == "maxNbEvaluationPerPolicy") {
                params->maxNbEvaluationPerPolicy = param;
                continue;
            }
            if (key == "nbRegisters") {
                params->nbRegisters = param;
                continue;
            }
            if (key == "nbThreads") {
                params->nbThreads = param;
                continue;
            }
        }
        // we didn't recognize the symbol
        std::cerr << "Ignoring unknown parameter " << key << std::endl;
    }
}

void Learn::loadParametersFromJson(const char* path, LearningParameters *params) {
    Json::Value root;
    readConfigFile(path, root);

    setAllParamsFrom(root, params);
}