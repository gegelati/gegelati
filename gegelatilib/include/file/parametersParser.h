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

#ifndef PARAMETERS_PARSER_H
#define PARAMETERS_PARSER_H

#include <string>

#include "learn/learningParameters.h"

/** Used to avoid importing the JsonCpp lib now,
 * so it tells the compiler Json::Value exists.
 */
namespace Json {
    class Value;
}

namespace File {
    /**
    * \brief Namespace containing the functions for filling an instance of the
    * Learn::LearningParameters class from a Json file.
    */
    namespace ParametersParser {
        /**
        * \brief Loads a given json file and fills the parameters it contains in
        * given LearningParameters.
        *
        * High level method that simply calls more complicated ones as follow :
        * - readConfigFile to get the derivative tree from a JSON file path
        * - setAllParamsFrom to set the parameters given the obtained tree.
        *
        * \param[in] path path of the JSON file from which the parameters are
        *            read.
        * \param[out] params the LearningParameters being updated.
        */
        void loadParametersFromJson(const char *path,
                                    Learn::LearningParameters &params);

        /**
        * \brief Given a parameter name, sets its value in given
        * LearningParameters.
        *
        * To find the right parameter, the method contains a lot of if
        * statements, each of them finishing by a return. These statements
        * compare the given parameter name to known parameters names.
        * If a parameter is found, it casts value to the right type and sets
        * the given parameter to this value.
        * If no parameter was found, it simply ignores the input and logs it
        * explicitly.
        *
        * \param[out] params the learning parameters we are going to set.
        * \param[in] param the name of the LearningParameters being updated.
        * \param[in] value the value we want to set the parameter to.
        */
        void setParameterFromString(Learn::LearningParameters &params,
                                    std::string &param, Json::Value value);

        /**
        * \brief Puts the parameters described in the derivative tree root in
        * given LearningParameters.
        *
        * Browses the JSON tree. If the node we're looking at is a leaf,
        * we call setParameterFromString. Otherwise, we browe its children to
        * follow the known parameters structure.
        *
        * \param[in] root JSON tree we will use to set parameters.
        * \param[out] params the LearningParameters being updated.
        */
        void setAllParamsFrom(const Json::Value &root,
                              Learn::LearningParameters &params);

        /**
        * \brief Reads a given json file and puts the derivative tree in root.
        *
        * Opens the file and calls the parseFromStream() method from JsonCpp
        * which handles all the parsing of the JSON file. It eventually returns
        * errors in a parameter, e.g. if the file does not respect JSON format.
        * In this case, the file is simply ignored and it is logged explicitly.
        * However, in case of JsonCpp internal errors, there can be exceptions,
        * as described in throws.
        *
        * \param[in] path path of the JSON file from which the parameters are
        *            read.
        * \param[out] root JSON tree we are going to build with the file.
        * \throws std::exception if json parser settings are not in their
        * right formats.
        */
        void readConfigFile(const char *path, Json::Value &root);
    }
}

#endif 