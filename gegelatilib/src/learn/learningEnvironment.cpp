/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
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
#include <stdexcept>

#include "learn/learningEnvironment.h"

Learn::LearningEnvironment* Learn::LearningEnvironment::clone() const
{
    return NULL;
}

bool Learn::LearningEnvironment::isCopyable() const
{
    return false;
}

bool Learn::LearningEnvironment::isUsingUtility() const
{
    return false;
}

void Learn::LearningEnvironment::doAction(double actionID)
{
    if (nbActions > 1 && !isDiscreteEnvironment) {
        throw std::runtime_error(
            "With more than one continuous action, doAction() "
            "method should not be called. Use doActions() instead.");
    }

    if (actionID >= this->getNbActions()) {
        throw std::runtime_error("Given action ID exceeds the number of "
                                 "actions for this learning environment.");
    }
}

void Learn::LearningEnvironment::doActions(std::vector<double> vectActionID)
{

    // If vectActionID contain only one action, the doAction method is called
    // instead
    if (vectActionID.size() == 1) {
        this->doAction(vectActionID[0]);
    }
    else {
        if (isDiscreteEnvironment) {
            throw std::runtime_error(
                "Gegelati does not support multiple Discrete actions for now");
        }

        if (vectActionID.size() != nbActions) {
            throw std::runtime_error(
                "Vector of action ID given is not the same "
                "size as the number of actions wanted");
        }
    }
}

double Learn::LearningEnvironment::getUtility() const
{
    throw std::runtime_error(
        "Utility should not be call except if 'isUsingUtility' is override to "
        "return true."
        "\nIn that case, this method should be override too.");
}
