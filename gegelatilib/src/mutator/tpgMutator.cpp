/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2024) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2024)
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
#include <mutex>
#include <numeric>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>
#include <array>

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

const TPG::TPGAction* Mutator::TPGMutator::initActionVertex(
    TPG::TPGGraph& graph, const Mutator::MutationParameters& params,
    Mutator::RNG& rng, uint64_t nbActionEdgeInit, uint64_t actionID)
{
    const TPG::TPGAction* action = &(graph.addNewAction(actionID));

    std::set<uint64_t> actionUsed;
    for(size_t j = 0; j < nbActionEdgeInit; j++){
        
        // Create a program and specify action program
        std::shared_ptr<Program::Program> p =
            std::make_shared<Program::Program>(graph.getEnvironment(),
                                                true);

        // RandomInit the Programs
        Mutator::ProgramMutator::initRandomProgram(*p, params, rng);

        // Find an action class not already used by this action 
        uint64_t actionClass;
        do{
            actionClass = rng.getUnsignedInt64(0, graph.getEnvironment().getNbContinuousActions() - 1);}
        while(actionUsed.find(actionClass) != actionUsed.end());
        actionUsed.insert(actionClass);

        // Create the action edge
        graph.addNewActionEdge(*action, p, actionClass);
    }

    if(params.tpg.useMultiActionProgram){
        graph.orderActionEdges(action);
    }

    return action;
}

void Mutator::TPGMutator::initRandomTPG(
    TPG::TPGGraph& graph, const Mutator::MutationParameters& params,
    Mutator::RNG& rng, uint64_t nbActions)
{

    // Number of action edge per action vertex.
    uint64_t nbActionEdgeInit = params.tpg.nbActionEdgeInit;
    uint64_t initNbTeams = (uint64_t)(params.tpg.ratioTeamsOverActions * params.tpg.nbRoots);
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

        if(params.tpg.useActionProgram || params.tpg.useMultiActionProgram){
            throw std::runtime_error(
                "Cannot use action program or multi action program with no "
                "action continuous available.");
        }

    }
    else if (graph.getEnvironment().getNbRegisters() <
             nbActions + (int)!params.tpg.useActionProgram && !params.tpg.useMultiActionProgram) {
        throw std::runtime_error(
            "The number of registers is below the number of values outputted.");
    }
    else {

        if(params.tpg.useActionProgram && !params.tpg.teamAccessAllActions && params.tpg.ratioTeamsOverActions == 1){
            throw std::runtime_error("Parameter only allow teams to select action roots, but there is not action roots considering the ratio!");
        }

        // No action edge.
        if(!params.tpg.useActionProgram){
            nbActionEdgeInit = 0;
        // Only one action edge.
        } else if(!params.tpg.useMultiActionProgram){
            nbActionEdgeInit = 1;
        }

        if (nbActionEdgeInit > graph.getEnvironment().getNbContinuousActions()){
            throw std::runtime_error("Maximum initial number of outgoing action edges "
                                "cannot exceed the number of action");
        }

        nbActionNotRoots = 2 * initNbTeams;
        initNbActions = params.tpg.nbRoots - initNbTeams + nbActionNotRoots;
    }


    if(params.tpg.maxInitOutgoingEdges < 2 && initNbTeams > 0){
        throw std::runtime_error(
            "A team should have at least two edges at initialisation.");
    }

    // Empty graph
    graph.clear();

    // Create teams, programs and Actions
    std::vector<const TPG::TPGAction*> actions;
    std::vector<const TPG::TPGTeam*> teams;
    std::vector<std::shared_ptr<Program::Program>> programs;


    for (size_t i = 0; i < initNbActions ; i++) {
        actions.push_back(initActionVertex(graph, params, rng, nbActionEdgeInit, i));
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
        graph.addNewEdge(*teams.at(i / 2),
                         *actions.at(i % nbActionNotRoots),
                         programs.at(i));
    }

    // Add additional connections to TPG
    // Team-by-Team
    for (const TPG::TPGTeam* team : teams) {
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
                            [&iter, &programs](const TPG::TPGEdge* edge) {
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
            graph.addNewEdge(*team,
                             *actions.at(rng.getUnsignedInt64(0, nbActionNotRoots - 1)),
                             programs.at(selectedProgramIndex));
        }
    }

    if(params.tpg.useMultiActionProgram){
        graph.updateAllAssessedActions();
    }

}



void Mutator::TPGMutator::removeRandomActionEdge(TPG::TPGGraph& graph,
                                        const TPG::TPGAction& action,
                                        Mutator::RNG& rng)
{
    // Pick an outgoing edge randomly,
    const std::list<TPG::TPGEdge*>& pickableEdges = action.getOutgoingEdges();

    // Note: No need to take special care of Actions. Since cycles can not
    // appear in TPG with the current mutation process, there is no need to
    // maintain an action within each team.

    // Pick a random edge
    auto iterSet = pickableEdges.begin();
    std::advance(iterSet, rng.getUnsignedInt64(0, pickableEdges.size() - 1));
    const TPG::TPGEdge* removedEdge = *iterSet;
    graph.removeActionEdge(*removedEdge);
}
 
void Mutator::TPGMutator::addRandomActionEdge(
    TPG::TPGGraph& graph, const TPG::TPGAction& action,
    const std::list<const TPG::TPGEdge*>& preExistingEdges, Mutator::RNG& rng)
{
    // Pick an edge (excluding ones from the team and edges with the team as a
    // destination)
    auto pickableEdges(preExistingEdges);
    // cf erase-remove idiom
    pickableEdges.erase(
        std::remove_if(
            pickableEdges.begin(), pickableEdges.end(),
            [&action](const TPG::TPGEdge* edge) -> bool {
                if(dynamic_cast<const TPG::TPGActionEdge*>(edge) != nullptr &&
                action.getAssessedActions().find(dynamic_cast<const TPG::TPGActionEdge*>(edge)->getActionClass()) ==
                action.getAssessedActions().end())
                {
                    return edge->getSource() == &action;
                } else {
                    return true;
                }
                
            }
        ),
        pickableEdges.end()
    );

    if(pickableEdges.size() == 0){
        // Chances are really low but the pickableEdges can be empty
        return;
    }

    // Pick a pickable Edge
    // (This code assumes that the set of pickable edge is never empty..
    // otherwise it will throw an exception. Possible solution if needed
    // initialize an entirely new program and pick a random target.)
    std::list<const TPG::TPGEdge*>::iterator iter = pickableEdges.begin();
    std::advance(iter, rng.getUnsignedInt64(0, pickableEdges.size() - 1));
    const TPG::TPGEdge* pickedEdge = *iter;



    // Create new edge from team and with the same ProgramSharedPointer
    // But with the team as its source
    // throw std::runtime_error if the edge is not from the graph;
    const TPG::TPGEdge& newEdge = graph.cloneEdge(*pickedEdge);
    graph.setEdgeSource(newEdge, action);
}




void Mutator::TPGMutator::swapActionEdges(            
    TPG::TPGGraph& graph, const TPG::TPGAction& action, Mutator::RNG& rng)
{

    // Randomly select two edges
    size_t index1 = rng.getUnsignedInt64(0, action.getOutgoingEdges().size() - 1);
    size_t index2 = rng.getUnsignedInt64(0, action.getOutgoingEdges().size() - 2);
    if(index1 == index2){
        index2++;
    }

    // Use a single iterator to traverse and identify both edges
    TPG::TPGEdge* edge1 = nullptr;
    TPG::TPGEdge* edge2 = nullptr;
    size_t currentIndex = 0;

    for (auto it = action.getOutgoingEdges().begin(); it != action.getOutgoingEdges().end(); ++it, ++currentIndex) {
        if (currentIndex == index1) {
            edge1 = *it;
        } else if (currentIndex == index2) {
            edge2 = *it;
        }
        if (edge1 && edge2) {
            break; // Stop as soon as both edges are found
        }
    }

    // Extract and swap action classes
    auto actionClass1 = dynamic_cast<TPG::TPGActionEdge*>(edge1)->getActionClass();
    auto actionClass2 = dynamic_cast<TPG::TPGActionEdge*>(edge2)->getActionClass();

    graph.setActionClassEdge(edge1, actionClass2);
    graph.setActionClassEdge(edge2, actionClass1);

}
 
 
 
 void Mutator::TPGMutator::mutateTPGActionEdge(
     TPG::TPGGraph& graph, const TPG::TPGAction& action, TPG::TPGActionEdge* actionEdge,
     std::list<std::shared_ptr<Program::Program>>& newPrograms,
     const Mutator::MutationParameters& params, Mutator::RNG& rng)
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
        action.getAssessedActions().size() < graph.getEnvironment().getNbContinuousActions() &&
        params.tpg.pChangeActionClass > rng.getDouble(0.0, 1.0)) {
 
        uint64_t newActionID = rng.getUnsignedInt64(0, graph.getEnvironment().getNbContinuousActions() - 1);
        while(action.getAssessedActions().find(newActionID) != action.getAssessedActions().end()){
             newActionID = rng.getUnsignedInt64(0, graph.getEnvironment().getNbContinuousActions() - 1);
        }
 
        actionEdge->setActionClass(newActionID);
         
        graph.updateAssessedActions(&action);   
    }
}
 
 
 void Mutator::TPGMutator::mutateTPGAction(
     TPG::TPGGraph& graph, const TPG::TPGAction& action,
     std::list<const TPG::TPGEdge*> preExistingEdges,
     std::list<std::shared_ptr<Program::Program>>& newPrograms,
     const Mutator::MutationParameters& params, Mutator::RNG& rng)
 { 
     if(params.tpg.useMultiActionProgram){
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
         while (action.getOutgoingEdges().size() < graph.getEnvironment().getNbContinuousActions() &&
             proba > rng.getDouble(0.0, 1.0)) {
             // Add an edge (by duplication of an existing one)
             addRandomActionEdge(graph, action, preExistingEdges, rng);
 
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
         while(indexUsed.size() < action.getOutgoingEdges().size()  && proba > rng.getDouble(0.0, 1.0)){

             do {
                 index = rng.getUnsignedInt64(0, action.getOutgoingEdges().size()-1);
             } while(std::find(indexUsed.begin(), indexUsed.end(), index) != indexUsed.end()) ;
 
             indexUsed.push_back(index);
     
             std::list<TPG::TPGEdge *>::const_iterator iter = action.getOutgoingEdges().begin();
             std::advance(iter, index);
             TPG::TPGActionEdge* actionEdge = dynamic_cast<TPG::TPGActionEdge*>(*iter);
 
             mutateTPGActionEdge(graph, action, actionEdge, newPrograms, params, rng);
 
             proba *= params.tpg.pMutateActionProgram;
 
             anyMutationDone = true;
         }
     } while (!anyMutationDone && params.tpg.pMutateActionProgram != 0.0);
 
 
     graph.orderActionEdges(&action);
 
 }

void Mutator::TPGMutator::removeRandomEdge(TPG::TPGGraph& graph,
                                           const TPG::TPGTeam& team,
                                           Mutator::RNG& rng)
{
    // Pick an outgoing edge randomly,
    const std::list<TPG::TPGEdge*>& pickableEdges = team.getOutgoingEdges();

    // Note: No need to take special care of Actions. Since cycles can not
    // appear in TPG with the current mutation process, there is no need to
    // maintain an action within each team.

    // Pick a random edge
    auto iterSet = pickableEdges.begin();
    std::advance(iterSet, rng.getUnsignedInt64(0, pickableEdges.size() - 1));
    const TPG::TPGEdge* removedEdge = *iterSet;
    graph.removeEdge(*removedEdge);
}

void Mutator::TPGMutator::addRandomEdge(
    TPG::TPGGraph& graph, const TPG::TPGTeam& team,
    const std::list<const TPG::TPGEdge*>& preExistingEdges, Mutator::RNG& rng)
{
    // Pick an edge (excluding ones from the team, edges with the team as a
    // destination and the edges that are action edges)
    auto pickableEdges(preExistingEdges);
    // cf erase-remove idiom
    pickableEdges.erase(
        std::remove_if(pickableEdges.begin(), pickableEdges.end(),
                       [&team](const TPG::TPGEdge* edge) -> bool {
                           return dynamic_cast<const TPG::TPGActionEdge*>(
                                      edge) != nullptr ||
                                  edge->getSource() == &team ||
                                  edge->getDestination() == &team;
                       }),
        pickableEdges.end());

    // Pick a pickable Edge
    // (This code assumes that the set of pickable edge is never empty..
    // otherwise it will throw an exception. Possible solution if needed
    // initialize an entirely new program and pick a random target.)
    std::list<const TPG::TPGEdge*>::iterator iter = pickableEdges.begin();
    std::advance(iter, rng.getUnsignedInt64(0, pickableEdges.size() - 1));
    const TPG::TPGEdge* pickedEdge = *iter;

    // Create new edge from team and with the same ProgramSharedPointer
    // But with the team as its source
    // throw std::runtime_error if the edge is not from the graph;
    const TPG::TPGEdge& newEdge = graph.cloneEdge(*pickedEdge);
    graph.setEdgeSource(newEdge, team);
}

void Mutator::TPGMutator::mutateEdgeDestination(
    TPG::TPGGraph& graph, const TPG::TPGEdge* edge,
    const std::vector<const TPG::TPGTeam*>& preExistingTeams,
    const std::vector<const TPG::TPGAction*>& preExistingActions,
    const Mutator::MutationParameters& params, Mutator::RNG& rng)
{
    // Pick an edge among preexisting vertices
    const TPG::TPGVertex* target = NULL;

    // Should the new target be an action or a team
    bool targetAction =
        rng.getDouble(0, 1) < params.tpg.pEdgeDestinationIsAction;

    // Pick any target
    // Note: Having an action in all teams is no longer enforced,
    // as the presence of cycle in TPGs is not possible according to the current
    // mutation process.
    if (targetAction) {
        target =  preExistingActions.at(
            rng.getUnsignedInt64(0, preExistingActions.size() - 1));
    }
    else {
        target = preExistingTeams.at(
            rng.getUnsignedInt64(0, preExistingTeams.size() - 1));
    }

    // Change the target
    // Changing the target should not fail.
    graph.setEdgeDestination(*edge, *target);
}

void Mutator::TPGMutator::mutateOutgoingEdge(
    TPG::TPGGraph& graph, const TPG::TPGEdge* edge,
    const std::vector<const TPG::TPGTeam*>& preExistingTeams,
    const std::vector<const TPG::TPGAction*>& preExistingActions,
    const std::list<const TPG::TPGEdge*>& preExistingEdges,
    std::list<std::shared_ptr<Program::Program>>& newPrograms,
    const Mutator::MutationParameters& params, Mutator::RNG& rng)
{

    // copy program
    std::shared_ptr<Program::Program> newProg(
        new Program::Program(edge->getProgram(), false));

    // Set the mutated program to the edge
    edge->setProgram(newProg);

    if (graph.getEnvironment().getNbContinuousActions() > 0 &&
        params.tpg.useActionProgram &&
        dynamic_cast<const TPG::TPGAction*>(edge->getDestination()) !=
            nullptr &&
        rng.getDouble(0.0, 1.0) > params.tpg.probaContextOverActionProgram) {

        // Clone the randomly selected action
        const TPG::TPGAction& newAction = (const TPG::TPGAction&)graph.cloneVertex(*edge->getDestination());

        mutateTPGAction(graph, newAction, preExistingEdges, newPrograms, 
                               params, rng);

        // Set the action
        graph.setEdgeDestination(*edge, newAction);
    } else {


        // Add it to the list of new Program to be mutated.
        newPrograms.push_back(newProg);


        // Edge target modification
        // As it Stephen kelly's work, Edge target modification is conditionned
        // to the modification of the prealable Edge.Program behavior.
        if (rng.getDouble(0.0, 1.0) < params.tpg.pEdgeDestinationChange) {
            mutateEdgeDestination(graph, edge, preExistingTeams, preExistingActions,
                                params, rng);
        }
    }

}

void Mutator::TPGMutator::mutateTPGTeam(
    TPG::TPGGraph& graph, const Archive& archive, const TPG::TPGTeam& team,
    const std::vector<const TPG::TPGTeam*>& preExistingTeams,
    const std::vector<const TPG::TPGAction*>& preExistingActions,
    const std::list<const TPG::TPGEdge*>& preExistingEdges,
    std::list<std::shared_ptr<Program::Program>>& newPrograms,
    const Mutator::MutationParameters& params, Mutator::RNG& rng)
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
            addRandomEdge(graph, team, preExistingEdges, rng);
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
            for (TPG::TPGEdge* edge : team.getOutgoingEdges()) {
                // Edge->Program bid modification
                if (rng.getDouble(0.0, 1.0) < params.tpg.pProgramMutation) {
                    // Mutate the edge
                    mutateOutgoingEdge(graph, edge, preExistingTeams,
                                       preExistingActions, preExistingEdges, newPrograms, params,
                                       rng);
                    anyMutationDone = true;
                }
            }
        } while (!anyMutationDone);
    }
}

void Mutator::TPGMutator::mutateProgramBehaviorAgainstArchive(
    std::shared_ptr<Program::Program>& newProg,
    const Mutator::MutationParameters& params, const Archive& archive,
    Mutator::RNG& rng)
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
    Mutator::RNG& rng, const Mutator::MutationParameters& params,
    const Archive& archive)
{
    // This is a computing intensive part of the mutation process
    // Hence the parallelization.
    if (maxNbThreads <= 1) {
        // Sequential (kept for determinism check mostly)
        for (std::shared_ptr<Program::Program> newProg : newPrograms) {
            Mutator::RNG privateRNG(rng.getUnsignedInt64(0, UINT64_MAX));
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
            Mutator::RNG privateRNG;
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

void Mutator::TPGMutator::updateClonableAndExistingVertexForTournament(
    std::vector<const TPG::TPGTeam *>& teamsClonable,
    std::vector<const TPG::TPGAction*>& actionsClonable,
    std::vector<const TPG::TPGTeam*>& preExistingTeams,
    std::vector<const TPG::TPGAction*>& preExistingActions
)
{
    // The root not set to be deleted are not used during evolution
    teamsClonable.erase(
        std::remove_if(teamsClonable.begin(), teamsClonable.end(),
                        [](const TPG::TPGVertex* vertex) -> bool {
                            return !vertex->isToBeDeleted();}),
                            teamsClonable.end());

    actionsClonable.erase(
        std::remove_if(actionsClonable.begin(), actionsClonable.end(),
                        [](const TPG::TPGVertex* vertex) -> bool {
                            return !vertex->isToBeDeleted();}),
                            actionsClonable.end());


    // Erase the vertex set to be deleted to the list of pre existing vertex.
    // They are only used for being a new destination
    preExistingTeams.erase(
        std::remove_if(preExistingTeams.begin(), preExistingTeams.end(),
                        [](const TPG::TPGVertex* vertex) -> bool {
                            return vertex->isToBeDeleted();}),
                            preExistingTeams.end());

    preExistingActions.erase(
        std::remove_if(preExistingActions.begin(), preExistingActions.end(),
                        [](const TPG::TPGVertex* vertex) -> bool {
                            return vertex->isToBeDeleted();}),
                            preExistingActions.end());
}


void Mutator::TPGMutator::populateTPG(TPG::TPGGraph& graph,
                                      const Archive& archive,
                                      const Mutator::MutationParameters& params,
                                      Mutator::RNG& rng, uint64_t nbActions,
                                      uint64_t maxNbThreads)
{
    // Get current vertex set (copy)
    auto vertices(graph.getVertices());
    // Get current root teams (copy)
    auto rootVertices(graph.getRootVertices());
    // Get root Teams
    std::vector<const TPG::TPGTeam*> rootTeams;
    std::vector<const TPG::TPGAction*> rootActions;
    std::for_each(rootVertices.begin(), rootVertices.end(),
                  [&rootTeams, &rootActions, &params](const TPG::TPGVertex* vertex) {
                      if (dynamic_cast<const TPG::TPGTeam*>(vertex) !=
                          nullptr) {
                          rootTeams.push_back((const TPG::TPGTeam*)vertex);
                      } else if(params.tpg.useActionProgram){
                          rootActions.push_back((const TPG::TPGAction*)vertex);
                      }
                  });

    // If the graph doesn't contain any root teams, call the init procedure.
    // (note that execution of this code is not a very good sign.. maybe an
    // exception would be more appropriate?)
    if (rootTeams.size() == 0 && !params.tpg.useActionProgram) {
        initRandomTPG(graph, params, rng, nbActions);
        vertices = graph.getVertices();
        rootVertices = graph.getRootVertices();
        rootTeams.clear();
        std::for_each(rootVertices.begin(), rootVertices.end(),
                      [&rootTeams](const TPG::TPGVertex* vertex) {
                          rootTeams.push_back((const TPG::TPGTeam*)vertex);
                      });
    }

    // Pre compute liste of available TPGTeam and TPGActions, TPGActions are only roots
    std::vector<const TPG::TPGTeam*> preExistingTeams;
    std::vector<const TPG::TPGAction*> preExistingActions;

    std::for_each(
        vertices.begin(), vertices.end(),
        [&preExistingActions, &preExistingTeams, &params](const TPG::TPGVertex* target) {
            if (dynamic_cast<const TPG::TPGAction*>(target) != nullptr
                && (target->getIncomingEdges().size() == 0 || params.tpg.teamAccessAllActions)) {
                preExistingActions.push_back((const TPG::TPGAction*)target);
            }
            else {
                preExistingTeams.push_back((const TPG::TPGTeam*)target);
            }
        });

    // Get a list of pre existing edges before mutations (copy)
    std::list<const TPG::TPGEdge*> preExistingEdges;
    std::for_each(
        graph.getEdges().begin(), graph.getEdges().end(),
        [&preExistingEdges](const std::unique_ptr<TPG::TPGEdge>& edge) {
            preExistingEdges.push_back(edge.get());
        });


    // Create an empty list to store Programs to mutate.
    std::list<std::shared_ptr<Program::Program>> newPrograms;

    // Create list of teams and actions clonable
    std::vector<const TPG::TPGTeam *> teamsClonable = rootTeams;
    std::vector<const TPG::TPGAction *> actionsClonable = rootActions;

    bool useTournamentSelection = graph.getEnvironment().getParams().useTournamentSelection;
    if (useTournamentSelection) {
        // Update the list of clonable and preExisting Vertices because of the tournmanent selection
        updateClonableAndExistingVertexForTournament(teamsClonable, actionsClonable, preExistingTeams, preExistingActions);
    }

    // Create the new teams root
    uint64_t nbTeamsToCreate = (uint64_t)(params.tpg.nbRoots * params.tpg.ratioTeamsOverActions) - rootTeams.size() + (teamsClonable.size() * useTournamentSelection);
    uint64_t nbExpectedRoots = graph.getNbRootVertices() + nbTeamsToCreate;
    while (graph.getNbRootVertices() < nbExpectedRoots) {

        // Select a random existing root
        uint64_t clonedRootIndex =
            rng.getUnsignedInt64(0, teamsClonable.size() - 1);

        // clone it (the vertex and all its outgoing edges)
        const TPG::TPGTeam& newRoot =
            (const TPG::TPGTeam&)graph.cloneVertex(
                *teamsClonable.at(clonedRootIndex));

        // Apply mutations to the root
        mutateTPGTeam(graph, archive, newRoot,
                        preExistingTeams, preExistingActions,
                        preExistingEdges, newPrograms, params, rng);
    }

    // Create the new action roots
    uint64_t nbActionsCreated = 0;
    uint64_t nbActionsToCreate = (uint64_t)(params.tpg.nbRoots * (1 - params.tpg.ratioTeamsOverActions)) - rootActions.size() + (actionsClonable.size() * useTournamentSelection);
    while (nbActionsCreated < nbActionsToCreate) {

        // Select a random existing root
        uint64_t clonedRootIndex =
            rng.getUnsignedInt64(0, rootActions.size() - 1);

        // clone it (the vertex and all its outgoing edges)
        const TPG::TPGAction& newRoot =
            (const TPG::TPGAction&)graph.cloneVertex(
                *rootActions.at(clonedRootIndex));

        // Apply mutations to the root
        mutateTPGAction(graph, newRoot, preExistingEdges,newPrograms,
                        params, rng);

        
        // Increase the new number of roots
        nbActionsCreated++;
    }

    // Remove root to be deleted
    for(auto root: rootVertices){
        if(root->isToBeDeleted()){
            graph.removeVertex(*root);
        }
    }

    // Mutate the new Programs
    mutateNewProgramBehaviors(maxNbThreads, newPrograms, rng, params, archive);
}
