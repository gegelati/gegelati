

#include "algorithm/maple/mapleMutator.h"


void Algorithm::Maple::MapleMutator::updateSpecificContext(
    std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager,
    std::shared_ptr<Selector::Selector> selector,
    const Learn::LearningParameters& params,
    RNG::RNG& rng)
{
    // Call parent method to update currentContext
    Algorithm::Mutator::updateSpecificContext(graph, manager, selector, params, rng);


    // Update pre-existing elements
    this->preExistingTeams.clear();
    this->preExistingActions.clear();
    this->preExistingEdges.clear();

    std::set<std::shared_ptr<const EvoGraph::Edge>, SharedLess<EvoGraph::Edge>> usableEdges;
    std::set<std::shared_ptr<const EvoGraph::Vertex>, SharedLess<EvoGraph::Vertex>> usableVertices;

    // Initialize queue with vertices from all pre-existing agents
    for (auto agentPtr : this->currentContext->preExistingAgents) {
        if(auto tpgAgent = std::dynamic_pointer_cast<const MapleAgent>(agentPtr)) {
            // Vertex of the agent should be an action
            if(!std::dynamic_pointer_cast<const EvoGraph::Action>(tpgAgent->getVertex())){
                throw std::runtime_error("TPGMutator::updateSpecificContext: the vertex of a MapleAgent should be an Action.");
            }
            usableVertices.insert(tpgAgent->getVertex());
        } else {
            throw std::runtime_error("TPGMutator::updateSpecificContext: an agent in the current context is not a TPGAgent.");
        }
    }

    for(auto vertex: usableVertices){
        this->preExistingActions.push_back(std::dynamic_pointer_cast<const EvoGraph::Action>(vertex));
        usableEdges.insert(vertex->getOutgoingEdges().begin(), vertex->getOutgoingEdges().end());
    }

    this->preExistingEdges.insert(this->preExistingEdges.end(), usableEdges.begin(), usableEdges.end());
}

void Algorithm::Maple::MapleMutator::initRandomPopulation(std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Learn::LearningParameters& params, RNG::RNG& rng)
{

    auto outputs = manager->getOutputs();
    if(outputs.sizeContinuous() != 0 && outputs.sizeDiscrete() != 0){
        throw std::runtime_error("MapleMutator::initRandomPopulation: Maple does not support mixed discrete and continuous outputs.");
    } else if (outputs.sizeDiscrete() != 0 || outputs.sizeContinuous() != 0){
        if(params.mutation.tpg.nbActionEdgeInit > outputs.size()){
            throw std::runtime_error("MapleMutator::initRandomPopulation: Number of discrete outputs cannot be lower than the number of initial edges.");
        }
    } else if (outputs.size() == 0){
        throw std::runtime_error("TPGMutator::initRandomPopulation: No outputs defined.");
    }

    // Empty agent manager
    manager->clearAgents();

    for (size_t idx = 0; idx < params.mutation.tpg.nbRoots; idx++) {
        this->initRandomAgent(graph, manager, params, rng);
    }
}

void Algorithm::Maple::MapleMutator::initRandomSpecificAgent(std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Learn::LearningParameters& params, RNG::RNG& rng)
{

    auto vertex = std::dynamic_pointer_cast<const MapleAgent>(agent)->getVertex();
    auto action = std::dynamic_pointer_cast<const EvoGraph::Action>(vertex);

    // Get program mutator and manager
    auto programMutator = this->getSubMutator(this->programAlgorithmName);
    auto programManager = manager->getSubManager(this->programAlgorithmName);

    // Get available actions classes
    std::vector<uint64_t> availableActions(manager->getOutputs().size());
    std::iota(availableActions.begin(), availableActions.end(), uint64_t{0});


    size_t remaining = availableActions.size();
    for (size_t idxAction = 0; idxAction < params.mutation.tpg.nbActionEdgeInit; idxAction++) {
        auto programAgent = programMutator->initRandomAgent(graph, programManager, params, rng);

        // Pick uniformly from remaining values
        size_t pickIdx = rng.getUnsignedInt64(0, remaining - 1);
        uint64_t actionClass = availableActions[pickIdx];

        // Remove picked element (swap with last)
        // By swapping, the order of availableActions is changed.
        std::swap(availableActions[pickIdx], availableActions[remaining - 1]);
        --remaining;

        // Create the action edge
        graph->addNewActionEdge(*action, programAgent, actionClass);
    }

    graph->orderActionEdges(action);
}

void Algorithm::Maple::MapleMutator::crossoverAgents(
    std::vector<std::shared_ptr<const Agent>> agents, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, std::vector<std::shared_ptr<const Agent>>& newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng)
{

}

void Algorithm::Maple::MapleMutator::addRandomEdge(
    std::shared_ptr<EvoGraph::Graph> graph, const EvoGraph::Action& action,
    RNG::RNG& rng)
{
    // Pick an edge (excluding ones from the team, edges with the team as a
    // destination and the edges that are action edges)
    auto pickableEdges(this->preExistingEdges);
    // cf erase-remove idiom
    pickableEdges.erase(
        std::remove_if(pickableEdges.begin(), pickableEdges.end(),
                       [&action](std::shared_ptr<const EvoGraph::Edge> edge) -> bool {
                            // Rewritte this condition but in a friendly way for the reader
                            if (edge == nullptr || edge->getSource().get() == &action) {
                                return true;
                            } else if (auto actionEdge = std::dynamic_pointer_cast<const EvoGraph::ActionEdge>(edge)) {
                                return action.getAssessedActions().find(actionEdge->getActionClass()) != action.getAssessedActions().end();
                            }
                            return false;
                       }),
        pickableEdges.end());

    if(pickableEdges.size() == 0){
        return;
    }

    // Pick a pickable Edge
    // (This code assumes that the set of pickable edge is never empty..
    // otherwise it will throw an exception. Possible solution if needed
    // initialize an entirely new program and pick a random target.)
    auto iter = pickableEdges.begin();
    std::advance(iter, rng.getUnsignedInt64(0, pickableEdges.size() - 1));
    std::shared_ptr<const EvoGraph::Edge> pickedEdge = *iter;

    // Create new edge from action and with the same ProgramSharedPointer
    // But with the action as its source
    // throw std::runtime_error if the edge is not from the graph;
    graph->setEdgeSource(*graph->cloneEdge(*pickedEdge), action);
}

void Algorithm::Maple::MapleMutator::swapActionEdges(std::shared_ptr<EvoGraph::Graph> graph,
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

    // Extract and swap action classes
    auto actionClass1 = std::dynamic_pointer_cast<const EvoGraph::ActionEdge>(*it1)->getActionClass();
    auto actionClass2 = std::dynamic_pointer_cast<const EvoGraph::ActionEdge>(*it2)->getActionClass();

    // Set the swapped action classes
    graph->setActionClassEdge(*it1, actionClass2);
    graph->setActionClassEdge(*it2, actionClass1);
}

void Algorithm::Maple::MapleMutator::mutateOutgoingEdge(
    std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<const EvoGraph::Edge> edge,
    const std::set<size_t>& actionClasses, std::shared_ptr<AgentManager> manager,
    std::vector<std::shared_ptr<const Agent>>& newSubAgents,
    const Learn::LearningParameters& params, RNG::RNG& rng)
{
    auto originAgent = edge->getProgram();
    // copy program
    std::shared_ptr<const Algorithm::Agent> newAgent = manager->getSubManager(originAgent->getAlgorithmName())->copyAgent(originAgent, graph);

    // Set the mutated agent to the edge
    edge->setProgram(newAgent);

    // Add it to the list of new agent to be mutated.
    newSubAgents.push_back(newAgent);

    size_t nbActions = manager->getOutputs().size();
    // Change action ID randomly if the action do not contain all actions.
    if (actionClasses.size() < nbActions &&
        params.mutation.tpg.pChangeActionClass > rng.getDouble(0.0, 1.0)) {

        std::vector<size_t> possibleNewActionClasses;
        for (size_t actionID = 0; actionID < nbActions; actionID++) {
            if (actionClasses.find(actionID) == actionClasses.end()) {
                possibleNewActionClasses.push_back(actionID);
            }
        }

        size_t newActionID = possibleNewActionClasses[rng.getUnsignedInt64(0, possibleNewActionClasses.size() - 1)];
        graph->setActionClassEdge(edge, newActionID);
    }
}

void Algorithm::Maple::MapleMutator::mutateAgent(
    std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, std::vector<std::shared_ptr<const Agent>>& newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng)
{
    if(std::dynamic_pointer_cast<const MapleAgent>(agent) == nullptr){
        throw std::runtime_error("MapleMutator::mutateAgent: the agent to mutate is not a MapleAgent.");
    }
    std::shared_ptr<const EvoGraph::Vertex> vertex = std::dynamic_pointer_cast<const MapleAgent>(agent)->getVertex();
    std::shared_ptr<const EvoGraph::Action> action = std::dynamic_pointer_cast<const EvoGraph::Action>(vertex);

    // 1. Remove randomly selected edges
    // Keep at least two edges (otherwise the team is useless)
    double proba = params.mutation.tpg.pActionEdgeDeletion;
    while (action->getOutgoingEdges().size() > 1 &&
            proba > rng.getDouble(0.0, 1.0)) {
        this->removeRandomEdge(graph, *action, rng);

        // Decrement the proba of removing another edge
        proba *= params.mutation.tpg.pActionEdgeDeletion;

        // Update assessed actions
        graph->updateAssessedActions(action);
    }

    // 2. Add random duplicated edge with the team as its source
    proba = params.mutation.tpg.pActionEdgeAddition;
    while (action->getOutgoingEdges().size() <
                manager->getOutputs().size() &&
            proba > rng.getDouble(0.0, 1.0)) {
        // Add an edge (by duplication of an existing one)
        this->addRandomEdge(graph, *action, rng);

        // Decrement the proba of adding another edge
        proba *= params.mutation.tpg.pActionEdgeAddition;

        // Update assessed actions
        graph->updateAssessedActions(action);

    }

    // 3. swap randomly selected edges
    // With at least two edges
    proba = params.mutation.tpg.pSwapActionProgram;
    while (action->getOutgoingEdges().size() > 2 &&
            proba > rng.getDouble(0.0, 1.0)) {
        this->swapActionEdges(graph, *action, rng);

        // Decrement the proba of swapping two edges
        proba *= params.mutation.tpg.pSwapActionProgram;
    }

    bool anyMutationDone = false;
    do {
        std::vector<uint64_t> indexUsed;
        uint64_t index;
        // 4. mutate randomly selected program on action Edge.
        double proba = params.mutation.tpg.pMutateActionProgram;
        while (indexUsed.size() < action->getOutgoingEdges().size() &&
               proba > rng.getDouble(0.0, 1.0)) {

            // Pick a random edge not already used
            do {
                index = rng.getUnsignedInt64(
                    0, action->getOutgoingEdges().size() - 1);
            } while (std::find(indexUsed.begin(), indexUsed.end(), index) !=
                     indexUsed.end());
            indexUsed.push_back(index);

            auto iter = action->getOutgoingEdges().begin();
            std::advance(iter, index);

            this->mutateOutgoingEdge(graph, *iter, action->getAssessedActions(), manager, newSubAgents, params, rng);
            graph->updateAssessedActions(action);

            proba *= params.mutation.tpg.pMutateActionProgram;
            anyMutationDone = true;
        }
    } while (!anyMutationDone && params.mutation.tpg.pMutateActionProgram != 0.0);
    graph->orderActionEdges(action);
}

