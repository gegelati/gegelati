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

#include "evoGraph/team.h"
#include "evoGraph/action.h"
#include "evoGraph/actionEdge.h"
#include <stdexcept>

void EvoGraph::Team::addOutgoingEdge(std::shared_ptr<const Edge> edge)
{
    if (std::dynamic_pointer_cast<const ActionEdge>(edge) != nullptr) {
        throw std::runtime_error(
            "Cannot add an action edge to an Action team.");
    }
    else {
        Vertex::addOutgoingEdge(edge);
    }
}


void EvoGraph::Team::orderActionEdges()
{
    // Take all the edges of the team, put the edges pointing towards team first, in the same order, then edges pointing towards action vertex, in order of actionID.
    this->outgoingEdges.sort([](std::shared_ptr<const EvoGraph::Edge> edge1, std::shared_ptr<const EvoGraph::Edge> edge2) {
        auto dest1 = edge1->getDestination();
        auto dest2 = edge2->getDestination();

        if(std::dynamic_pointer_cast<const EvoGraph::Team>(dest1) != nullptr &&
           std::dynamic_pointer_cast<const EvoGraph::Action>(dest2) != nullptr) {
            return true;
        }
        else if(std::dynamic_pointer_cast<const EvoGraph::Action>(dest1) != nullptr &&
                std::dynamic_pointer_cast<const EvoGraph::Team>(dest2) != nullptr) {
            return false;
        }
        else if(auto action1 = std::dynamic_pointer_cast<const EvoGraph::Action>(dest1)){

            if(auto action2 = std::dynamic_pointer_cast<const EvoGraph::Action>(dest2)){
                return action1->getActionID() < action2->getActionID();
            }
        }
        return false;
    });
}


void EvoGraph::Team::updateAssessedActions()
{
    assessedActions.clear();
    for (std::shared_ptr<const Edge> edge : this->outgoingEdges) {
        // Otherwise, insert all assessed actions from the destination
        const auto& destinationActions =
            edge->getDestination()->getAssessedActions();
        assessedActions.insert(destinationActions.begin(),
                                destinationActions.end());
    }
}