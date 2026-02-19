

#include "algorithm/atpg/atpgMutator.h"

void Algorithm::ATPG::ATPGMutator::updateSpecificContext(
            EvoGraph::Graph& graph, AgentManager& manager,
            const Learn::LearningParameters& params,
            RNG::RNG& rng)
{
    TPG::TPGMutator::updateSpecificContext(graph, manager, params, rng);

    std::vector<std::reference_wrapper<const EvoGraph::Vertex>> preExistingProgramTeams;
    auto& algorithmName = this->actionProgramAlgorithmID;
    this->preExistingActionProgram.clear();

    // Separate teams with action programs from those without
    auto it = std::partition(
        this->preExistingTeams.begin(),
        this->preExistingTeams.end(),
        [&algorithmName](const EvoGraph::Vertex& vertex) {
            return (!vertex.hasProgram() || vertex.getProgram().getAlgorithmID() != algorithmName);
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

    for(const EvoGraph::Vertex& vertex: preExistingProgramTeams){
        this->preExistingActionProgram.push_back(vertex.getProgram());
    }
    
    // Add agents contains by all aggregated managers.
    const AgentManager& actionProgramManager = manager.cGetSubManager(this->actionProgramAlgorithmID);
    for(const Algorithm::AgentManager& accessedManager: actionProgramManager.getAggregatedManagers()){
        const std::vector<std::reference_wrapper<const Algorithm::Agent>> accessedAgents(accessedManager.getAgents());
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

void Algorithm::ATPG::ATPGMutator::initRandomPopulation(EvoGraph::Graph& graph, AgentManager& manager, const Learn::LearningParameters& params, RNG::RNG& rng)
{
    this->isConfigurationValid(params, manager.getOutputs());
    
    // Empty agent manager
    manager.clearAgents(graph);

    // Create teams, programs and Actions
    std::vector<std::reference_wrapper<const EvoGraph::Vertex>> leafVertices;
    std::vector<std::reference_wrapper<const EvoGraph::Vertex>> teams;
    std::vector<std::reference_wrapper<const Agent>> programAgents;


    for (size_t idx = 0; idx < params.mutation.tpg.nbRoots; idx++) {
        teams.push_back(dynamic_cast<const TPG::TPGAgent&>(manager.createAgent(graph)).getVertex());
    }

    // Connect each team with two distinct actions, through two distinct
    // programs Association here are determinists since randomness would
    // uselessly complicate the code while bringing no real value since anyway,
    // Programs have been initialized randomly.
    Mutator& programMutator = this->getSubMutator(this->programAlgorithmID);
    AgentManager& programManager = manager.getSubManager(this->programAlgorithmID);

    Mutator& actionProgramMutator = this->getSubMutator(this->actionProgramAlgorithmID);
    AgentManager& actionProgramManager = manager.getSubManager(this->actionProgramAlgorithmID);
    for (size_t i = 0; i < 2 * params.mutation.tpg.nbRoots; i++) {

        // Create a program agent
        programAgents.push_back(programMutator.initRandomAgent(graph, programManager, params, rng));

        // Create a program agent and a new team
        auto& actionProgram = actionProgramMutator.initRandomAgent(graph, actionProgramManager, params, rng);
        leafVertices.push_back(graph.addNewTeam());

        // Set the vertex program to the action program.
        graph.setVertexProgram(leafVertices.back(), actionProgram);

        // Add the edge
        graph.addNewEdge(teams.at(i / 2), leafVertices.back(),
                         programAgents.at(i));
    }

    this->addAditionnalEdges(graph, leafVertices, teams, programAgents, params, rng);
}

void Algorithm::ATPG::ATPGMutator::initRandomSpecificAgent(const Agent& agent, EvoGraph::Graph& graph, AgentManager& manager, const Learn::LearningParameters& params, RNG::RNG& rng)
{
    // First agent is initialized, check validity of the configuration.
    if(manager.getAgents().size() == 1){
        this->isConfigurationValid(params, manager.getOutputs());
    }

    manager.emptyAgent(agent, graph);

    const EvoGraph::Vertex& vertex = dynamic_cast<const TPG::TPGAgent&>(agent).getVertex();

    Mutator& programMutator = this->getSubMutator(this->programAlgorithmID);
    AgentManager& programManager = manager.getSubManager(this->programAlgorithmID);

    Mutator& actionProgramMutator = this->getSubMutator(this->actionProgramAlgorithmID);
    AgentManager& actionProgramManager = manager.getSubManager(this->actionProgramAlgorithmID);

    size_t nbEdges = rng.getUnsignedInt64(2, params.mutation.tpg.maxInitOutgoingEdges);
    for(size_t idx = 0; idx < nbEdges; idx++){

        // Create a program agent and a new team
        const Agent& actionProgram = actionProgramMutator.initRandomAgent(graph, actionProgramManager, params, rng);
        const EvoGraph::Team& leafVertex = graph.addNewTeam();

        // Set the vertex program to the action program.
        graph.setVertexProgram(leafVertex, actionProgram);

        // Add edge
        graph.addNewEdge(vertex, leafVertex,
                            programMutator.initRandomAgent(graph, programManager, params, rng));
    }
}


void Algorithm::ATPG::ATPGMutator::mutateEdgeDestination(
    EvoGraph::Graph& graph, const EvoGraph::Edge& edge,
    AgentManager& manager,
    std::vector<std::reference_wrapper<const Agent>>& newSubAgents,
    const Learn::LearningParameters& params, RNG::RNG& rng)
{
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
        const EvoGraph::Vertex& target = graph.addNewTeam();

        std::reference_wrapper<const Agent> targetAgent = this->preExistingActionProgram.at(
            rng.getUnsignedInt64(0, this->preExistingActionProgram.size() - 1));


        // Target name agent is different from algorithm
        if(targetAgent.get().getAlgorithmID() != this->actionProgramAlgorithmID){
            // Try to find an aggregated manager with this name (will throw if not found)
            AgentManager& actionProgramManager = manager.getSubManager(this->actionProgramAlgorithmID);
            const AgentManager& aggregatedManager = actionProgramManager.getAggregatedManager(targetAgent.get().getAlgorithmID());

            // Dupplication to exchange the agent from aggregatedManager to actionProgramManager.
            const Algorithm::Agent& newTargetAgent = actionProgramManager.copyAgent(targetAgent, graph);
            targetAgent = newTargetAgent;
        }

        // Set corresponding agent to the new team
        graph.setVertexProgram(target, targetAgent);

        // Change the target
        // Changing the target should not fail.
        graph.setEdgeDestination(edge, target);
    } else {
        const EvoGraph::Vertex& target = this->preExistingTeams.at(
            rng.getUnsignedInt64(0, this->preExistingTeams.size() - 1));

        // Change the target
        // Changing the target should not fail.
        graph.setEdgeDestination(edge, target);
    }

}

void Algorithm::ATPG::ATPGMutator::mutateOutgoingEdge(
    EvoGraph::Graph& graph, const EvoGraph::Edge& edge,
    AgentManager& manager,
    std::vector<std::reference_wrapper<const Agent>>& newSubAgents,
    const Learn::LearningParameters& params, RNG::RNG& rng)
{
    if(edge.getDestination().hasProgram() && edge.getDestination().getProgram().getAlgorithmID() == this->actionProgramAlgorithmID &&
       rng.getDouble(0.0, 1.0) < params.mutation.tpg.pMutateActionProgram){
       
        // copy program
        const Algorithm::Agent& newAgent = manager.getSubManager(edge.getDestination().getProgram().getAlgorithmID()).copyAgent(edge.getDestination().getProgram(), graph);

        // Clone vertex destination
        const EvoGraph::Vertex& newDestination = graph.cloneVertex(edge.getDestination());

        // Set the new destination
        graph.setEdgeDestination(edge, newDestination);

        // Set the mutated agent to the edge
        graph.setVertexProgram(newDestination, newAgent);

        // Add it to the list of new agent to be mutated.
        newSubAgents.push_back(newAgent);

    } else {
        const Agent& originAgent = edge.getProgram();
        // copy program
        const Algorithm::Agent& newAgent = manager.getSubManager(originAgent.getAlgorithmID()).copyAgent(originAgent, graph);

        // Set the mutated agent to the edge
        graph.setEdgeProgram(edge, newAgent);

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
            std::vector<std::reference_wrapper<const Agent>>& agents, EvoGraph::Graph& graph, 
            AgentManager& manager, const Learn::LearningParameters& params, 
            RNG::RNG& rng, uint64_t maxNbThreads)
{
    // Devide agents into program agents and action program agents
    std::vector<std::reference_wrapper<const Agent>> programAgents;
    std::vector<std::reference_wrapper<const Agent>> actionProgramAgents;
    for(const Algorithm::Agent& agent : agents){
        if(agent.getAlgorithmID() == this->actionProgramAlgorithmID){
            actionProgramAgents.push_back(agent);
        } else {
            programAgents.push_back(agent);
        }
    }

    // Mutate action program using the archive of TPG
    TPG::TPGMutator::mutateSubAgents(programAgents, graph, manager, params, rng, maxNbThreads);

    
    // mutate action programs
    Mutator& actionProgramMutator = this->getSubMutator(this->actionProgramAlgorithmID);
    AgentManager& actionProgramManager = manager.getSubManager(this->actionProgramAlgorithmID);
    std::vector<std::reference_wrapper<const Agent>> newSubAgents;
    for(const Algorithm::Agent& agent : actionProgramAgents){
        actionProgramMutator.mutateAgent(
            agent, graph, actionProgramManager, newSubAgents, params, rng);
    }
    // Mutate the new subAgents with actionProgramMutator.
    actionProgramMutator.mutateSubAgents(
        newSubAgents, graph, actionProgramManager, params, rng, maxNbThreads);
}
