

#include "algorithm/atpg/atpgMutator.h"

void Algorithm::ATPG::ATPGMutator::updateSpecificContext(
            std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, std::shared_ptr<Selector::Selector> selector,
            const Learn::LearningParameters& params,
            RNG::RNG& rng)
{
    TPG::TPGMutator::updateSpecificContext(graph, manager, selector, params, rng);

    auto& algorithmName = this->actionProgramAlgorithmName;
    // Remove teams that contains an action program
    this->preExistingTeams.erase(
        std::remove_if(
            this->preExistingTeams.begin(),
            this->preExistingTeams.end(),
            [&algorithmName](const std::shared_ptr<const EvoGraph::Team>& vertex) {
                return (vertex->getProgram() != nullptr && vertex->getProgram()->getAlgorithmName() == algorithmName);
            }
        ),
        preExistingTeams.end()
    );
}


bool Algorithm::ATPG::ATPGMutator::isConfigurationValid(const Learn::LearningParameters& params, const Output::OutputHandler& outputs) const
{
    if(outputs.sizeContinuous() == 0 && outputs.sizeDiscrete() == 0){
        throw std::runtime_error("ATPGMutator::initRandomPopulation: No outputs defined.");
    }
    
    if (params.mutation.tpg.maxInitOutgoingEdges < 2) {
        throw std::runtime_error(
            "A team should have at least two edges at initialisation.");
    }
    return true;
}

void Algorithm::ATPG::ATPGMutator::initRandomPopulation(std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Learn::LearningParameters& params, RNG::RNG& rng)
{
    auto outputs = manager->getOutputs();
    this->isConfigurationValid(params, outputs);
    
    // Empty agent manager
    manager->clearAgents();

    // Create teams, programs and Actions
    std::vector<std::shared_ptr<const EvoGraph::Action>> actions;
    std::vector<std::shared_ptr<const EvoGraph::Vertex>> teams;
    std::vector<std::shared_ptr<const Agent>> programAgent;


    for (size_t idx = 0; idx < params.mutation.tpg.nbRoots; idx++) {
        teams.push_back(std::dynamic_pointer_cast<const ATPGAgent>(manager->createAgent(graph))->getVertex());
    }

    // Connect each team with two distinct actions, through two distinct
    // programs Association here are determinists since randomness would
    // uselessly complicate the code while bringing no real value since anyway,
    // Programs have been initialized randomly.
    auto programMutator = this->getSubMutator(this->programAlgorithmName);
    auto programManager = manager->getSubManager(this->programAlgorithmName);
    for (size_t i = 0; i < 2 * params.mutation.tpg.nbRoots; i++) {

        // Create a program agent
        programAgent.push_back(programMutator->initRandomAgent(graph, programManager, params, rng));

        // Add the edge
        graph->addNewEdge(*teams.at(i / 2), *actions.at(i % actions.size()),
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
                *actions.at(rng.getUnsignedInt64(0, actions.size() - 1)),
                programAgent.at(selectedProgramIndex));
        }
    }
}

void Algorithm::ATPG::ATPGMutator::initRandomSpecificAgent(std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Learn::LearningParameters& params, RNG::RNG& rng)
{
    auto tpgAgent = std::dynamic_pointer_cast<const ATPGAgent>(manager->createAgent(graph))->getVertex();

    auto programMutator = this->getSubMutator(this->programAlgorithmName);
    auto programManager = manager->getSubManager(this->programAlgorithmName);

    // Get action vertices in the graph
    std::vector<std::shared_ptr<const EvoGraph::Action>> actions;
    for (const auto& vertex : graph->getVertices()) {
        if (auto action = std::dynamic_pointer_cast<const EvoGraph::Action>(vertex)) {
            actions.push_back(action);
        }
    }

    if(actions.size() == 0){
        throw std::runtime_error("TPGMutator::initRandomSpecificAgent: No action vertices in the graph.");
    }

    // Add the edge
    graph->addNewEdge(*tpgAgent, *actions.at(rng.getUnsignedInt64(0, actions.size() - 1)),
                        programMutator->initRandomAgent(graph, programManager, params, rng));
}


void Algorithm::ATPG::ATPGMutator::mutateEdgeDestination(
    std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<const EvoGraph::Edge> edge,
    std::shared_ptr<AgentManager> manager,
    std::vector<std::shared_ptr<const Agent>>& newSubAgents,
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

        const auto& subAgents = this->getSubMutator(this->actionProgramAlgorithmName)->getContext().preExistingAgents;

        auto originAgent = subAgents.at(rng.getUnsignedInt64(
            0, subAgents.size() - 1));

        // copy program
        std::shared_ptr<const Algorithm::Agent> newAgent = manager->getSubManager(originAgent->getAlgorithmName())->copyAgent(originAgent, graph);

        newSubAgents.push_back(newAgent);

        // Since newAgent is a copy of a program set on a vertex, its element must be a vertex too.
        auto elementAgent = newAgent->getElement();
        if(auto vertexAgent = std::dynamic_pointer_cast<const EvoGraph::Vertex>(elementAgent)){
            target = vertexAgent;
        } else {
            throw std::runtime_error("ATPGMutator::mutateEdgeDestination: Action program's element is not a vertex.");
        }

    } else {
        target = this->preExistingTeams.at(
            rng.getUnsignedInt64(0, this->preExistingTeams.size() - 1));
    }

    // Change the target
    // Changing the target should not fail.
    graph->setEdgeDestination(*edge, *target);
}

void Algorithm::ATPG::ATPGMutator::mutateOutgoingEdge(
    std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<const EvoGraph::Edge> edge,
    std::shared_ptr<AgentManager> manager,
    std::vector<std::shared_ptr<const Agent>>& newSubAgents,
    const Learn::LearningParameters& params, RNG::RNG& rng)
{
    auto originAgent = edge->getProgram();
    // copy program
    std::shared_ptr<const Algorithm::Agent> newAgent = manager->getSubManager(originAgent->getAlgorithmName())->copyAgent(originAgent, graph);

    // Set the mutated agent to the edge
    graph->setEdgeProgram(*edge, newAgent);

    // Add it to the list of new agent to be mutated.
    newSubAgents.push_back(newAgent);

    // Edge target modification
    // As it Stephen kelly's work, Edge target modification is conditionned
    // to the modification of the prealable Edge.Program behavior.
    if (rng.getDouble(0.0, 1.0) < params.mutation.tpg.pEdgeDestinationChange) {
        mutateEdgeDestination(graph, edge, manager, newSubAgents, params, rng);
    }
}
