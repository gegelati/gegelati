/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
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

#include <algorithm>
#include <array>
#include <mutex>
#include <numeric>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

#include "archive.h"

#include "program/programExecutionEngine.h"
#include "tpg/tpgAction.h"
#include "tpg/tpgEdge.h"
#include "tpg/tpgGraph.h"
#include "tpg/tpgTeam.h"

#include "mutator/mutationParameters.h"
#include "mutator/programMutator.h"
#include "mutator/rng.h"
#include "mutator/tpgMutator.h"

const EvoGraph::Action* Mutator::TPGMutator::initActionVertex(
    EvoGraph::Graph& graph, const Mutator::MutationParameters& params,
    RNG::RNG& rng, uint64_t nbActionEdgeInit, uint64_t actionID)
{
    const EvoGraph::Action* action = &(graph.addNewAction(actionID));

    std::set<uint64_t> actionUsed;
    for (size_t j = 0; j < nbActionEdgeInit; j++) {

        // Create a program and specify action program
        std::shared_ptr<Program::Program> p =
            std::make_shared<Program::Program>(graph.getEnvironment(), true);

        // RandomInit the Programs
        Mutator::ProgramMutator::initRandomProgram(*p, params, rng);

        // Find an action class not already used by this action
        uint64_t actionClass;
        do {
            actionClass = rng.getUnsignedInt64(
                0, graph.getEnvironment().getNbContinuousActions() - 1);
        } while (actionUsed.find(actionClass) != actionUsed.end());
        actionUsed.insert(actionClass);

        // Create the action edge
        graph.addNewActionEdge(*action, p, actionClass);
    }

    if (params.tpg.useMultiActionProgram) {
        graph.orderActionEdges(action);
    }

    return action;
}

void Mutator::TPGMutator::initRandomTPG(
    EvoGraph::Graph& graph, const Mutator::MutationParameters& params,
    RNG::RNG& rng, uint64_t nbActions)
{

    // Number of action edge per action vertex.
    uint64_t nbActionEdgeInit = params.tpg.nbActionEdgeInit;
    uint64_t initNbTeams =
        (uint64_t)(params.tpg.ratioTeamsOverActions * params.tpg.nbRoots);
    uint64_t initNbActions = nbActions;
    uint64_t nbActionNotRoots = nbActions;

    if (graph.getEnvironment().getNbContinuousActions() == 0) {
        nbActionEdgeInit = 0;

        if (params.tpg.maxInitOutgoingEdges > nbActions) {
            throw std::runtime_error("Maximum initial number of outgoing edges "
                                     "cannot exceed the number of action");
        }
        if (nbActions < 2) {
            throw std::runtime_error(
                "A TPG with a single action makes no sense.");
        }

        if (params.tpg.useActionProgram || params.tpg.useMultiActionProgram) {
            throw std::runtime_error(
                "Cannot use action program or multi action program with no "
                "action continuous available.");
        }
    }
    else if (graph.getEnvironment().getParams().nbRegisters <
                 nbActions + (int)!params.tpg.useActionProgram &&
             !params.tpg.useMultiActionProgram) {
        throw std::runtime_error(
            "The number of registers is below the number of values outputted.");
    }
    else {

        if (params.tpg.useActionProgram && !params.tpg.teamAccessAllActions &&
            params.tpg.ratioTeamsOverActions == 1.0) {
            throw std::runtime_error("If there is no action roots, teams "
                                     "should have access to all actions");
        }

        // No action edge.
        if (!params.tpg.useActionProgram) {
            nbActionEdgeInit = 0;
            // Only one action edge.
        }
        else if (!params.tpg.useMultiActionProgram) {
            nbActionEdgeInit = 1;
        }

        if (nbActionEdgeInit >
            graph.getEnvironment().getNbContinuousActions()) {
            throw std::runtime_error(
                "Maximum initial number of outgoing action edges "
                "cannot exceed the number of action");
        }

        nbActionNotRoots = 2 * initNbTeams;
        initNbActions = params.tpg.nbRoots - initNbTeams + nbActionNotRoots;
    }

    if (params.tpg.maxInitOutgoingEdges < 2 && initNbTeams > 0) {
        throw std::runtime_error(
            "A team should have at least two edges at initialisation.");
    }

    if (!params.tpg.useActionProgram &&
        params.tpg.ratioTeamsOverActions != 1.0) {
        throw std::runtime_error(
            "The ratio of team should be one when no action program are used "
            "(we do not want action root at the initialisation).");
    }

    // Empty graph
    graph.clear();

    // Create teams, programs and Actions
    std::vector<const EvoGraph::Action*> actions;
    std::vector<const EvoGraph::TPGTeam*> teams;
    std::vector<std::shared_ptr<Program::Program>> programs;

    for (size_t i = 0; i < initNbActions; i++) {
        actions.push_back(
            initActionVertex(graph, params, rng, nbActionEdgeInit, i));
    }
    for (size_t i = 0; i < initNbTeams; i++) {
        teams.push_back(&(graph.addNewTeam()));
    }

    // Connect each team with two distinct actions, through two distinct
    // programs Association here are determinists since randomness would
    // uselessly complicate the code while bringing no real value since anyway,
    // Programs have been initialized randomly.
    for (size_t i = 0; i < 2 * initNbTeams; i++) {
        // Create a program and specify context program
        programs.emplace_back(
            new Program::Program(graph.getEnvironment(), false));
        // RandomInit the Programs
        Mutator::ProgramMutator::initRandomProgram(*programs.back(), params,
                                                   rng);
        graph.addNewEdge(*teams.at(i / 2), *actions.at(i % nbActionNotRoots),
                         programs.at(i));
    }

    // Add additional connections to TPG
    // Team-by-Team
    for (const EvoGraph::TPGTeam* team : teams) {
        // Pick a number of additional outedge
        size_t nbAdditionalEdges =
            rng.getUnsignedInt64(0, params.tpg.maxInitOutgoingEdges - 2);

        // For each additional edge to add
        for (uint64_t i = 0; i < nbAdditionalEdges; i++) {
            // Pick 2 random programs not already used by the Team
            int64_t randomProgIndex[2] = {-1, -1};
            int pickedProgram = 0;
            {
                // Copy the list of programs
                std::vector<int> availableChoices(programs.size());
                std::iota(availableChoices.begin(), availableChoices.end(), 0);
                // Remove already connected ones
                auto iter = availableChoices.begin();
                while (iter < availableChoices.end()) {
                    if (std::count_if(
                            team->getOutgoingEdges().begin(),
                            team->getOutgoingEdges().end(),
                            [&iter, &programs](const EvoGraph::Edge* edge) {
                                return &edge->getProgram() ==
                                       programs.at(*iter).get();
                            }) > 0) {
                        iter = availableChoices.erase(iter);
                    }
                    else {
                        iter++;
                    }
                }

                // Pick two programs (if possible, maybe only one is available)
                for (int i = 0; i < 2 && availableChoices.size() > 0; i++) {
                    uint64_t progNr =
                        rng.getUnsignedInt64(0, availableChoices.size() - 1);
                    randomProgIndex[i] = availableChoices.at(progNr);
                    availableChoices.erase(availableChoices.begin() + progNr);
                    pickedProgram++;
                }
            }
            // Select the least used program for the connection
            uint64_t selectedProgramIndex =
                (pickedProgram > 1 &&
                 programs.at(randomProgIndex[1]).use_count() <
                     programs.at(randomProgIndex[0]).use_count())
                    ? randomProgIndex[1]
                    : randomProgIndex[0];

            // Add the connection
            graph.addNewEdge(
                *team,
                *actions.at(rng.getUnsignedInt64(0, nbActionNotRoots - 1)),
                programs.at(selectedProgramIndex));
        }
    }

    if (params.tpg.useMultiActionProgram) {
        graph.updateAllAssessedActions();
    }
}

void Mutator::TPGMutator::removeRandomActionEdge(EvoGraph::Graph& graph,
                                                 const EvoGraph::Action& action,
                                                 RNG::RNG& rng)
{
    // Pick an outgoing edge randomly,
    const std::list<EvoGraph::Edge*>& pickableEdges = action.getOutgoingEdges();

    // Note: No need to take special care of Actions. Since cycles can not
    // appear in TPG with the current mutation process, there is no need to
    // maintain an action within each team.

    // Pick a random edge
    auto iterSet = pickableEdges.begin();
    std::advance(iterSet, rng.getUnsignedInt64(0, pickableEdges.size() - 1));
    const EvoGraph::Edge* removedEdge = *iterSet;
    graph.removeActionEdge(*removedEdge);
}

void Mutator::TPGMutator::addRandomActionEdge(
    EvoGraph::Graph& graph, const EvoGraph::Action& action,
    const Selector::SelectionContext* context, RNG::RNG& rng)
{
    // Pick an edge (excluding ones from the team and edges with the team as a
    // destination)
    auto pickableEdges(context->preExistingEdges);
    // cf erase-remove idiom
    pickableEdges.erase(
        std::remove_if(pickableEdges.begin(), pickableEdges.end(),
                       [&action](const EvoGraph::Edge* edge) -> bool {
                           if (dynamic_cast<const EvoGraph::ActionEdge*>(edge) !=
                                   nullptr &&
                               action.getAssessedActions().find(
                                   dynamic_cast<const EvoGraph::ActionEdge*>(edge)
                                       ->getActionClass()) ==
                                   action.getAssessedActions().end()) {
                               return edge->getSource() == &action;
                           }
                           else {
                               return true;
                           }
                       }),
        pickableEdges.end());

    if (pickableEdges.size() == 0) {
        // Chances are really low but the pickableEdges can be empty
        return;
    }

    // Pick a pickable Edge
    // (This code assumes that the set of pickable edge is never empty..
    // otherwise it will throw an exception. Possible solution if needed
    // initialize an entirely new program and pick a random target.)
    std::list<const EvoGraph::Edge*>::iterator iter = pickableEdges.begin();
    std::advance(iter, rng.getUnsignedInt64(0, pickableEdges.size() - 1));
    const EvoGraph::Edge* pickedEdge = *iter;

    // Create new edge from team and with the same ProgramSharedPointer
    // But with the team as its source
    // throw std::runtime_error if the edge is not from the graph;
    const EvoGraph::Edge& newEdge = graph.cloneEdge(*pickedEdge);
    graph.setEdgeSource(newEdge, action);
}

void Mutator::TPGMutator::swapActionEdges(EvoGraph::Graph& graph,
                                          const EvoGraph::Action& action,
                                          RNG::RNG& rng)
{

    // Randomly select two edges
    size_t index1 =
        rng.getUnsignedInt64(0, action.getOutgoingEdges().size() - 1);
    size_t index2 =
        rng.getUnsignedInt64(0, action.getOutgoingEdges().size() - 2);
    if (index2 == index1) {
        index2++;
    }

    // Get iterators to the selected edges
    auto it1 = action.getOutgoingEdges().begin();
    std::advance(it1, index1);
    auto it2 = action.getOutgoingEdges().begin();
    std::advance(it2, index2);

    EvoGraph::Edge* edge1 = *it1;
    EvoGraph::Edge* edge2 = *it2;

    // Extract and swap action classes
    auto actionClass1 =
        dynamic_cast<EvoGraph::ActionEdge*>(edge1)->getActionClass();
    auto actionClass2 =
        dynamic_cast<EvoGraph::ActionEdge*>(edge2)->getActionClass();

    graph.setActionClassEdge(edge1, actionClass2);
    graph.setActionClassEdge(edge2, actionClass1);
}

void Mutator::TPGMutator::mutateActionEdge(
    EvoGraph::Graph& graph, const EvoGraph::Action& action,
    EvoGraph::ActionEdge* actionEdge,
    std::list<std::shared_ptr<Program::Program>>& newPrograms,
    const Mutator::MutationParameters& params, RNG::RNG& rng)
{

    // copy program
    std::shared_ptr<Program::Program> newProg(
        new Program::Program(*actionEdge->getProgramSharedPointer(), true));

    // Add it to the list of new Program to be mutated.
    newPrograms.push_back(newProg);

    // Set the mutated program to the edge
    actionEdge->setProgram(newProg);

    // Change action ID randomly if the action do not contain all actions.
    if (params.tpg.useMultiActionProgram &&
        action.getAssessedActions().size() <
            graph.getEnvironment().getNbContinuousActions() &&
        params.tpg.pChangeActionClass > rng.getDouble(0.0, 1.0)) {

        uint64_t newActionID = rng.getUnsignedInt64(
            0, graph.getEnvironment().getNbContinuousActions() - 1);
        while (action.getAssessedActions().find(newActionID) !=
               action.getAssessedActions().end()) {
            newActionID = rng.getUnsignedInt64(
                0, graph.getEnvironment().getNbContinuousActions() - 1);
        }

        actionEdge->setActionClass(newActionID);

        graph.updateAssessedActions(&action);
    }
}

void Mutator::TPGMutator::mutateAction(
    EvoGraph::Graph& graph, const EvoGraph::Action& action,
    const Selector::SelectionContext* context,
    std::list<std::shared_ptr<Program::Program>>& newPrograms,
    const Mutator::MutationParameters& params, RNG::RNG& rng)
{
    if (params.tpg.useMultiActionProgram) {
        // 1. Remove randomly selected edges
        // Keep at least two edges (otherwise the team is useless)
        double proba = params.tpg.pActionEdgeDeletion;
        while (action.getOutgoingEdges().size() > 1 &&
               proba > rng.getDouble(0.0, 1.0)) {
            removeRandomActionEdge(graph, action, rng);

            // Decrement the proba of removing another edge
            proba *= params.tpg.pActionEdgeDeletion;

            // Update assessed actions
            graph.updateAssessedActions(&action);
        }

        // 2. Add random duplicated edge with the team as its source
        proba = params.tpg.pActionEdgeAddition;
        while (action.getOutgoingEdges().size() <
                   graph.getEnvironment().getNbContinuousActions() &&
               proba > rng.getDouble(0.0, 1.0)) {
            // Add an edge (by duplication of an existing one)
            addRandomActionEdge(graph, action, context, rng);

            // Decrement the proba of adding another edge
            proba *= params.tpg.pActionEdgeAddition;

            // Update assessed actions
            graph.updateAssessedActions(&action);
        }

        // 3. swap randomly selected edges
        // With at least two edges
        proba = params.tpg.pSwapActionProgram;
        while (action.getOutgoingEdges().size() > 2 &&
               proba > rng.getDouble(0.0, 1.0)) {
            swapActionEdges(graph, action, rng);

            // Decrement the proba of swapping two edges
            proba *= params.tpg.pSwapActionProgram;
        }
    }

    bool anyMutationDone = false;
    do {
        std::vector<uint64_t> indexUsed;
        uint64_t index;
        // 4. mutate randomly selected program on action Edge.
        double proba = params.tpg.pMutateActionProgram;
        while (indexUsed.size() < action.getOutgoingEdges().size() &&
               proba > rng.getDouble(0.0, 1.0)) {

            do {
                index = rng.getUnsignedInt64(
                    0, action.getOutgoingEdges().size() - 1);
            } while (std::find(indexUsed.begin(), indexUsed.end(), index) !=
                     indexUsed.end());

            indexUsed.push_back(index);

            std::list<EvoGraph::Edge*>::const_iterator iter =
                action.getOutgoingEdges().begin();
            std::advance(iter, index);
            EvoGraph::ActionEdge* actionEdge =
                dynamic_cast<EvoGraph::ActionEdge*>(*iter);

            mutateActionEdge(graph, action, actionEdge, newPrograms, params,
                                rng);

            proba *= params.tpg.pMutateActionProgram;

            anyMutationDone = true;
        }
    } while (!anyMutationDone && params.tpg.pMutateActionProgram != 0.0);

    graph.orderActionEdges(&action);
}

void Mutator::TPGMutator::crossProgram(
    EvoGraph::Graph& graph, std::vector<const EvoGraph::Action*> childs,
    size_t actionID, const Mutator::MutationParameters& params,
    RNG::RNG& rng)
{

    // Create new empty programs
    std::array<std::shared_ptr<Program::Program>, 2> newProgs = {
        std::make_shared<Program::Program>(graph.getEnvironment(), true),
        std::make_shared<Program::Program>(graph.getEnvironment(), true)};

    // Get the programs of the parents, it should alreay be checked that program
    // exist.
    std::array<std::shared_ptr<Program::Program>, 2> originProgs = {
        childs.at(0)->getEdgeOfAction(actionID)->getProgramSharedPointer(),
        childs.at(1)->getEdgeOfAction(actionID)->getProgramSharedPointer()};

    std::array<uint64_t, 2> cutStart, cutEnd, sizeProgs;

    // if the sum of the parents program size is above the max size, the size of
    // the cross lines is the same for both parents.
    bool specialCase =
        originProgs[0]->getNbLines() + originProgs[1]->getNbLines() >=
        params.prog.maxProgramSize;

    // Select random index for the crossover, normal case
    for (int i = 0; i < 2; i++) {

        uint64_t nbLines = originProgs[i]->getNbLines();
        if (specialCase) {
            nbLines = std::min(originProgs[0]->getNbLines(),
                               originProgs[1]->getNbLines());
        }

        if (nbLines < 2)
            return; // If a program has only one line, crossover cannot happen.

        cutStart[i] = rng.getUnsignedInt64(0, nbLines - 1);
        cutEnd[i] = rng.getUnsignedInt64(0, nbLines - 2);
        if (cutEnd[i] == cutStart[i]) {
            cutEnd[i]++;
        }
        else if (cutEnd[i] < cutStart[i]) {
            std::swap(cutStart[i], cutEnd[i]);
        }

        if (specialCase) {
            cutStart[1] = cutStart[0];
            cutEnd[1] = cutEnd[0];
            break;
        }
    }

    // Compute program size of the children
    for (int i = 0; i < 2; i++) {
        sizeProgs[i] = originProgs[i]->getNbLines() -
                       (cutEnd[i] - cutStart[i]) +
                       (cutEnd[1 - i] - cutStart[1 - i]);
    }

    // Create new programs with the cut
    for (int childIdx = 0; childIdx < 2; childIdx++) {
        auto& newProg = newProgs[childIdx];
        auto& parent1 = originProgs[childIdx];
        auto& parent2 = originProgs[1 - childIdx];
        uint64_t start1 = cutStart[childIdx], end1 = cutEnd[childIdx];
        uint64_t start2 = cutStart[1 - childIdx], end2 = cutEnd[1 - childIdx];

        for (size_t idx = 0; idx < sizeProgs[childIdx]; idx++) {
            if (idx < start1) {
                newProg->addNewLine(parent1->getLine(idx));
            }
            else if (idx >= start1 + (end2 - start2)) {
                newProg->addNewLine(
                    parent1->getLine(idx + (end1 - start1) - (end2 - start2)));
            }
            else {
                newProg->addNewLine(parent2->getLine(idx - start1 + start2));
            }
        }
    }

    // Add the new programs to the child.
    for (int i = 0; i < 2; i++) {
        graph.addNewActionEdge(*childs.at(i), newProgs[i], actionID);
        graph.removeActionEdge(*childs.at(i)->getEdgeOfAction(actionID));
        newProgs[i]->identifyIntrons();
    }
}

void Mutator::TPGMutator::crossEdges(EvoGraph::Graph& graph,
                                     std::vector<const EvoGraph::Action*> childs,
                                     size_t actionID,
                                     const Mutator::MutationParameters& params,
                                     RNG::RNG& rng)
{

    // get the edges
    EvoGraph::ActionEdge* edge1 = childs.at(0)->getEdgeOfAction(actionID);
    EvoGraph::ActionEdge* edge2 = childs.at(1)->getEdgeOfAction(actionID);

    // Only add the edge if the action is founded.
    if (edge1 != nullptr) {
        graph.addNewActionEdge(*childs.at(1), edge1->getProgramSharedPointer(),
                               actionID);
        graph.removeActionEdge(*edge1);
    }

    // Only add the edge if the action is founded.
    if (edge2 != nullptr) {
        graph.addNewActionEdge(*childs.at(0), edge2->getProgramSharedPointer(),
                               actionID);
        graph.removeActionEdge(*edge2);
    }
}

void Mutator::TPGMutator::crossAction(
    EvoGraph::Graph& graph, std::vector<const EvoGraph::Action*> childs,
    const Mutator::MutationParameters& params, RNG::RNG& rng)
{

    std::vector<uint64_t> indexUsed;
    uint64_t indexAction;

    // Always do at least one crossover, except is the proba is at zero
    // (mearning we don't want any crossover)
    double proba = (params.tpg.pCrossAgents != 0) ? 1 : 0;
    while (indexUsed.size() < graph.getEnvironment().getNbContinuousActions() &&
           proba > rng.getDouble(0.0, 1.0)) {

        // Select the action ID
        do {
            indexAction = rng.getUnsignedInt64(
                0, graph.getEnvironment().getNbContinuousActions() - 1);
        } while (std::find(indexUsed.begin(), indexUsed.end(), indexAction) !=
                 indexUsed.end());

        indexUsed.push_back(indexAction);

        // A crossover at program level can be done only the both parents
        // assessed the action concerned
        if (childs.at(0)->getAssessedActions().count(indexAction) > 0 &&
            childs.at(1)->getAssessedActions().count(indexAction) > 0 &&
            params.tpg.pCrossPrograms > rng.getDouble(0, 1)) {
            crossProgram(graph, childs, indexAction, params, rng);
        }
        else {
            crossEdges(graph, childs, indexAction, params, rng);
        }
        proba *= params.tpg.pCrossAgents;
    }

    for (auto child : childs) {
        graph.updateAssessedActions(child);
    }
}

void Mutator::TPGMutator::removeRandomEdge(EvoGraph::Graph& graph,
                                           const EvoGraph::TPGTeam& team,
                                           RNG::RNG& rng)
{
    // Pick an outgoing edge randomly,
    const std::list<EvoGraph::Edge*>& pickableEdges = team.getOutgoingEdges();

    // Note: No need to take special care of Actions. Since cycles can not
    // appear in TPG with the current mutation process, there is no need to
    // maintain an action within each team.

    // Pick a random edge
    auto iterSet = pickableEdges.begin();
    std::advance(iterSet, rng.getUnsignedInt64(0, pickableEdges.size() - 1));
    const EvoGraph::Edge* removedEdge = *iterSet;
    graph.removeEdge(*removedEdge);
}

void Mutator::TPGMutator::addRandomEdge(
    EvoGraph::Graph& graph, const EvoGraph::TPGTeam& team,
    const Selector::SelectionContext* context, RNG::RNG& rng)
{
    // Pick an edge (excluding ones from the team, edges with the team as a
    // destination and the edges that are action edges)
    auto pickableEdges(context->preExistingEdges);
    // cf erase-remove idiom
    pickableEdges.erase(
        std::remove_if(pickableEdges.begin(), pickableEdges.end(),
                       [&team](const EvoGraph::Edge* edge) -> bool {
                           return edge == nullptr ||
                                  dynamic_cast<const EvoGraph::ActionEdge*>(
                                      edge) != nullptr ||
                                  edge->getSource() == &team ||
                                  edge->getDestination() == &team;
                       }),
        pickableEdges.end());

    // Pick a pickable Edge
    // (This code assumes that the set of pickable edge is never empty..
    // otherwise it will throw an exception. Possible solution if needed
    // initialize an entirely new program and pick a random target.)
    std::list<const EvoGraph::Edge*>::iterator iter = pickableEdges.begin();
    std::advance(iter, rng.getUnsignedInt64(0, pickableEdges.size() - 1));
    const EvoGraph::Edge* pickedEdge = *iter;

    // Create new edge from team and with the same ProgramSharedPointer
    // But with the team as its source
    // throw std::runtime_error if the edge is not from the graph;
    const EvoGraph::Edge& newEdge = graph.cloneEdge(*pickedEdge);
    graph.setEdgeSource(newEdge, team);
}

void Mutator::TPGMutator::mutateEdgeDestination(
    EvoGraph::Graph& graph, const EvoGraph::Edge* edge,
    const Selector::SelectionContext* context,
    const Mutator::MutationParameters& params, RNG::RNG& rng)
{
    // Pick an edge among preexisting vertices
    const EvoGraph::Vertex* target = nullptr;

    // Should the new target be an action or a team
    bool targetAction =
        rng.getDouble(0, 1) < params.tpg.pEdgeDestinationIsAction;

    // Pick any target
    // Note: Having an action in all teams is no longer enforced,
    // as the presence of cycle in TPGs is not possible according to the current
    // mutation process.
    if (targetAction) {
        if (context->preExistingActions.size() > 0) {
            if (params.tpg.teamAccessAllActions) {
                target = context->preExistingActions.at(rng.getUnsignedInt64(
                    0, context->preExistingActions.size() - 1));
            }
            else {
                target = &graph.cloneVertex(
                    *context->preExistingActions.at(rng.getUnsignedInt64(
                        0, context->preExistingActions.size() - 1)));
            }
            // Change the target
            // Changing the target should not fail.
            graph.setEdgeDestination(*edge, *target);
        }
    }
    else if (context->preExistingTeams.size() > 0) {
        target = context->preExistingTeams.at(
            rng.getUnsignedInt64(0, context->preExistingTeams.size() - 1));
        // Change the target
        // Changing the target should not fail.
        graph.setEdgeDestination(*edge, *target);
    }
}

void Mutator::TPGMutator::mutateOutgoingEdge(
    EvoGraph::Graph& graph, const EvoGraph::Edge* edge,
    const Selector::SelectionContext* context,
    std::list<std::shared_ptr<Program::Program>>& newPrograms,
    const Mutator::MutationParameters& params, RNG::RNG& rng)
{

    // copy program
    std::shared_ptr<Program::Program> newProg(
        new Program::Program(edge->getProgram(), false));

    // Set the mutated program to the edge
    edge->setProgram(newProg);

    if (graph.getEnvironment().getNbContinuousActions() > 0 &&
        params.tpg.useActionProgram &&
        dynamic_cast<const EvoGraph::Action*>(edge->getDestination()) !=
            nullptr &&
        rng.getDouble(0.0, 1.0) > params.tpg.probaContextOverActionProgram) {

        // Clone the randomly selected action
        const EvoGraph::Action& newAction =
            (const EvoGraph::Action&)graph.cloneVertex(*edge->getDestination());

        mutateAction(graph, newAction, context, newPrograms, params, rng);

        // Set the action
        graph.setEdgeDestination(*edge, newAction);
    }
    else {

        // Add it to the list of new Program to be mutated.
        newPrograms.push_back(newProg);

        // Edge target modification
        // As it Stephen kelly's work, Edge target modification is conditionned
        // to the modification of the prealable Edge.Program behavior.
        if (rng.getDouble(0.0, 1.0) < params.tpg.pEdgeDestinationChange) {
            mutateEdgeDestination(graph, edge, context, params, rng);
        }
    }
}

void Mutator::TPGMutator::mutateTPGTeam(
    EvoGraph::Graph& graph, const Archive& archive, const EvoGraph::TPGTeam& team,
    const Selector::SelectionContext* context,
    std::list<std::shared_ptr<Program::Program>>& newPrograms,
    const Mutator::MutationParameters& params, RNG::RNG& rng)
{
    // 1. Remove randomly selected edges
    {
        // Keep at least two edges (otherwise the team is useless)
        double proba = 1.0;
        while (team.getOutgoingEdges().size() > 2 &&
               proba > rng.getDouble(0.0, 1.0)) {

            removeRandomEdge(graph, team, rng);

            // Decrement the proba of removing another edge
            proba *= params.tpg.pEdgeDeletion;
        }
    }

    // 2. Add random duplicated edge with the team as its source
    {
        double proba = 1.0;
        while (team.getOutgoingEdges().size() < params.tpg.maxOutgoingEdges &&
               proba > rng.getDouble(0.0, 1.0)) {
            // Add an edge (by duplication of an existing one)
            addRandomEdge(graph, team, context, rng);
            // Decrement the proba of adding another edge
            proba *= params.tpg.pEdgeAddition;
        }
    }

    // 3. Mutate edges of the team
    {
        bool anyMutationDone = false;
        do {
            // Process edge-by-edge
            // And possibly modify their target
            for (EvoGraph::Edge* edge : team.getOutgoingEdges()) {
                // Edge->Program bid modification
                if (rng.getDouble(0.0, 1.0) < params.tpg.pProgramMutation) {
                    // Mutate the edge
                    mutateOutgoingEdge(graph, edge, context, newPrograms,
                                       params, rng);
                    anyMutationDone = true;
                }
            }
        } while (!anyMutationDone);
    }
}

void Mutator::TPGMutator::mutateProgramBehaviorAgainstArchive(
    std::shared_ptr<Program::Program>& newProg,
    const Mutator::MutationParameters& params, const Archive& archive,
    RNG::RNG& rng)
{
    // If the Program behavior should be new after mutation:
    std::shared_ptr<Program::Program> newProgCopy(nullptr);
    if (params.tpg.forceProgramBehaviorChangeOnMutation) {
        // Copy the program to check that its behavior is changed before
        // verifying its unicity against the archive
        newProgCopy = std::make_shared<Program::Program>(
            *newProg, newProg->isActionProgram());
    }

    bool allUnique;
    // Mutate behavior until it changes (against the archive).
    do {

        // If a new program is created
        if (rng.getDouble(0.0, 1.0) < params.prog.pNewProgram) {
            Mutator::ProgramMutator::initRandomProgram(*newProg, params, rng);
        }
        else {
            // Mutate until something is mutated (i.e. the function returns
            // true) And until the program behavior is changed
            while (!(
                Mutator::ProgramMutator::mutateProgram(*newProg, params, rng) &&
                !(newProgCopy != nullptr &&
                  newProg->hasIdenticalBehavior(*newProgCopy))))
                ;
        }
        // Check for uniqueness in archive
        auto archivedDataHandlers = archive.getDataHandlers();
        std::map<size_t, double> hashesAndResults;
        Program::ProgramExecutionEngine pee(*newProg);
        for (std::pair<
                 size_t,
                 std::vector<std::reference_wrapper<const Data::DataHandler>>>
                 archiveDatahandler : archivedDataHandlers) {
            // Execute the mutated program on the archive data handlers
            pee.setDataSources(archiveDatahandler.second);
            double result = pee.executeProgram();
            hashesAndResults.insert({archiveDatahandler.first, result});
        }

        // If the result is not unique, do another mutation.
        allUnique = archive.areProgramResultsUnique(hashesAndResults);

        // Do not use Archive right now if the environment is continuous
        // TODO Update that
    } while (!allUnique &&
             (newProg->getEnvironment().getNbContinuousActions() == 0 ||
              params.tpg.useMultiActionProgram));
}

void Mutator::TPGMutator::mutateNewProgramBehaviors(
    const uint64_t& maxNbThreads,
    std::list<std::shared_ptr<Program::Program>>& newPrograms,
    RNG::RNG& rng, const Mutator::MutationParameters& params,
    const Archive& archive)
{
    // This is a computing intensive part of the mutation process
    // Hence the parallelization.
    if (maxNbThreads <= 1) {
        // Sequential (kept for determinism check mostly)
        for (std::shared_ptr<Program::Program> newProg : newPrograms) {
            RNG::RNG privateRNG(rng.getUnsignedInt64(0, UINT64_MAX));
            mutateProgramBehaviorAgainstArchive(newProg, params, archive,
                                                privateRNG);
        }
    }
    else {
        // Parallel
        // Create job list with Program pointers and seed
        std::queue<std::pair<std::shared_ptr<Program::Program>, uint64_t>>
            programsToMutate;
        for (std::shared_ptr<Program::Program> newProg : newPrograms) {
            programsToMutate.push(
                {newProg, rng.getUnsignedInt64(0, UINT64_MAX)});
        }

        std::mutex mutexMutation;

        // Function executed in threads
        auto parallelWorker = [&programsToMutate, &mutexMutation, &params,
                               &archive]() {
            RNG::RNG privateRNG;
            // While there is work to be done
            bool jobDone;
            do {
                std::pair<std::shared_ptr<Program::Program>, uint64_t> job;
                jobDone = false;
                { // get one job critical section
                    std::lock_guard lock(mutexMutation);
                    if (programsToMutate.size() != 0) {
                        jobDone = true;
                        job = programsToMutate.front();
                        programsToMutate.pop();
                    }
                }

                //  Do the job (if any)
                if (jobDone) {
                    privateRNG.setSeed(job.second);
                    mutateProgramBehaviorAgainstArchive(job.first, params,
                                                        archive, privateRNG);
                }
            } while (jobDone);
        };

        // Start threads
        std::vector<std::thread> threads;
        for (auto idx = 0; idx < maxNbThreads - 1; idx++) {
            threads.emplace_back(std::thread(parallelWorker));
        }

        // Work in the main thread also
        parallelWorker();

        // Join the threads
        for (auto& thread : threads) {
            thread.join();
        }
    }
}

void Mutator::TPGMutator::populateTPG(EvoGraph::Graph& graph,
                                      Selector::Selector& selector,
                                      const Archive& archive,
                                      const Mutator::MutationParameters& params,
                                      RNG::RNG& rng, uint64_t nbActions,
                                      uint64_t maxNbThreads)
{

    const Selector::SelectionContext* context = &selector.updateContext();

    // If the graph doesn't contain any clonable teams, call the init procedure.
    // (note that execution of this code is not a very good sign.. maybe an
    // exception would be more appropriate?)
    if (context->teamsClonable.size() == 0 && !params.tpg.useActionProgram &&
        graph.getNbVertices() < params.tpg.nbRoots) {
        initRandomTPG(graph, params, rng, nbActions);
        context = &selector.updateContext();
    }

    // Create an empty list to store Programs to mutate.
    std::list<std::shared_ptr<Program::Program>> newPrograms;

    // Create the new teams root
    uint64_t nbExpectedRoots =
        graph.getNbRootVertices() + context->nbTeamsToCreate;
    while (graph.getNbRootVertices() < nbExpectedRoots) {

        // Select a random existing root
        uint64_t clonedRootIndex =
            rng.getUnsignedInt64(0, context->teamsClonable.size() - 1);

        // clone it (the vertex and all its outgoing edges)
        const EvoGraph::TPGTeam& newRoot = (const EvoGraph::TPGTeam&)graph.cloneVertex(
            *context->teamsClonable.at(clonedRootIndex));

        // Apply mutations to the root
        mutateTPGTeam(graph, archive, newRoot, context, newPrograms, params,
                      rng);
    }

    // Divide root used into two subVector with half of the roots, randomly
    // selected.
    std::vector<const EvoGraph::Action*> subActionsClonable1(
        context->actionsClonable);
    std::vector<const EvoGraph::Action*> subActionsClonable2;
    for (size_t idx = 0; idx < context->actionsClonable.size() / 2; idx++) {
        auto root = subActionsClonable1.at(
            rng.getUnsignedInt64(0, subActionsClonable1.size() - 1));
        subActionsClonable2.push_back(root);
        std::swap(root, subActionsClonable1.back());
        subActionsClonable1.pop_back();
    }

    // Create the new action roots
    // With rounding number, nbActionsToCreate can be -1, which we dont want.
    uint64_t nbActionsCreated = 0;
    while (nbActionsCreated < context->nbActionsToCreate) {

        // Select two random existing roots
        uint64_t clonedRootIndex1 =
            rng.getUnsignedInt64(0, subActionsClonable1.size() - 1);
        std::vector<const EvoGraph::Action*> childs = {
            (const EvoGraph::Action*)(&graph.cloneVertex(
                *subActionsClonable1.at(clonedRootIndex1)))};

        // Be sure we have agents in both sub lists, and we still have at least
        // two agents to create
        if (subActionsClonable2.size() > 0 &&
            nbActionsCreated < context->nbActionsToCreate - 1) {

            uint64_t clonedRootIndex2 =
                rng.getUnsignedInt64(0, subActionsClonable2.size() - 1);

            // clone the child
            childs.push_back((const EvoGraph::Action*)(&graph.cloneVertex(
                *subActionsClonable2.at(clonedRootIndex2))));

            // Do the crossover over the childs
            crossAction(graph, childs, params, rng);
        }

        // Do the mutation over the childs
        for (auto child : childs) {
            if (child->getOutgoingEdges().size() == 0) {
                graph.removeVertex(*child);
            }
            else {
                // Apply mutations to the root and increase the number of roots
                mutateAction(graph, *child, context, newPrograms, params,
                                rng);
                nbActionsCreated++;
            }
        }
    }

    selector.updateAfterPopulate();

    // Mutate the new Programs
    mutateNewProgramBehaviors(maxNbThreads, newPrograms, rng, params, archive);
}
