#include <array>
#include <algorithm>

#include "representation/maple/mapleMutator.h"

bool Representation::Maple::MapleMutator::isConfigurationValid(const RepresentationParameters& params, const Output::OutputHandler& outputs) const
{
    size_t nbOutputs = (outputs.sizeDiscrete() == 1) ? outputs.front().getNbValues() : outputs.size();
    if(outputs.sizeContinuous() != 0 && outputs.sizeDiscrete() != 0){
        throw std::runtime_error("MapleMutator::initRandomPopulation: Maple does not support mixed discrete and continuous outputs.");
    } else if (outputs.sizeDiscrete() != 0 || outputs.sizeContinuous() != 0){
        if(params.maple.nbActionEdgeInit > nbOutputs){
            throw std::runtime_error("MapleMutator::initRandomPopulation: Number of discrete outputs cannot be lower than the number of initial edges.");
        }        
    } else if (outputs.size() == 0){
        throw std::runtime_error("TPGMutator::initRandomPopulation: No outputs defined.");
    }
    return true;
}

void Representation::Maple::MapleMutator::initRandomPopulation(EvoGraph::Graph& graph, Population& population, const RepresentationParameters& params, RNG::RNG& rng)
{
    this->isConfigurationValid(params, population.getOutputs());
    size_t nbOutputs = (population.getOutputs().sizeDiscrete() == 1) ? population.getOutputs().front().getNbValues() : population.getOutputs().size();
    this->initActionVertices(graph, nbOutputs);

    // Empty individual population
    population.clearIndividuals(graph);

    for (size_t idx = 0; idx < params.nbIndividuals; idx++) {
        this->initRandomIndividual(graph, population, params, rng);
    }
}

void Representation::Maple::MapleMutator::initRandomSpecificIndividual(const Individual& individual, EvoGraph::Graph& graph, Population& population, const RepresentationParameters& params, RNG::RNG& rng)
{
    // First individual initialized, check configuration validity and create action vertices
    if(population.getIndividuals().size() == 1){
        this->isConfigurationValid(params, population.getOutputs());
        size_t nbOutputs = (population.getOutputs().sizeDiscrete() == 1) ? population.getOutputs().front().getNbValues() : population.getOutputs().size();
        this->initActionVertices(graph, nbOutputs);
    }

    population.emptyIndividual(individual, graph);
    
    const EvoGraph::Vertex& vertex = dynamic_cast<const MapleIndividual&>(individual).getVertex();
    const EvoGraph::Team& team = dynamic_cast<const EvoGraph::Team&>(vertex);

    // Get program mutator and population
    Mutator& programMutator = this->getSubMutator(this->programRepresentationID);
    Population& programPopulation = population.getSubPopulation(this->programRepresentationID);

    // Get available actions classes
    
    size_t nbOutputs = (population.getOutputs().sizeDiscrete() == 1) ? population.getOutputs().front().getNbValues() : population.getOutputs().size();
    std::vector<uint64_t> availableActions(nbOutputs);
    std::iota(availableActions.begin(), availableActions.end(), uint64_t{0});


    // Get the actions vertices.
    auto actionVertices = graph.getActions();

    size_t remaining = availableActions.size();
    for (size_t idxAction = 0; idxAction < params.maple.nbActionEdgeInit; idxAction++) {
        const Individual& programIndividual = programMutator.initRandomIndividual(graph, programPopulation, params, rng);

        // Pick uniformly from remaining values
        size_t pickIdx = rng.getUnsignedInt64(0, remaining - 1);
        uint64_t actionClass = availableActions[pickIdx];

        // Remove picked element (swap with last)
        // By swapping, the order of availableActions is changed.
        std::swap(availableActions[pickIdx], availableActions[remaining - 1]);
        --remaining;

        // Create the action edge
        graph.addNewEdge(team, actionVertices.at(actionClass), programIndividual);
    }

    graph.orderActionEdges(team);
}

void Representation::Maple::MapleMutator::crossoverPrograms(
    std::array<std::reference_wrapper<const EvoGraph::Team>, 2> teams, uint64_t indexCross, EvoGraph::Graph& graph, Population& population, std::vector<std::reference_wrapper<const Individual>>& newSubIndividuals, const RepresentationParameters& params, RNG::RNG& rng)
{
    // Get program to cross for the teams. It should exist.
    std::vector<std::reference_wrapper<const Individual>> swapPrograms;
    for (size_t i = 0; i < teams.size(); ++i) {
        const EvoGraph::Team& team = teams[i];
        for (const EvoGraph::Edge& edge : team.getOutgoingEdges()) {
            if (auto action = dynamic_cast<const EvoGraph::Action*>(
                    &edge.getDestination())) {
                if (action->getActionID() == indexCross &&
                    edge.getProgram().getRepresentationID() ==
                        this->programRepresentationID) {

                    const Individual& originIndividual = edge.getProgram();
                    // copy program
                    const Representation::Individual& newIndividual = population.getSubPopulation(originIndividual.getRepresentationID()).copyIndividual(originIndividual, graph);

                    // Set the mutated individual to the edge
                    graph.setEdgeProgram(edge, newIndividual);

                    swapPrograms.push_back(edge.getProgram());
                    break;
                }
            }
        }
    }
    if(swapPrograms.size() != 2){
        throw std::runtime_error("MapleMutator::crossoverPrograms Program not found, while it should exist");
    }


    std::array<std::reference_wrapper<const Individual>, 2> programs{swapPrograms[0], swapPrograms[1]};
    // Do the crossover
    Population& programPopulation = population.getSubPopulation(this->programRepresentationID);
    this->getSubMutator(swapPrograms.front().get().getRepresentationID()).crossoverIndividuals(programs, graph, programPopulation, newSubIndividuals, params, rng);
}

void Representation::Maple::MapleMutator::crossoverEdges(
    std::array<std::reference_wrapper<const EvoGraph::Team>, 2> teams, uint64_t indexCross, EvoGraph::Graph& graph, Population& population, std::vector<std::reference_wrapper<const Individual>>& newSubIndividuals, const RepresentationParameters& params, RNG::RNG& rng)
{
    // Get edge to swap for the teams if it exist
    std::array<const EvoGraph::Edge*, 2> swapEdges{nullptr, nullptr};
    for (size_t i = 0; i < teams.size(); ++i) {
        const EvoGraph::Team& team = teams[i];
        if (team.getAssessedActions().count(indexCross)) {
            for (const EvoGraph::Edge& edge : team.getOutgoingEdges()) {
                if (auto action = dynamic_cast<const EvoGraph::Action*>(&edge.getDestination())) {
                    if (action->getActionID() == indexCross) {
                        swapEdges[i] = (&edge);
                    }
                }
            }
        }
    }

    // If both edges exist, switch the programs
    if(swapEdges.at(0) != nullptr && swapEdges.at(1) != nullptr) {
        const Individual& program0 = swapEdges.at(0)->getProgram();
        const Individual& program1 = swapEdges.at(1)->getProgram();
        graph.setEdgeProgram(*swapEdges.at(0), program1);
        graph.setEdgeProgram(*swapEdges.at(1), program0);

    // Else change the source of the edge.
    } else if (swapEdges.at(0) != nullptr) {
        graph.setEdgeSource(*swapEdges.at(0), teams.at(1));
    } else if (swapEdges.at(1) != nullptr) {
        graph.setEdgeSource(*swapEdges.at(1), teams.at(0));
    }
}


void Representation::Maple::MapleMutator::crossoverIndividuals(
    std::array<std::reference_wrapper<const Individual>, 2> individuals, EvoGraph::Graph& graph, Population& population, std::vector<std::reference_wrapper<const Individual>>& newSubIndividuals, const RepresentationParameters& params, RNG::RNG& rng)
{
    // No crossover
    if (params.maple.pCrossIndividuals == 0) {
        return;
    }
    // Initialize available actions
    size_t nbOutputs = (population.getOutputs().sizeDiscrete() == 1) ? population.getOutputs().front().getNbValues() : population.getOutputs().size();
    std::vector<uint64_t> availableActions(nbOutputs);
    std::iota(availableActions.begin(), availableActions.end(), uint64_t{0});

    uint64_t indexAction;

    
    const EvoGraph::Vertex& vertex0 = dynamic_cast<const MapleIndividual&>(individuals.at(0).get()).getVertex();
    const EvoGraph::Team& team0 = dynamic_cast<const EvoGraph::Team&>(vertex0);

    const EvoGraph::Vertex& vertex1 = dynamic_cast<const MapleIndividual&>(individuals.at(1).get()).getVertex();
    const EvoGraph::Team& team1 = dynamic_cast<const EvoGraph::Team&>(vertex1);

    std::array<std::reference_wrapper<const EvoGraph::Team>, 2> teamsArray = {team0, team1};

    // Always do at least one crossover
    // (mearning we don't want any crossover)
    double proba = 1;
    size_t remaining = availableActions.size();
    while (remaining > 0 &&
           proba > rng.getDouble(0.0, 1.0)) {

        // Pick uniformly from remaining values
        size_t pickIdx = rng.getUnsignedInt64(0, remaining - 1);
        indexAction = availableActions[pickIdx];

        // Remove picked element (swap with last)
        std::swap(availableActions[pickIdx], availableActions[remaining - 1]);
        --remaining;

        // A crossover at program level can be done only the both parents
        // assessed the action concerned
        if (team0.getAssessedActions().count(indexAction) > 0 &&
            team1.getAssessedActions().count(indexAction) > 0 &&
            params.maple.pCrossPrograms > rng.getDouble(0, 1)) {

            this->crossoverPrograms(teamsArray, indexAction, graph, population, newSubIndividuals, params, rng);
        }
        else {
            this->crossoverEdges(teamsArray, indexAction, graph, population, newSubIndividuals, params, rng);

        }

        graph.updateAssessedActions(team0);
        graph.updateAssessedActions(team1);
        
        graph.orderActionEdges(team0);
        graph.orderActionEdges(team1);
        proba *= params.maple.pCrossIndividuals;
    }

}

void Representation::Maple::MapleMutator::addRandomEdge(
    EvoGraph::Graph& graph, const EvoGraph::Team& team,
    RNG::RNG& rng)
{
    // Pick an edge (excluding ones from the team, edges with the team as a
    // destination and the edges that are action edges)
    auto pickableEdges(this->preExistingEdges);
    // cf erase-remove idiom
    pickableEdges.erase(
        std::remove_if(pickableEdges.begin(), pickableEdges.end(),
                       [&team](const EvoGraph::Edge& edge) -> bool {
                            // Rewritte this condition but in a friendly way for the reader
                            if (&edge == nullptr || edge.getSource() == team) {
                                return true;
                            } else if (auto action = dynamic_cast<const EvoGraph::Action*>(&edge.getDestination())) {
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
    const EvoGraph::Edge& pickedEdge = *iter;

    // Create new edge from action and with the same ProgramSharedPointer
    // But with the action as its source
    // throw std::runtime_error if the edge is not from the graph;
    graph.setEdgeSource(graph.cloneEdge(pickedEdge), team);
}

void Representation::Maple::MapleMutator::swapEdges(EvoGraph::Graph& graph,
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
    const EvoGraph::Vertex& actionVertex1 = (*it1).get().getDestination();
    const EvoGraph::Vertex& actionVertex2 = (*it2).get().getDestination();

    // Set the swapped action classes
    graph.setEdgeDestination((*it1), actionVertex2);
    graph.setEdgeDestination((*it2), actionVertex1);
}


void Representation::Maple::MapleMutator::mutateEdgeDestination(
    EvoGraph::Graph& graph, const EvoGraph::Edge& edge,
    const std::set<size_t>& actionClasses,
    const RepresentationParameters& params, RNG::RNG& rng)
{
    std::vector<std::reference_wrapper<const EvoGraph::Action>> actionVertices(graph.getActions());
    actionVertices.erase(
        std::remove_if(
            actionVertices.begin(), actionVertices.end(),
            [&actionClasses](const std::reference_wrapper<const EvoGraph::Action>& ref) {
                const EvoGraph::Action& actionVertex = ref.get();
                return actionClasses.find(actionVertex.getActionID()) != actionClasses.end();
            }
        ),
        actionVertices.end()
    );

    auto newAction = actionVertices[rng.getUnsignedInt64(0, actionVertices.size() - 1)];
    graph.setEdgeDestination(edge, newAction);
}

void Representation::Maple::MapleMutator::mutateOutgoingEdge(
    EvoGraph::Graph& graph, const EvoGraph::Edge& edge,
    const std::set<size_t>& actionClasses, Population& population,
    std::vector<std::reference_wrapper<const Individual>>& newSubIndividuals,
    const RepresentationParameters& params, RNG::RNG& rng)
{
    const Individual& originIndividual = edge.getProgram();
    // copy program
    const Representation::Individual& newIndividual = population.getSubPopulation(originIndividual.getRepresentationID()).copyIndividual(originIndividual, graph);

    // Set the mutated individual to the edge
    graph.setEdgeProgram(edge, newIndividual);

    // Add it to the list of new individual to be mutated.
    newSubIndividuals.push_back(newIndividual);

    size_t nbActions = (population.getOutputs().sizeDiscrete() == 1) ? population.getOutputs().front().getNbValues() : population.getOutputs().size();
    // Change action ID randomly if the action do not contain all actions.
    if (actionClasses.size() < nbActions &&
        params.maple.pChangeActionClass > rng.getDouble(0.0, 1.0)) {
        
        this->mutateEdgeDestination(graph, edge, actionClasses, params, rng);
    }
}

void Representation::Maple::MapleMutator::mutateIndividual(
    const Individual& individual, EvoGraph::Graph& graph, Population& population, std::vector<std::reference_wrapper<const Individual>>& newSubIndividuals, const RepresentationParameters& params, RNG::RNG& rng)
{
    const EvoGraph::Vertex& vertex = dynamic_cast<const MapleIndividual&>(individual).getVertex();
    const EvoGraph::Team& team = dynamic_cast<const EvoGraph::Team&>(vertex);

    // 1. Remove randomly selected edges
    // Keep at least two edges (otherwise the team is useless)
    double proba = params.maple.pActionEdgeDeletion;
    while (team.getOutgoingEdges().size() > 1 &&
            proba > rng.getDouble(0.0, 1.0)) {
        this->removeRandomEdge(graph, team, rng);

        // Decrement the proba of removing another edge
        proba *= params.maple.pActionEdgeDeletion;

        // Update assessed actions
        graph.updateAssessedActions(team);
    }

    // 2. Add random duplicated edge with the team as its source
    proba = params.maple.pActionEdgeAddition;
    size_t nbOutputs = (population.getOutputs().sizeDiscrete() == 1) ? population.getOutputs().front().getNbValues() : population.getOutputs().size();
    while (team.getOutgoingEdges().size() <
                nbOutputs &&
            proba > rng.getDouble(0.0, 1.0)) {
        // Add an edge (by duplication of an existing one)
        this->addRandomEdge(graph, team, rng);

        // Decrement the proba of adding another edge
        proba *= params.maple.pActionEdgeAddition;

        // Update assessed actions
        graph.updateAssessedActions(team);

    }

    // 3. swap randomly selected edges
    // With at least two edges
    proba = params.maple.pSwapActionProgram;
    while (team.getOutgoingEdges().size() > 2 &&
            proba > rng.getDouble(0.0, 1.0)) {
        this->swapEdges(graph, team, rng);

        // Decrement the proba of swapping two edges
        proba *= params.maple.pSwapActionProgram;
    }

    bool anyMutationDone = false;
    do {
        
        // Get available actions classes
        std::vector<uint64_t> availableEdges(team.getOutgoingEdges().size());
        std::iota(availableEdges.begin(), availableEdges.end(), uint64_t{0});
        size_t remaining = availableEdges.size();

        // 4. mutate randomly selected program on action Edge.
        double proba = params.maple.pMutateActionProgram;
        while (remaining > 0 &&
               proba > rng.getDouble(0.0, 1.0)) {

            // Pick uniformly from remaining values
            size_t pickIdx = rng.getUnsignedInt64(0, remaining - 1);

            // Remove picked element (swap with last)
            // By swapping, the order of availableActions is changed.
            std::swap(availableEdges[pickIdx], availableEdges[remaining - 1]);
            --remaining;

            auto iter = team.getOutgoingEdges().begin();
            std::advance(iter, pickIdx);

            this->mutateOutgoingEdge(graph, *iter, team.getAssessedActions(), population, newSubIndividuals, params, rng);
            graph.updateAssessedActions(team);

            proba *= params.maple.pMutateActionProgram;
            anyMutationDone = true;
        }
    } while (!anyMutationDone && params.maple.pMutateActionProgram != 0.0);
    graph.orderActionEdges(team);
}

