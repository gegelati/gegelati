/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
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

#include "tpg/tpgAction.h"
#include <stdexcept>

void TPG::TPGAction::addOutgoingEdge(TPGEdge* edge)
{
    if (dynamic_cast<TPGActionEdge*>(edge) == nullptr) {
        throw std::runtime_error(
            "Cannot add an outgoing edge to an Action vertex.");
    }
    else {
        TPGVertex::addOutgoingEdge(edge);
    }
}

void TPG::TPGAction::orderActionEdges()
{

    this->outgoingEdges.sort([](TPG::TPGEdge* edge1, TPG::TPGEdge* edge2) {
        // Use static_cast to convert TPGEdge* into TPGActionEdge*
        TPG::TPGActionEdge* actionEdge1 =
            static_cast<TPG::TPGActionEdge*>(edge1);
        TPG::TPGActionEdge* actionEdge2 =
            static_cast<TPG::TPGActionEdge*>(edge2);

        // Compare actionClass
        return actionEdge1->getActionClass() < actionEdge2->getActionClass();
    });
}

TPG::TPGActionEdge* TPG::TPGAction::getEdgeOfAction(uint64_t actionClass) const
{

    // Search the edge with the searched action class
    auto it = std::find_if(
        outgoingEdges.begin(), outgoingEdges.end(),
        [actionClass](TPG::TPGEdge* edge) {
            return static_cast<TPG::TPGActionEdge*>(edge)->getActionClass() ==
                   actionClass;
        });

    // If action found, return the shared pointer, else return nullptr
    if (it != outgoingEdges.end()) {
        return (TPG::TPGActionEdge*)(*it);
    }
    else {
        return nullptr;
    }
}
