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

#include "file/parametersParser.h"

void File::ParametersParser::readConfigFile(const char* path, Json::Value& root) {
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

void File::ParametersParser::setAllParamsFrom(const Json::Value& root, Learn::LearningParameters& params) {
	for (std::string& key : root.getMemberNames()) {
		if (key == "mutation") {
			// we have a subtree of mutation : parameters like mutation.xxx.xxx
			for (std::string const& key2 : root[key].getMemberNames()) {
				if (key2 == "tpg") {
					// we're on a mutation.tpg.xxx parameter
					for (std::string& key3 : root[key][key2].getMemberNames()) {
						double value = root[key][key2][key3].asDouble();
						setParameterFromString(params, key3, value);
					}
				}
				else {
					if (key2 == "prog") {
						// we're on a mutation.prog.xxx parameter
						for (std::string& key3 : root[key][key2].getMemberNames()) {
							double value = root[key][key2][key3].asDouble();
							setParameterFromString(params, key3, value);
						}
					}
				}
			}
			continue;
		}
		if (root[key].size() == 0) {
			// we have a parameter without subtree (as a leaf)
			double value = root[key].asDouble();
			setParameterFromString(params, key, value);
		}
	}
}

void File::ParametersParser::setParameterFromString(Learn::LearningParameters& params, std::string& key, double value) {
	if (key == "nbActions") {
		params.mutation.tpg.nbActions = (size_t)value;
		return;
	}
	if (key == "nbRoots") {
		params.mutation.tpg.nbRoots = (size_t)value;
		return;
	}
	if (key == "maxInitOutgoingEdges") {
		params.mutation.tpg.maxInitOutgoingEdges = (size_t)value;
		return;
	}
	if (key == "maxOutgoingEdges") {
		params.mutation.tpg.maxOutgoingEdges = (size_t)value;
		return;
	}
	if (key == "pEdgeDeletion") {
		params.mutation.tpg.pEdgeDeletion = value;
		return;
	}
	if (key == "pEdgeAddition") {
		params.mutation.tpg.pEdgeAddition = value;
		return;
	}
	if (key == "pProgramMutation") {
		params.mutation.tpg.pProgramMutation = value;
		return;
	}
	if (key == "pEdgeDestinationChange") {
		params.mutation.tpg.pEdgeDestinationChange = value;
		return;
	}
	if (key == "pEdgeDestinationIsAction") {
		params.mutation.tpg.pEdgeDestinationIsAction = value;
		return;
	}
	if (key == "maxProgramSize") {
		params.mutation.prog.maxProgramSize = (size_t)value;
		return;
	}
	if (key == "pDelete") {
		params.mutation.prog.pDelete = value;
		return;
	}
	if (key == "pAdd") {
		params.mutation.prog.pAdd = value;
		return;
	}
	if (key == "pMutate") {
		params.mutation.prog.pMutate = value;
		return;
	}
	if (key == "pSwap") {
		params.mutation.prog.pSwap = value;
		return;
	}
	if (key == "archiveSize") {
		params.archiveSize = (size_t)value;
		return;
	}
	if (key == "archivingProbability") {
		params.archivingProbability = value;
		return;
	}
	if (key == "nbIterationsPerPolicyEvaluation") {
		params.nbIterationsPerPolicyEvaluation = (uint64_t)value;
		return;
	}
	if (key == "maxNbActionsPerEval") {
		params.maxNbActionsPerEval = (uint64_t)value;
		return;
	}
	if (key == "ratioDeletedRoots") {
		params.ratioDeletedRoots = value;
		return;
	}
	if (key == "nbGenerations") {
		params.nbGenerations = (uint64_t)value;
		return;
	}
	if (key == "maxNbEvaluationPerPolicy") {
		params.maxNbEvaluationPerPolicy = (size_t)value;
		return;
	}
	if (key == "nbRegisters") {
		params.nbRegisters = (size_t)value;
		return;
	}
	if (key == "nbThreads") {
		params.nbThreads = (size_t)value;
		return;
	}
	// we didn't recognize the symbol
	std::cerr << "Ignoring unknown parameter " << key << std::endl;
}

void File::ParametersParser::loadParametersFromJson(const char* path, Learn::LearningParameters& params) {
	Json::Value root;
	readConfigFile(path, root);

	setAllParamsFrom(root, params);
}