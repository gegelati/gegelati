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

void Learn::LearningEnvironment::doAction(uint64_t actionID)
{
    if (vectActions.size() > 1) {
        throw std::runtime_error(
            "The vector of actions contain more than one action, doAction() "
            "method should not be called. Use doActions() instead.");
    }

    if (actionID >= this->getNbActions()) {
        throw std::runtime_error("Given action ID exceeds the number of "
                                 "actions for this learning environment.");
    }
}

void Learn::LearningEnvironment::doActions(std::vector<uint64_t> vectActionID)
{
    if (vectActionID.size() != this->vectActions.size()) {
        throw std::runtime_error("Vector of action ID given is not the same "
                                 "size as the vector of actions wanted");
    }

    // If vectActionID contain only one action, the doAction method is called
    // instead
    if (vectActionID.size() == 1) {
        this->doAction(vectActionID[0]);
    }

    for (auto index = 0; index < vectActionID.size(); index++) {
        if (vectActionID[index] >= this->vectActions[index]) {
            throw std::runtime_error("Given action ID " +
                                     std::to_string(index) +
                                     " exceeds the number of "
                                     "actions for this learning environment.");
        }
    }
}
