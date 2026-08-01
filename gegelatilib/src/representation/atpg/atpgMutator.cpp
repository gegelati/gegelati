

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
    
    // Add individuals contains by all aggregated populations.
    const Population& actionProgramPopulation = population.cGetSubPopulation(this->actionProgramRepresentationID);
    for(const Representation::Population& accessedPopulation: actionProgramPopulation.getAggregatedPopulations()){
        const std::vector<std::reference_wrapper<const Representation::Individual>> accessedIndividuals(accessedPopulation.getIndividuals());
        this->preExistingActionProgram.insert(
            this->preExistingActionProgram.end(),
            accessedIndividuals.begin(),
            accessedIndividuals.end()
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
    
    // Empty individual population
    population.clearIndividuals(graph);

    // Create teams, programs and Actions
    std::vector<std::reference_wrapper<const EvoGraph::Vertex>> leafVertices;
    std::vector<std::reference_wrapper<const EvoGraph::Vertex>> teams;
    std::vector<std::reference_wrapper<const Individual>> programIndividuals;


    for (size_t idx = 0; idx < params.nbIndividuals; idx++) {
        teams.push_back(dynamic_cast<const TPG::TpgIndividual&>(population.createIndividual(graph)).getVertex());
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

        // Create a program individual
        programIndividuals.push_back(programMutator.initRandomIndividual(graph, programPopulation, params, rng));

        // Create a program individual and a new team
        auto& actionProgram = actionProgramMutator.initRandomIndividual(graph, actionProgramPopulation, params, rng);
        leafVertices.push_back(graph.addNewTeam());

        // Set the vertex program to the action program.
        graph.setVertexProgram(leafVertices.back(), actionProgram);

        // Add the edge
        graph.addNewEdge(teams.at(i / 2), leafVertices.back(),
                         programIndividuals.at(i));
    }

    this->addAditionnalEdges(graph, leafVertices, teams, programIndividuals, params, rng);
}

void Representation::ATPG::ATPGMutator::initRandomSpecificIndividual(const Individual& individual, EvoGraph::Graph& graph, Population& population, const RepresentationParameters& params, RNG::RNG& rng)
{
    // First individual is initialized, check validity of the configuration.
    if(population.getIndividuals().size() == 1){
        this->isConfigurationValid(params, population.getOutputs());
    }

    population.emptyIndividual(individual, graph);

    const EvoGraph::Vertex& vertex = dynamic_cast<const TPG::TpgIndividual&>(individual).getVertex();

    Mutator& programMutator = this->getSubMutator(this->programRepresentationID);
    Population& programPopulation = population.getSubPopulation(this->programRepresentationID);

    Mutator& actionProgramMutator = this->getSubMutator(this->actionProgramRepresentationID);
    Population& actionProgramPopulation = population.getSubPopulation(this->actionProgramRepresentationID);

    size_t nbEdges = rng.getUnsignedInt64(2, params.tpg.maxInitOutgoingEdges);
    for(size_t idx = 0; idx < nbEdges; idx++){

        // Create a program individual and a new team
        const Individual& actionProgram = actionProgramMutator.initRandomIndividual(graph, actionProgramPopulation, params, rng);
        const EvoGraph::Team& leafVertex = graph.addNewTeam();

        // Set the vertex program to the action program.
        graph.setVertexProgram(leafVertex, actionProgram);

        // Add edge
        graph.addNewEdge(vertex, leafVertex,
                            programMutator.initRandomIndividual(graph, programPopulation, params, rng));
    }
}


void Representation::ATPG::ATPGMutator::mutateEdgeDestination(
    EvoGraph::Graph& graph, const EvoGraph::Edge& edge,
    Population& population,
    std::vector<std::reference_wrapper<const Individual>>& newSubIndividuals,
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

        std::reference_wrapper<const Individual> targetIndividual = this->preExistingActionProgram.at(
            rng.getUnsignedInt64(0, this->preExistingActionProgram.size() - 1));


        // Target name individual is different from representation
        if(targetIndividual.get().getRepresentationID() != this->actionProgramRepresentationID){
            // Try to find an aggregated population with this name (will throw if not found)
            Population& actionProgramPopulation = population.getSubPopulation(this->actionProgramRepresentationID);
            const Population& aggregatedPopulation = actionProgramPopulation.getAggregatedPopulation(targetIndividual.get().getRepresentationID());

            // Dupplication to exchange the individual from aggregatedPopulation to actionProgramPopulation.
            const Representation::Individual& newTargetIndividual = actionProgramPopulation.copyIndividual(targetIndividual, graph);
            targetIndividual = newTargetIndividual;
        }

        // Set corresponding individual to the new team
        graph.setVertexProgram(target, targetIndividual);

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
    std::vector<std::reference_wrapper<const Individual>>& newSubIndividuals,
    const RepresentationParameters& params, RNG::RNG& rng)
{
    if(edge.getDestination().hasProgram() && edge.getDestination().getProgram().getRepresentationID() == this->actionProgramRepresentationID &&
       rng.getDouble(0.0, 1.0) < params.atpg.probaContextOverActionProgram){
       
        // copy program
        const Representation::Individual& newIndividual = population.getSubPopulation(edge.getDestination().getProgram().getRepresentationID()).copyIndividual(edge.getDestination().getProgram(), graph);

        // Clone vertex destination
        const EvoGraph::Vertex& newDestination = graph.cloneVertex(edge.getDestination());

        // Set the new destination
        graph.setEdgeDestination(edge, newDestination);

        // Set the mutated individual to the edge
        graph.setVertexProgram(newDestination, newIndividual);

        // Add it to the list of new individual to be mutated.
        newSubIndividuals.push_back(newIndividual);

    } else {
        const Individual& originIndividual = edge.getProgram();
        // copy program
        const Representation::Individual& newIndividual = population.getSubPopulation(originIndividual.getRepresentationID()).copyIndividual(originIndividual, graph);

        // Set the mutated individual to the edge
        graph.setEdgeProgram(edge, newIndividual);

        // Add it to the list of new individual to be mutated.
        newSubIndividuals.push_back(newIndividual);

        // Edge target modification
        // As it Stephen kelly's work, Edge target modification is conditionned
        // to the modification of the prealable Edge.Program behavior.
        if (rng.getDouble(0.0, 1.0) < params.tpg.pEdgeDestinationChange) {
            mutateEdgeDestination(graph, edge, population, newSubIndividuals, params, rng);
        }
    }

    
}

void Representation::ATPG::ATPGMutator::mutateSubIndividuals(
            std::vector<std::reference_wrapper<const Individual>>& individuals, EvoGraph::Graph& graph, 
            Population& population, const RepresentationParameters& params, 
            RNG::RNG& rng, uint64_t maxNbThreads)
{
    // Devide individuals into program individuals and action program individuals
    std::vector<std::reference_wrapper<const Individual>> programIndividuals;
    std::vector<std::reference_wrapper<const Individual>> actionProgramIndividuals;
    for(const Representation::Individual& individual : individuals){
        if(individual.getRepresentationID() == this->actionProgramRepresentationID){
            actionProgramIndividuals.push_back(individual);
        } else {
            programIndividuals.push_back(individual);
        }
    }

    // Mutate action program using the archive of TPG
    TPG::TPGMutator::mutateSubIndividuals(programIndividuals, graph, population, params, rng, maxNbThreads);

    
    // mutate action programs
    Mutator& actionProgramMutator = this->getSubMutator(this->actionProgramRepresentationID);
    Population& actionProgramPopulation = population.getSubPopulation(this->actionProgramRepresentationID);
    std::vector<std::reference_wrapper<const Individual>> newSubIndividuals;
    for(const Representation::Individual& individual : actionProgramIndividuals){
        actionProgramMutator.mutateIndividual(
            individual, graph, actionProgramPopulation, newSubIndividuals, params, rng);
    }
    // Mutate the new subIndividuals with actionProgramMutator.
    actionProgramMutator.mutateSubIndividuals(
        newSubIndividuals, graph, actionProgramPopulation, params, rng, maxNbThreads);
}
