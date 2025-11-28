/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
 * Thomas Bourgoin <tbourgoi@insa-rennes.fr> (2021)
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

#include <algorithm>
#include <set>
#include <vector>

#include "program/programExecutionEngine.h"
#include "evoGraph/edge.h"

#include "evoGraph/executionEngine.h"

void EvoGraph::ExecutionEngine::setArchive(Archive* newArchive)
{
    this->archive = newArchive;
}

void EvoGraph::ExecutionEngine::applyActivationFunctionOnActions(
    std::vector<double>& actionsTaken)
{

    for (int i = 0; i < actionsTaken.size(); i++) {
        if (std::isnan(actionsTaken[i])) {
            actionsTaken[i] = -std::numeric_limits<double>::infinity();
        }
    }

    // Sigmoid function
    if (env.getParams().activationFunction == "sigmoid") {
        for (size_t i = 0; i < actionsTaken.size(); i++) {
            actionsTaken[i] = 1.0 / (1.0 + std::exp(-actionsTaken[i]));
        }
    }
    else if (env.getParams().activationFunction == "tanh") {
        std::transform(actionsTaken.begin(), actionsTaken.end(),
                       actionsTaken.begin(),
                       [](double x) { return std::tanh(x); });
    }
    else if (env.getParams().activationFunction == "none") {
        for (double& actionTaken : actionsTaken) {
            actionTaken = std::clamp(actionTaken, -1.0, 1.0);
        }
    }
    else {
        throw std::runtime_error(
            "Activation function for converting continuous actions not known");
    }
}

double EvoGraph::ExecutionEngine::evaluateEdge(const Edge& edge)
{
    // Get the program
    Program::Program& prog = edge.getProgram();

    // Set the progExecutionEngine to the program
    this->progExecutionEngine.setProgram(prog);

    // Execute the program.
    double result = this->progExecutionEngine.executeProgram();

    // Filter NaN results: replace with -inf
    result = (std::isnan(result)) ? -std::numeric_limits<double>::infinity()
                                  : result;

    // Put the result in the archive before returning it.
    if (this->archive != NULL) {
        this->archive->addRecording(&prog, progExecutionEngine.getDataSources(),
                                    result);
    }

    return result;
}

const EvoGraph::Edge& EvoGraph::ExecutionEngine::evaluateTeam(const TPGTeam& team)
{
    // Copy outgoing edge list
    const std::list<EvoGraph::Edge*>& outgoingEdges = team.getOutgoingEdges();

    // Note: No need to exclude previously visited edges as the graph is now
    // assumed to be acyclic.

#ifdef DEBUG
    std::cout << "New team :" << &team << std::endl;
#endif

    // Evaluate all Edge
    // First
    Edge* bestEdge = *outgoingEdges.begin();
    double bestBid = this->evaluateEdge(*bestEdge);
#ifdef DEBUG
    std::cout << "R = " << bestBid << "*" << std::endl;
#endif
    // Others
    for (auto iter = ++outgoingEdges.begin(); iter != outgoingEdges.end();
         iter++) {
        Edge* edge = *iter;
        double bid = this->evaluateEdge(*edge);
#ifdef DEBUG
        std::cout << "R = " << bid;
#endif
        if (bid >= bestBid) {
#ifdef DEBUG
            std::cout << "*" << std::endl;
#endif
            bestEdge = edge;
            bestBid = bid;
        }
        else {
#ifdef DEBUG
            std::cout << std::endl;
#endif
        }
    }

    return *bestEdge;
}

const std::pair<std::vector<const EvoGraph::Vertex*>, std::vector<double>> EvoGraph::
    ExecutionEngine::executeFromRoot(
        const Vertex& root, const std::vector<uint64_t>& initActions)
{
    const Vertex* currentVertex = &root;
    const Edge* edge = nullptr;

    std::vector<const Vertex*> visitedVertices;
    visitedVertices.push_back(currentVertex);
    // Browse the TPG until a Action is reached.
    while (dynamic_cast<const EvoGraph::TPGTeam*>(currentVertex)) {
        // Get the next edge
        edge = &this->evaluateTeam(*(const TPGTeam*)currentVertex);
        Program::Program p =
            currentVertex->getOutgoingEdges().front()->getProgram();
        // update currentVertex and backup in visitedVertex.
        if (edge->getDestination() != nullptr) {
            currentVertex = edge->getDestination();
        }
        visitedVertices.push_back(currentVertex);
    }

    // An action value must be positive, so -1 for an action mean that no action
    // value is choosen yet.
    std::vector<double> actionsTaken(env.getNbContinuousActions(), 0.0);
    // If continuous action are used, the n actions taken are the value 1 to n+1
    // in the last executed register.
    if (env.getNbContinuousActions() > 0) {

        // True if the action contain multiple ActionEdge
        if (env.getParams().mutation.tpg.useMultiActionProgram) {

            if (currentVertex != nullptr) {
                for (auto edge : currentVertex->getOutgoingEdges()) {
                    auto actionEdge = dynamic_cast<ActionEdge*>(edge);

                    // Evaluate the edge and set the action value
                    actionsTaken[actionEdge->getActionClass()] =
                        this->evaluateEdge(*edge);
                }
            }
        }
        // True if the action contain one ActionEdge
        else if (env.getParams().mutation.tpg.useActionProgram) {

            if (currentVertex != nullptr) {

                if (currentVertex->getOutgoingEdges().size() != 1) {
                    throw std::runtime_error(
                        "Current vertex is a Action, it "
                        "should have exactly one edge.");
                }

                this->evaluateEdge(*currentVertex->getOutgoingEdges().front());

                Program::Program p =
                    currentVertex->getOutgoingEdges().front()->getProgram();

                // Get the register values
                actionsTaken = progExecutionEngine.getRegisterValues(
                    env.getNbContinuousActions());
            }
        }
        else {
            if (edge != nullptr) {
                // Re-evaluate the last edge to get the register values.
                // TODO Wont work if memory is added
                this->evaluateEdge(*edge);
            }

            // Get the register values + the bid and erase the bid
            actionsTaken = progExecutionEngine.getRegisterValues(
                env.getNbContinuousActions() + 1);
            actionsTaken.erase(actionsTaken.begin());
        }

        this->applyActivationFunctionOnActions(actionsTaken);

        return std::make_pair(visitedVertices, actionsTaken);
    }
    else {
        std::vector<double> actionID;
        if (currentVertex != nullptr) {
            actionID.push_back(
                (double)dynamic_cast<const EvoGraph::Action*>(currentVertex)
                    ->getActionID());
        }
        return std::make_pair(visitedVertices, actionID);
    }
}
