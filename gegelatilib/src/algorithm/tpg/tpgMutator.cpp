
#include "algorithm/tpg/tpgMutator.h"
#include <queue>


void Algorithm::TPG::TPGMutator::updateSpecificContext(
    std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, std::shared_ptr<Selector::Selector> selector,
    const Learn::LearningParameters& params,
    RNG::RNG& rng, size_t nbOutputs)
{
    // Call parent method to update currentContext
    Algorithm::Mutator::updateSpecificContext(graph, manager, selector, params, rng, nbOutputs);

    // Update pre-existing elements
    this->preExistingTeams.clear();
    this->preExistingActions.clear();
    this->preExistingEdges.clear();

    /*std::set<std::shared_ptr<const EvoGraph::Edge>, SharedLess<EvoGraph::Edge>> usableEdges;
    std::set<std::shared_ptr<const EvoGraph::Vertex>, SharedLess<EvoGraph::Vertex>> usableVertices;
    std::queue<std::shared_ptr<const EvoGraph::Vertex>> toVisit;

    // Initialize queue with vertices from all pre-existing agents
    for (auto agentPtr : this->currentContext->preExistingAgents) {
        if(auto tpgAgent = std::dynamic_pointer_cast<const TPGAgent>(agentPtr)) {
            toVisit.push(tpgAgent->getVertex());
        } else {
            throw std::runtime_error("TPGMutator::updateSpecificContext: an agent in the current context is not a TPGAgent.");

        }

    }

    // BFS to collect all vertices reachable from pre-existing agents
    while (!toVisit.empty()) {
        std::shared_ptr<const EvoGraph::Vertex> vertex = toVisit.front();
        toVisit.pop();

        // Skip if already visited
        if (usableVertices.find(vertex) != usableVertices.end()) {
            continue;
        }
        usableVertices.insert(vertex);

        // Add all connected vertices to the queue
        // Outgoing edges: vertices that this vertex points to
        for (auto edge : vertex->getOutgoingEdges()) {
            auto destination = edge->getDestination();
            if (usableVertices.find(destination) == usableVertices.end()) {
                toVisit.push(destination);
            }
        }
    }

    for(auto vertex: usableVertices){
        if(auto team = std::dynamic_pointer_cast<const EvoGraph::Team>(vertex)){
            this->preExistingTeams.push_back(team);
        } else if (auto action = std::dynamic_pointer_cast<const EvoGraph::Action>(vertex)){
            this->preExistingActions.push_back(action);
        } else {
            throw std::runtime_error("TPGMutator::updateSpecificContext: a vertex should be either a team or an action.");
        }

        usableEdges.insert(vertex->getOutgoingEdges().begin(), vertex->getOutgoingEdges().end());
    }

    this->preExistingEdges.insert(this->preExistingEdges.end(), usableEdges.begin(), usableEdges.end());*/

    
    // Get current vertex set (copy)
    auto vertices(graph->getVertices());
    // Get current agent teams (copy)
    auto agentVertices(graph->getRootVertices());

    // Fill the list of available Team and Actions, Actions are only
    // agents if they are not accessible by the teams
    this->preExistingTeams.clear();
    this->preExistingActions.clear();
    for (auto vertex : vertices) {
        if (std::dynamic_pointer_cast<const EvoGraph::Action>(vertex) != nullptr &&
            (params.mutation.tpg.teamAccessAllActions ||
             vertex->getIncomingEdges().size() == 0)) {
            this->preExistingActions.push_back(
                std::dynamic_pointer_cast<const EvoGraph::Action>(vertex));
        }
        else if (std::dynamic_pointer_cast<const EvoGraph::Team>(vertex) != nullptr) {
            this->preExistingTeams.push_back(
                std::dynamic_pointer_cast<const EvoGraph::Team>(vertex));
        }
    }

    // Fill the list of pre existing edges before mutations (copy)
    this->preExistingEdges.clear();
    for(auto edge: graph->getEdges()){
        this->preExistingEdges.push_back(edge);
    }

}

void Algorithm::TPG::TPGMutator::initRandomPopulation(std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Learn::LearningParameters& params, RNG::RNG& rng, size_t nbOutputs)
{

    if (params.mutation.tpg.maxInitOutgoingEdges > nbOutputs) {
        throw std::runtime_error("Maximum initial number of outgoing edges "
                                    "cannot exceed the number of outputs");
    }
    if (nbOutputs < 2) {
        throw std::runtime_error(
            "A TPG with a single output makes no sense.");
    }
    if (params.mutation.tpg.maxInitOutgoingEdges < 2) {
        throw std::runtime_error(
            "A team should have at least two edges at initialisation.");
    }

    // Empty agent manager
    manager->clearAgents();

    // Create teams, programs and Actions
    std::vector<std::shared_ptr<const EvoGraph::Action>> actions;
    std::vector<std::shared_ptr<const EvoGraph::Vertex>> teams;
    std::vector<std::shared_ptr<const Agent>> programAgent;


    for (size_t idx = 0; idx < nbOutputs; idx++) {
        actions.push_back(graph->addNewAction(idx));
    }
    for (size_t idx = 0; idx < params.mutation.tpg.nbRoots; idx++) {
        teams.push_back(std::dynamic_pointer_cast<const TPGAgent>(manager->createAgent(graph))->getVertex());
    }

    // Connect each team with two distinct actions, through two distinct
    // programs Association here are determinists since randomness would
    // uselessly complicate the code while bringing no real value since anyway,
    // Programs have been initialized randomly.
    auto programMutator = this->getSubMutator(this->programAlgorithmName);
    auto programManager = manager->getSubManager(this->programAlgorithmName);
    for (size_t i = 0; i < 2 * params.mutation.tpg.nbRoots; i++) {

        // Create a program agent
        programAgent.push_back(programMutator->initRandomAgent(graph, programManager, params, rng, 1));

        // Add the edge
        graph->addNewEdge(*teams.at(i / 2), *actions.at(i % nbOutputs),
                         programAgent.at(i));
    }

    // Add additional connections to TPG
    // Team-by-Team
    for (std::shared_ptr<const EvoGraph::Vertex> team : teams) {
        // Pick a number of additional outedge
        size_t nbAdditionalEdges =
            rng.getUnsignedInt64(0, params.mutation.tpg.maxInitOutgoingEdges - 2);

        // For each additional edge to add
        for (uint64_t i = 0; i < nbAdditionalEdges; i++) {
            // Pick 2 random programs not already used by the Team
            int64_t randomProgIndex[2] = {-1, -1};
            int pickedProgram = 0;
            {
                // Copy the list of programs
                std::vector<int> availableChoices(programAgent.size());
                std::iota(availableChoices.begin(), availableChoices.end(), 0);
                // Remove already connected ones
                auto iter = availableChoices.begin();
                while (iter < availableChoices.end()) {
                    if (std::count_if(
                            team->getOutgoingEdges().begin(),
                            team->getOutgoingEdges().end(),
                            [&iter, &programAgent](std::shared_ptr<const EvoGraph::Edge> edge) {
                                return edge->getProgram() ==
                                       programAgent.at(*iter);
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
                 programAgent.at(randomProgIndex[1]).use_count() <
                     programAgent.at(randomProgIndex[0]).use_count())
                    ? randomProgIndex[1]
                    : randomProgIndex[0];

            // Add the connection
            graph->addNewEdge(
                *team,
                *actions.at(rng.getUnsignedInt64(0, nbOutputs - 1)),
                programAgent.at(selectedProgramIndex));
        }
    }
}

std::shared_ptr<const Algorithm::Agent> Algorithm::TPG::TPGMutator::initRandomAgent(std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Learn::LearningParameters& params, RNG::RNG& rng, size_t nbOutputs)
{
    return nullptr;
}

void Algorithm::TPG::TPGMutator::crossoverAgents(
    std::vector<std::shared_ptr<const Agent>> agents, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Selector::SelectionContext& context, std::vector<std::shared_ptr<const Agent>> newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng)
{
    /* CROSSOVER ARE NOT USED IN TPG */
}



void Algorithm::TPG::TPGMutator::removeRandomEdge(std::shared_ptr<EvoGraph::Graph> graph,
                                                const EvoGraph::Team& team,
                                                RNG::RNG& rng)
{
    // Pick an outgoing edge randomly,
    auto pickableEdges = team.getOutgoingEdges();

    // Note: No need to take special care of Actions. Since cycles can not
    // appear in TPG with the current mutation process, there is no need to
    // maintain an action within each team.

    // Pick a random edge
    auto iterSet = pickableEdges.begin();
    std::advance(iterSet, rng.getUnsignedInt64(0, pickableEdges.size() - 1));
    auto removedEdge = *iterSet;
    graph->removeEdge(*removedEdge);
}



void Algorithm::TPG::TPGMutator::addRandomEdge(
    std::shared_ptr<EvoGraph::Graph> graph, const EvoGraph::Team& team,
    const Selector::SelectionContext& context, RNG::RNG& rng)
{
    // Pick an edge (excluding ones from the team, edges with the team as a
    // destination and the edges that are action edges)
    auto pickableEdges(this->preExistingEdges);
    // cf erase-remove idiom
    pickableEdges.erase(
        std::remove_if(pickableEdges.begin(), pickableEdges.end(),
                       [&team](std::shared_ptr<const EvoGraph::Edge> edge) -> bool {
                           return edge == nullptr ||
                                  std::dynamic_pointer_cast<const EvoGraph::ActionEdge>(
                                      edge) != nullptr ||
                                  edge->getSource().get() == &team ||
                                  edge->getDestination().get() == &team;
                       }),
        pickableEdges.end());

    // Pick a pickable Edge
    // (This code assumes that the set of pickable edge is never empty..
    // otherwise it will throw an exception. Possible solution if needed
    // initialize an entirely new program and pick a random target.)
    auto iter = pickableEdges.begin();
    std::advance(iter, rng.getUnsignedInt64(0, pickableEdges.size() - 1));
    std::shared_ptr<const EvoGraph::Edge> pickedEdge = *iter;

    // Create new edge from team and with the same ProgramSharedPointer
    // But with the team as its source
    // throw std::runtime_error if the edge is not from the graph;
    graph->setEdgeSource(*graph->cloneEdge(*pickedEdge), team);
}

void Algorithm::TPG::TPGMutator::mutateEdgeDestination(
    std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<const EvoGraph::Edge> edge,
    const Selector::SelectionContext& context,
    const Learn::LearningParameters& params, RNG::RNG& rng)
{
    // Pick an edge among preexisting vertices
    std::shared_ptr<const EvoGraph::Vertex> target = nullptr;

    // Should the new target be an action or a team
    bool targetAction =
        rng.getDouble(0, 1) < params.mutation.tpg.pEdgeDestinationIsAction;

    // Pick any target
    // Note: Having an action in all teams is no longer enforced,
    // as the presence of cycle in TPGs is not possible according to the current
    // mutation process.
    if (targetAction) {
        target = this->preExistingActions.at(rng.getUnsignedInt64(
            0, this->preExistingActions.size() - 1));
    } else {
        target = this->preExistingTeams.at(
            rng.getUnsignedInt64(0, this->preExistingTeams.size() - 1));
    }

    // Change the target
    // Changing the target should not fail.
    graph->setEdgeDestination(*edge, *target);
}

void Algorithm::TPG::TPGMutator::mutateOutgoingEdge(
    std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<const EvoGraph::Edge> edge,
    std::shared_ptr<AgentManager> manager,
    const Selector::SelectionContext& context,
    std::vector<std::shared_ptr<const Agent>> newSubAgents,
    const Learn::LearningParameters& params, RNG::RNG& rng)
{
    auto originAgent = edge->getProgram();
    // copy program
    std::shared_ptr<const Algorithm::Agent> newAgent = manager->getSubManager(originAgent->getAlgorithmName())->copyAgent(originAgent, graph);

    // Set the mutated agent to the edge
    edge->setProgram(newAgent);

    // Add it to the list of new agent to be mutated.
    newSubAgents.push_back(newAgent);

    // Edge target modification
    // As it Stephen kelly's work, Edge target modification is conditionned
    // to the modification of the prealable Edge.Program behavior.
    if (rng.getDouble(0.0, 1.0) < params.mutation.tpg.pEdgeDestinationChange) {
        mutateEdgeDestination(graph, edge, context, params, rng);
    }
}

void Algorithm::TPG::TPGMutator::mutateAgent(
    std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Selector::SelectionContext& context, std::vector<std::shared_ptr<const Agent>> newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng)
{
    std::shared_ptr<const EvoGraph::Vertex> vertex = std::dynamic_pointer_cast<const TPGAgent>(agent)->getVertex();
    std::shared_ptr<const EvoGraph::Team> team = std::dynamic_pointer_cast<const EvoGraph::Team>(vertex);

    // 1. Remove randomly selected edges
    {
        // Keep at least two edges (otherwise the team is useless)
        double proba = 1.0;
        while (team->getOutgoingEdges().size() > 2 &&
               proba > rng.getDouble(0.0, 1.0)) {

            this->removeRandomEdge(graph, *team, rng);

            // Decrement the proba of removing another edge
            proba *= params.mutation.tpg.pEdgeDeletion;
        }
    }

    // 2. Add random duplicated edge with the team as its source
    {
        double proba = 1.0;
        while (team->getOutgoingEdges().size() < params.mutation.tpg.maxOutgoingEdges &&
               proba > rng.getDouble(0.0, 1.0)) {
            // Add an edge (by duplication of an existing one)
            this->addRandomEdge(graph, *team, context, rng);
            // Decrement the proba of adding another edge
            proba *= params.mutation.tpg.pEdgeAddition;
        }
    }

    // 3. Mutate edges of the team
    {
        bool anyMutationDone = false;
        do {
            // Process edge-by-edge
            // And possibly modify their target
            for (std::shared_ptr<const EvoGraph::Edge> edge : team->getOutgoingEdges()) {
                // Edge->Program bid modification
                if (rng.getDouble(0.0, 1.0) < params.mutation.tpg.pProgramMutation) {
                    // Mutate the edge
                    this->mutateOutgoingEdge(graph, edge, manager, context, newSubAgents,
                                       params, rng);
                    anyMutationDone = true;
                }
            }
        } while (!anyMutationDone);
    }
}
