

#include "algorithm/maple/mapleMutator.h"

bool Algorithm::Maple::MapleMutator::isConfigurationValid(const Learn::LearningParameters& params, const Output::OutputHandler& outputs) const
{
    if(outputs.sizeContinuous() != 0 && outputs.sizeDiscrete() != 0){
        throw std::runtime_error("MapleMutator::initRandomPopulation: Maple does not support mixed discrete and continuous outputs.");
    } else if (outputs.sizeDiscrete() != 0 || outputs.sizeContinuous() != 0){
        if(params.mutation.tpg.nbActionEdgeInit > outputs.size()){
            throw std::runtime_error("MapleMutator::initRandomPopulation: Number of discrete outputs cannot be lower than the number of initial edges.");
        }        
    } else if (outputs.size() == 0){
        throw std::runtime_error("TPGMutator::initRandomPopulation: No outputs defined.");
    }
    return true;
}

void Algorithm::Maple::MapleMutator::initRandomPopulation(std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Learn::LearningParameters& params, RNG::RNG& rng)
{
    this->isConfigurationValid(params, manager->getOutputs());
    this->initActionVertices(graph, manager->getOutputs().size());

    // Empty agent manager
    manager->clearAgents(graph);

    for (size_t idx = 0; idx < params.mutation.tpg.nbRoots; idx++) {
        this->initRandomAgent(graph, manager, params, rng);
    }
}

void Algorithm::Maple::MapleMutator::initRandomSpecificAgent(const Agent& agent, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Learn::LearningParameters& params, RNG::RNG& rng)
{
    // First agent initialized, check configuration validity and create action vertices
    if(manager->getAgents().size() == 1){
        this->isConfigurationValid(params, manager->getOutputs());
        this->initActionVertices(graph, manager->getOutputs().size());
    }

    manager->emptyAgent(agent, graph);
    
    auto vertex = dynamic_cast<const MapleAgent&>(agent).getVertex();
    auto team = std::dynamic_pointer_cast<const EvoGraph::Team>(vertex);

    // Get program mutator and manager
    auto programMutator = this->getSubMutator(this->programAlgorithmName);
    auto programManager = manager->getSubManager(this->programAlgorithmName);

    // Get available actions classes
    std::vector<uint64_t> availableActions(manager->getOutputs().size());
    std::iota(availableActions.begin(), availableActions.end(), uint64_t{0});


    // Get the actions vertices.
    auto actionVertices = graph->getActions();

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
        graph->addNewEdge(*team, *actionVertices.at(actionClass), programAgent);
    }

    graph->orderActionEdges(team);
}

void Algorithm::Maple::MapleMutator::crossoverAgents(
    std::vector<std::reference_wrapper<const Agent>> agents, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, std::vector<std::weak_ptr<const Agent>>& newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng)
{

}

void Algorithm::Maple::MapleMutator::addRandomEdge(
    std::shared_ptr<EvoGraph::Graph> graph, const EvoGraph::Team& team,
    RNG::RNG& rng)
{
    // Pick an edge (excluding ones from the team, edges with the team as a
    // destination and the edges that are action edges)
    auto pickableEdges(this->preExistingEdges);
    // cf erase-remove idiom
    pickableEdges.erase(
        std::remove_if(pickableEdges.begin(), pickableEdges.end(),
                       [&team](std::shared_ptr<const EvoGraph::Edge> edge) -> bool {
                            // Rewritte this condition but in a friendly way for the reader
                            if (edge == nullptr || edge->getSource().get() == &team) {
                                return true;
                            } else if (auto action = std::dynamic_pointer_cast<const EvoGraph::Action>(edge->getDestination())) {
                                return team.getAssessedActions().find(action->getActionID()) != team.getAssessedActions().end();
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
    graph->setEdgeSource(*graph->cloneEdge(*pickedEdge), team);
}

void Algorithm::Maple::MapleMutator::swapEdges(std::shared_ptr<EvoGraph::Graph> graph,
                                          const EvoGraph::Team& team,
                                          RNG::RNG& rng)
{

    // Randomly select two edges
    size_t index1 =
        rng.getUnsignedInt64(0, team.getOutgoingEdges().size() - 1);
    size_t index2 =
        rng.getUnsignedInt64(0, team.getOutgoingEdges().size() - 2);
    if (index2 == index1) {
        index2++;
    }

    // Get iterators to the selected edges
    auto it1 = team.getOutgoingEdges().begin();
    std::advance(it1, index1);
    auto it2 = team.getOutgoingEdges().begin();
    std::advance(it2, index2);

    // Extract and swap action vertices
    auto actionVertex1 = std::dynamic_pointer_cast<const EvoGraph::Action>((*it1)->getDestination());
    auto actionVertex2 = std::dynamic_pointer_cast<const EvoGraph::Action>((*it2)->getDestination());

    // Set the swapped action classes
    graph->setEdgeDestination(*(*it1), *actionVertex2);
    graph->setEdgeDestination(*(*it2), *actionVertex1);
}


void Algorithm::Maple::MapleMutator::mutateEdgeDestination(
    std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<const EvoGraph::Edge> edge,
    const std::set<size_t>& actionClasses,
    const Learn::LearningParameters& params, RNG::RNG& rng)
{
    std::vector<std::shared_ptr<const EvoGraph::Action>> actionVertices(graph->getActions());
    actionVertices.erase(
        std::remove_if(
            actionVertices.begin(), actionVertices.end(),
            [&actionClasses](const auto& actionVertex) {
                return actionClasses.find(actionVertex->getActionID()) != actionClasses.end();
            }
        ),
        actionVertices.end()
    );

    auto newAction = actionVertices[rng.getUnsignedInt64(0, actionVertices.size() - 1)];
    graph->setEdgeDestination(*edge, *newAction);
}

void Algorithm::Maple::MapleMutator::mutateOutgoingEdge(
    std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<const EvoGraph::Edge> edge,
    const std::set<size_t>& actionClasses, std::shared_ptr<AgentManager> manager,
    std::vector<std::weak_ptr<const Agent>>& newSubAgents,
    const Learn::LearningParameters& params, RNG::RNG& rng)
{
    const Agent& originAgent = *edge->getProgram().lock();
    // copy program
    std::weak_ptr<const Algorithm::Agent> newAgent = manager->getSubManager(originAgent.getAlgorithmName())->copyAgent(originAgent, graph);

    // Set the mutated agent to the edge
    graph->setEdgeProgram(*edge, newAgent);

    // Add it to the list of new agent to be mutated.
    newSubAgents.push_back(newAgent);

    size_t nbActions = manager->getOutputs().size();
    // Change action ID randomly if the action do not contain all actions.
    if (actionClasses.size() < nbActions &&
        params.mutation.tpg.pChangeActionClass > rng.getDouble(0.0, 1.0)) {
        
        this->mutateEdgeDestination(graph, edge, actionClasses, params, rng);
    }
}

void Algorithm::Maple::MapleMutator::mutateAgent(
    const Agent& agent, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, std::vector<std::weak_ptr<const Agent>>& newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng)
{
    std::shared_ptr<const EvoGraph::Vertex> vertex = dynamic_cast<const MapleAgent&>(agent).getVertex();
    std::shared_ptr<const EvoGraph::Team> team = std::dynamic_pointer_cast<const EvoGraph::Team>(vertex);

    // 1. Remove randomly selected edges
    // Keep at least two edges (otherwise the team is useless)
    double proba = params.mutation.tpg.pActionEdgeDeletion;
    while (team->getOutgoingEdges().size() > 1 &&
            proba > rng.getDouble(0.0, 1.0)) {
        this->removeRandomEdge(graph, *team, rng);

        // Decrement the proba of removing another edge
        proba *= params.mutation.tpg.pActionEdgeDeletion;

        // Update assessed actions
        graph->updateAssessedActions(team);
    }

    // 2. Add random duplicated edge with the team as its source
    proba = params.mutation.tpg.pActionEdgeAddition;
    while (team->getOutgoingEdges().size() <
                manager->getOutputs().size() &&
            proba > rng.getDouble(0.0, 1.0)) {
        // Add an edge (by duplication of an existing one)
        this->addRandomEdge(graph, *team, rng);

        // Decrement the proba of adding another edge
        proba *= params.mutation.tpg.pActionEdgeAddition;

        // Update assessed actions
        graph->updateAssessedActions(team);

    }

    // 3. swap randomly selected edges
    // With at least two edges
    proba = params.mutation.tpg.pSwapActionProgram;
    while (team->getOutgoingEdges().size() > 2 &&
            proba > rng.getDouble(0.0, 1.0)) {
        this->swapEdges(graph, *team, rng);

        // Decrement the proba of swapping two edges
        proba *= params.mutation.tpg.pSwapActionProgram;
    }

    bool anyMutationDone = false;
    do {
        std::vector<uint64_t> indexUsed;
        uint64_t index;
        // 4. mutate randomly selected program on action Edge.
        double proba = params.mutation.tpg.pMutateActionProgram;
        while (indexUsed.size() < team->getOutgoingEdges().size() &&
               proba > rng.getDouble(0.0, 1.0)) {

            // Pick a random edge not already used
            do {
                index = rng.getUnsignedInt64(
                    0, team->getOutgoingEdges().size() - 1);
            } while (std::find(indexUsed.begin(), indexUsed.end(), index) !=
                     indexUsed.end());
            indexUsed.push_back(index);

            auto iter = team->getOutgoingEdges().begin();
            std::advance(iter, index);

            this->mutateOutgoingEdge(graph, *iter, team->getAssessedActions(), manager, newSubAgents, params, rng);
            graph->updateAssessedActions(team);

            proba *= params.mutation.tpg.pMutateActionProgram;
            anyMutationDone = true;
        }
    } while (!anyMutationDone && params.mutation.tpg.pMutateActionProgram != 0.0);
    graph->orderActionEdges(team);
}

