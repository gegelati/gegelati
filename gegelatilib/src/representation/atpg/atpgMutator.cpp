

#include "representation/atpg/atpgMutator.h"

void Representation::ATPG::ATPGMutator::updateSpecificContext(
            EvoGraph::Graph& graph, Population& population,
            const RepresentationParameters& params,
            RNG::RNG& rng)
{
    TPG::TPGMutator::updateSpecificContext(graph, population, params, rng);

    std::vector<std::reference_wrapper<const EvoGraph::Vertex>> preExistingProgramTeams;
    auto& representationName = this->actionProgramRepresentationID;
    this->preExistingActionProgram.clear();

    // Separate teams with action programs from those without
    auto it = std::partition(
        this->preExistingTeams.begin(),
        this->preExistingTeams.end(),
        [&representationName](const EvoGraph::Vertex& vertex) {
            return (!vertex.hasProgram() || vertex.getProgram().getRepresentationID() != representationName);
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
    
    // Add agents contains by all aggregated populations.
    const Population& actionProgramPopulation = population.cGetSubPopulation(this->actionProgramRepresentationID);
    for(const Representation::Population& accessedPopulation: actionProgramPopulation.getAggregatedPopulations()){
        const std::vector<std::reference_wrapper<const Representation::Individual>> accessedAgents(accessedPopulation.getAgents());
        this->preExistingActionProgram.insert(
            this->preExistingActionProgram.end(),
            accessedAgents.begin(),
            accessedAgents.end()
        );
    }

}


bool Representation::ATPG::ATPGMutator::isConfigurationValid(const RepresentationParameters& params, const Output::OutputHandler& outputs) const
{
    if(outputs.sizeContinuous() == 0 && outputs.sizeDiscrete() == 0){
        throw std::runtime_error("ATPGMutator::initRandomPopulation: No outputs defined.");
    }
    
    if (params.tpg.maxInitOutgoingEdges < 2) {
        throw std::runtime_error(
            "A team should have at least two edges at initialisation.");
    }
    return true;
}

void Representation::ATPG::ATPGMutator::initRandomPopulation(EvoGraph::Graph& graph, Population& population, const RepresentationParameters& params, RNG::RNG& rng)
{
    this->isConfigurationValid(params, population.getOutputs());
    
    // Empty agent population
    population.clearAgents(graph);

    // Create teams, programs and Actions
    std::vector<std::reference_wrapper<const EvoGraph::Vertex>> leafVertices;
    std::vector<std::reference_wrapper<const EvoGraph::Vertex>> teams;
    std::vector<std::reference_wrapper<const Individual>> programAgents;


    for (size_t idx = 0; idx < params.nbIndividuals; idx++) {
        teams.push_back(dynamic_cast<const TPG::TpgIndividual&>(population.createAgent(graph)).getVertex());
    }

    // Connect each team with two distinct actions, through two distinct
    // programs Association here are determinists since randomness would
    // uselessly complicate the code while bringing no real value since anyway,
    // Programs have been initialized randomly.
    Mutator& programMutator = this->getSubMutator(this->programRepresentationID);
    Population& programPopulation = population.getSubPopulation(this->programRepresentationID);

    Mutator& actionProgramMutator = this->getSubMutator(this->actionProgramRepresentationID);
    Population& actionProgramPopulation = population.getSubPopulation(this->actionProgramRepresentationID);
    for (size_t i = 0; i < 2 * params.nbIndividuals; i++) {

        // Create a program agent
        programAgents.push_back(programMutator.initRandomAgent(graph, programPopulation, params, rng));

        // Create a program agent and a new team
        auto& actionProgram = actionProgramMutator.initRandomAgent(graph, actionProgramPopulation, params, rng);
        leafVertices.push_back(graph.addNewTeam());

        // Set the vertex program to the action program.
        graph.setVertexProgram(leafVertices.back(), actionProgram);

        // Add the edge
        graph.addNewEdge(teams.at(i / 2), leafVertices.back(),
                         programAgents.at(i));
    }

    this->addAditionnalEdges(graph, leafVertices, teams, programAgents, params, rng);
}

void Representation::ATPG::ATPGMutator::initRandomSpecificAgent(const Individual& agent, EvoGraph::Graph& graph, Population& population, const RepresentationParameters& params, RNG::RNG& rng)
{
    // First agent is initialized, check validity of the configuration.
    if(population.getAgents().size() == 1){
        this->isConfigurationValid(params, population.getOutputs());
    }

    population.emptyAgent(agent, graph);

    const EvoGraph::Vertex& vertex = dynamic_cast<const TPG::TpgIndividual&>(agent).getVertex();

    Mutator& programMutator = this->getSubMutator(this->programRepresentationID);
    Population& programPopulation = population.getSubPopulation(this->programRepresentationID);

    Mutator& actionProgramMutator = this->getSubMutator(this->actionProgramRepresentationID);
    Population& actionProgramPopulation = population.getSubPopulation(this->actionProgramRepresentationID);

    size_t nbEdges = rng.getUnsignedInt64(2, params.tpg.maxInitOutgoingEdges);
    for(size_t idx = 0; idx < nbEdges; idx++){

        // Create a program agent and a new team
        const Individual& actionProgram = actionProgramMutator.initRandomAgent(graph, actionProgramPopulation, params, rng);
        const EvoGraph::Team& leafVertex = graph.addNewTeam();

        // Set the vertex program to the action program.
        graph.setVertexProgram(leafVertex, actionProgram);

        // Add edge
        graph.addNewEdge(vertex, leafVertex,
                            programMutator.initRandomAgent(graph, programPopulation, params, rng));
    }
}


void Representation::ATPG::ATPGMutator::mutateEdgeDestination(
    EvoGraph::Graph& graph, const EvoGraph::Edge& edge,
    Population& population,
    std::vector<std::reference_wrapper<const Individual>>& newSubAgents,
    const RepresentationParameters& params, RNG::RNG& rng)
{
    // Should the new target be an action or a team
    bool targetAction =
        rng.getDouble(0, 1) < params.tpg.pEdgeDestinationIsAction;

    // Pick any target
    // Note: Having an action in all teams is no longer enforced,
    // as the presence of cycle in TPGs is not possible according to the current
    // mutation process.
    if (targetAction) {
        
        // Create a new team
        // When a team is dupplicated, it is dupplicated with its corresponding edges, but not the destination team, thus we have to create a new one.
        const EvoGraph::Vertex& target = graph.addNewTeam();

        std::reference_wrapper<const Individual> targetAgent = this->preExistingActionProgram.at(
            rng.getUnsignedInt64(0, this->preExistingActionProgram.size() - 1));


        // Target name agent is different from representation
        if(targetAgent.get().getRepresentationID() != this->actionProgramRepresentationID){
            // Try to find an aggregated population with this name (will throw if not found)
            Population& actionProgramPopulation = population.getSubPopulation(this->actionProgramRepresentationID);
            const Population& aggregatedPopulation = actionProgramPopulation.getAggregatedPopulation(targetAgent.get().getRepresentationID());

            // Dupplication to exchange the agent from aggregatedPopulation to actionProgramPopulation.
            const Representation::Individual& newTargetAgent = actionProgramPopulation.copyAgent(targetAgent, graph);
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

void Representation::ATPG::ATPGMutator::mutateOutgoingEdge(
    EvoGraph::Graph& graph, const EvoGraph::Edge& edge,
    Population& population,
    std::vector<std::reference_wrapper<const Individual>>& newSubAgents,
    const RepresentationParameters& params, RNG::RNG& rng)
{
    if(edge.getDestination().hasProgram() && edge.getDestination().getProgram().getRepresentationID() == this->actionProgramRepresentationID &&
       rng.getDouble(0.0, 1.0) < params.atpg.probaContextOverActionProgram){
       
        // copy program
        const Representation::Individual& newAgent = population.getSubPopulation(edge.getDestination().getProgram().getRepresentationID()).copyAgent(edge.getDestination().getProgram(), graph);

        // Clone vertex destination
        const EvoGraph::Vertex& newDestination = graph.cloneVertex(edge.getDestination());

        // Set the new destination
        graph.setEdgeDestination(edge, newDestination);

        // Set the mutated agent to the edge
        graph.setVertexProgram(newDestination, newAgent);

        // Add it to the list of new agent to be mutated.
        newSubAgents.push_back(newAgent);

    } else {
        const Individual& originAgent = edge.getProgram();
        // copy program
        const Representation::Individual& newAgent = population.getSubPopulation(originAgent.getRepresentationID()).copyAgent(originAgent, graph);

        // Set the mutated agent to the edge
        graph.setEdgeProgram(edge, newAgent);

        // Add it to the list of new agent to be mutated.
        newSubAgents.push_back(newAgent);

        // Edge target modification
        // As it Stephen kelly's work, Edge target modification is conditionned
        // to the modification of the prealable Edge.Program behavior.
        if (rng.getDouble(0.0, 1.0) < params.tpg.pEdgeDestinationChange) {
            mutateEdgeDestination(graph, edge, population, newSubAgents, params, rng);
        }
    }

    
}

void Representation::ATPG::ATPGMutator::mutateSubAgents(
            std::vector<std::reference_wrapper<const Individual>>& agents, EvoGraph::Graph& graph, 
            Population& population, const RepresentationParameters& params, 
            RNG::RNG& rng, uint64_t maxNbThreads)
{
    // Devide agents into program agents and action program agents
    std::vector<std::reference_wrapper<const Individual>> programAgents;
    std::vector<std::reference_wrapper<const Individual>> actionProgramAgents;
    for(const Representation::Individual& agent : agents){
        if(agent.getRepresentationID() == this->actionProgramRepresentationID){
            actionProgramAgents.push_back(agent);
        } else {
            programAgents.push_back(agent);
        }
    }

    // Mutate action program using the archive of TPG
    TPG::TPGMutator::mutateSubAgents(programAgents, graph, population, params, rng, maxNbThreads);

    
    // mutate action programs
    Mutator& actionProgramMutator = this->getSubMutator(this->actionProgramRepresentationID);
    Population& actionProgramPopulation = population.getSubPopulation(this->actionProgramRepresentationID);
    std::vector<std::reference_wrapper<const Individual>> newSubAgents;
    for(const Representation::Individual& agent : actionProgramAgents){
        actionProgramMutator.mutateAgent(
            agent, graph, actionProgramPopulation, newSubAgents, params, rng);
    }
    // Mutate the new subAgents with actionProgramMutator.
    actionProgramMutator.mutateSubAgents(
        newSubAgents, graph, actionProgramPopulation, params, rng, maxNbThreads);
}
