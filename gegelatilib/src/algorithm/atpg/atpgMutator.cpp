

#include "algorithm/atpg/atpgMutator.h"

void Algorithm::ATPG::ATPGMutator::updateSpecificContext(
            std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager,
            const Learn::LearningParameters& params,
            RNG::RNG& rng)
{
    TPG::TPGMutator::updateSpecificContext(graph, manager, params, rng);

    std::vector<std::shared_ptr<const EvoGraph::Team>> preExistingProgramTeams;
    auto& algorithmName = this->actionProgramAlgorithmName;
    this->preExistingActionProgram.clear();

    // Separate teams with action programs from those without
    auto it = std::partition(
        this->preExistingTeams.begin(),
        this->preExistingTeams.end(),
        [&algorithmName](const std::shared_ptr<const EvoGraph::Team>& vertex) {
            auto locked = vertex->getProgram().lock();
            return (!locked || locked->getAlgorithmName() != algorithmName);
        }
    );

    // Add the deleted teams to preExistingProgramTeams
    preExistingProgramTeams.insert(
        preExistingProgramTeams.end(),
        it,
        this->preExistingTeams.end()
    );

    // Remove teams with action programs from preExistingTeams
    this->preExistingTeams.erase(it, this->preExistingTeams.end());

    for(const auto& team: preExistingProgramTeams){
        this->preExistingActionProgram.push_back(team->getProgram());
    }
    
    // Add agents contains by all aggregated managers.
    const AgentManager& actionProgramManager = *manager->cGetSubManager(this->actionProgramAlgorithmName);
    for(auto const& accessedManager: actionProgramManager.getAggregatedManagers()){
        const std::vector<std::weak_ptr<const Algorithm::Agent>> accessedAgents(accessedManager.get().getAgents());
        this->preExistingActionProgram.insert(
            this->preExistingActionProgram.end(),
            accessedAgents.begin(),
            accessedAgents.end()
        );
    }

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
    this->isConfigurationValid(params, manager->getOutputs());
    
    // Empty agent manager
    manager->clearAgents(graph);

    // Create teams, programs and Actions
    std::vector<std::shared_ptr<const EvoGraph::Vertex>> leafVertices;
    std::vector<std::shared_ptr<const EvoGraph::Vertex>> teams;
    std::vector<std::weak_ptr<const Agent>> programAgents;


    for (size_t idx = 0; idx < params.mutation.tpg.nbRoots; idx++) {
        teams.push_back(std::dynamic_pointer_cast<const TPG::TPGAgent>(manager->createAgent(graph).lock())->getVertex());
    }

    // Connect each team with two distinct actions, through two distinct
    // programs Association here are determinists since randomness would
    // uselessly complicate the code while bringing no real value since anyway,
    // Programs have been initialized randomly.
    auto programMutator = this->getSubMutator(this->programAlgorithmName);
    auto programManager = manager->getSubManager(this->programAlgorithmName);

    auto actionProgramMutator = this->getSubMutator(this->actionProgramAlgorithmName);
    auto actionProgramManager = manager->getSubManager(this->actionProgramAlgorithmName);
    for (size_t i = 0; i < 2 * params.mutation.tpg.nbRoots; i++) {

        // Create a program agent
        programAgents.push_back(programMutator->initRandomAgent(graph, programManager, params, rng));

        // Create a program agent and a new team
        auto actionProgram = actionProgramMutator->initRandomAgent(graph, actionProgramManager, params, rng);
        leafVertices.push_back(graph->addNewTeam());

        // Set the vertex program to the action program.
        graph->setVertexProgram(*leafVertices.back(), actionProgram);

        // Add the edge
        graph->addNewEdge(*teams.at(i / 2), *leafVertices.back(),
                         programAgents.at(i));
    }

    this->addAditionnalEdges(graph, leafVertices, teams, programAgents, params, rng);
}

void Algorithm::ATPG::ATPGMutator::initRandomSpecificAgent(const Agent& agent, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Learn::LearningParameters& params, RNG::RNG& rng)
{
    // First agent is initialized, check validity of the configuration.
    if(manager->getAgents().size() == 1){
        this->isConfigurationValid(params, manager->getOutputs());
    }

    auto tpgAgent = std::dynamic_pointer_cast<const TPG::TPGAgent>(manager->createAgent(graph).lock())->getVertex();

    auto programMutator = this->getSubMutator(this->programAlgorithmName);
    auto programManager = manager->getSubManager(this->programAlgorithmName);

    auto actionProgramMutator = this->getSubMutator(this->actionProgramAlgorithmName);
    auto actionProgramManager = manager->getSubManager(this->actionProgramAlgorithmName);

    size_t nbEdges = rng.getUnsignedInt64(2, params.mutation.tpg.maxInitOutgoingEdges);
    for(size_t idx = 0; idx < nbEdges; idx++){

        // Create a program agent and a new team
        auto actionProgram = actionProgramMutator->initRandomAgent(graph, actionProgramManager, params, rng);
        auto leafVertex = graph->addNewTeam();

        // Set the vertex program to the action program.
        graph->setVertexProgram(*leafVertex, actionProgram);

        // Add edge
        graph->addNewEdge(*tpgAgent, *leafVertex,
                            programMutator->initRandomAgent(graph, programManager, params, rng));
    }
}


void Algorithm::ATPG::ATPGMutator::mutateEdgeDestination(
    std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<const EvoGraph::Edge> edge,
    std::shared_ptr<AgentManager> manager,
    std::vector<std::weak_ptr<const Agent>>& newSubAgents,
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
        
        // Create a new team
        // When a team is dupplicated, it is dupplicated with its corresponding edges, but not the destination team, thus we have to create a new one.
        target = graph->addNewTeam();

        std::weak_ptr<const Agent> targetAgent = this->preExistingActionProgram.at(
            rng.getUnsignedInt64(0, this->preExistingActionProgram.size() - 1));

        const Agent& targetAgentRef = *targetAgent.lock();

        // Target name agent is different from algorithm
        if(targetAgentRef.getAlgorithmName() != this->actionProgramAlgorithmName){
            // Try to find an aggregated manager with this name (will throw if not found)
            AgentManager& actionProgramManager = *manager->getSubManager(this->actionProgramAlgorithmName);
            const AgentManager& aggregatedManager = actionProgramManager.getAggregatedManager(targetAgentRef.getAlgorithmName());

            // Dupplication to exchange the agent from aggregatedManager to actionProgramManager.
            std::weak_ptr<const Agent> newTargetAgent = actionProgramManager.copyAgent(targetAgentRef, graph);
            targetAgent = newTargetAgent;
        }

        // Set corresponding agent to the new team
        graph->setVertexProgram(*target, targetAgent);
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
    std::vector<std::weak_ptr<const Agent>>& newSubAgents,
    const Learn::LearningParameters& params, RNG::RNG& rng)
{
    auto agentProgramDestLock = edge->getDestination()->getProgram().lock();
    if(!agentProgramDestLock && agentProgramDestLock->getAlgorithmName() == this->actionProgramAlgorithmName &&
       rng.getDouble(0.0, 1.0) < params.mutation.tpg.pMutateActionProgram){
       
        const Agent& originAgent = *agentProgramDestLock;
        // copy program
        std::weak_ptr<const Algorithm::Agent> newAgent = manager->getSubManager(originAgent.getAlgorithmName())->copyAgent(originAgent, graph);

        // Clone vertex destination
        auto newDestination = graph->cloneVertex(*std::dynamic_pointer_cast<const EvoGraph::Team>(edge->getDestination()));

        // Set the new destination
        graph->setEdgeDestination(*edge, *newDestination);

        // Set the mutated agent to the edge
        graph->setVertexProgram(*newDestination, newAgent);

        // Add it to the list of new agent to be mutated.
        newSubAgents.push_back(newAgent);

    } else {
        const Agent& originAgent = *edge->getProgram().lock();
        // copy program
        std::weak_ptr<const Algorithm::Agent> newAgent = manager->getSubManager(originAgent.getAlgorithmName())->copyAgent(originAgent, graph);

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

    
}

void Algorithm::ATPG::ATPGMutator::mutateSubAgents(
            std::vector<std::weak_ptr<const Agent>>& agents, std::shared_ptr<EvoGraph::Graph> graph, 
            std::shared_ptr<AgentManager> manager, const Learn::LearningParameters& params, 
            RNG::RNG& rng, uint64_t maxNbThreads)
{
    // Devide agents into program agents and action program agents
    std::vector<std::weak_ptr<const Agent>> programAgents;
    std::vector<std::weak_ptr<const Agent>> actionProgramAgents;
    for(const auto& agent : agents){
        if(agent.lock()->getAlgorithmName() == this->actionProgramAlgorithmName){
            actionProgramAgents.push_back(agent);
        } else {
            programAgents.push_back(agent);
        }
    }

    // Mutate action program using the archive of TPG
    TPG::TPGMutator::mutateSubAgents(programAgents, graph, manager, params, rng, maxNbThreads);

    
    // mutate action programs
    auto actionProgramMutator = this->getSubMutator(this->actionProgramAlgorithmName);
    auto actionProgramManager = manager->getSubManager(this->actionProgramAlgorithmName);
    std::vector<std::weak_ptr<const Agent>> newSubAgents;
    for(auto & agent : actionProgramAgents){
        actionProgramMutator->mutateAgent(
            *agent.lock(), graph, actionProgramManager, newSubAgents, params, rng);
    }
    // Mutate the new subAgents with actionProgramMutator.
    actionProgramMutator->mutateSubAgents(
        newSubAgents, graph, actionProgramManager, params, rng, maxNbThreads);
}
