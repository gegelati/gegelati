

#include "algorithm/species/speciesMutator.h"


void Algorithm::Species::SpeciesMutator::setArchive(const Archive& archive)
{
    this->archive = archive;
}

bool Algorithm::Species::SpeciesMutator::isConfigurationValid(const Learn::LearningParameters& params, const Output::OutputHandler& outputs) const
{
    return true;
}

const EvoGraph::Team& Algorithm::Species::SpeciesMutator::initSpeciesGraphStructure(EvoGraph::Graph& graph, AgentManager& manager, const Learn::LearningParameters& params, RNG::RNG& rng)
{
    const auto& outputs = manager.getOutputs();
    this->isConfigurationValid(params, outputs);

    // Number of action vertices needed, created the action vertices
    size_t nbActionVertices = (outputs.sizeDiscrete() == 0) ? outputs.sizeContinuous() : outputs.front().getNbValues();
    std::vector<std::reference_wrapper<const EvoGraph::Action>> actions(this->initActionVertices(graph, nbActionVertices));


    // Connexion
    const EvoGraph::Team& team = graph.addNewTeam();
    const EvoGraph::Action& action = actions.at(rng.getUnsignedInt64(0, nbActionVertices - 1));
    graph.addNewEdge(team, action);
    graph.updateAllAssessedActions();
    return team;
}


bool Algorithm::Species::SpeciesMutator::addContextEdgeSpecies(const EvoGraph::Vertex& newRoot, AgentManager& manager, EvoGraph::Graph& graph, RNG::RNG& rng, std::map<std::reference_wrapper<const EvoGraph::Edge>, std::reference_wrapper<const EvoGraph::Edge>>& edgeMap)
{
    std::set<std::reference_wrapper<const EvoGraph::Team>> contextTeams(dynamic_cast<SpeciesManager&>(manager).getContextTeams());
    if(contextTeams.size() == 0) {
        return false;
    }

    auto it = contextTeams.begin();
    std::advance(it, rng.getUnsignedInt64(0, contextTeams.size() - 1));

    auto itEdges = it->get().getOutgoingEdges().begin();
    std::advance(it, rng.getUnsignedInt64(0, it->get().getOutgoingEdges().size() - 1));


    const EvoGraph::Edge* copyEdge;
    for(const auto& pair: edgeMap) {
        if(pair.second == *itEdges) {
            copyEdge = &pair.first.get();
        }
    }

    const EvoGraph::Edge& newEdge = graph.cloneEdge(*copyEdge);

    // Add the new edge at in the edgeMap, link to the same old edge
    // By doing so, the same program will be dupplicating for the copyEdge and newEdge, leading to small initial changes.
    edgeMap.insert({newEdge, *itEdges});
    return true;
}

bool Algorithm::Species::SpeciesMutator::addActivationEdgeSpecies(const EvoGraph::Vertex& newRoot, AgentManager& manager, EvoGraph::Graph& graph, RNG::RNG& rng, std::map<std::reference_wrapper<const EvoGraph::Edge>, std::reference_wrapper<const EvoGraph::Edge>>& edgeMap)
{    
    std::vector<std::reference_wrapper<const EvoGraph::Action>> availableActions(graph.getActions());
    std::set<std::reference_wrapper<const EvoGraph::Team>> activationTeams(dynamic_cast<SpeciesManager&>(manager).getActivationTeams());

    // Get all the vertex with available actions, take a vector for keeping track of the order
    std::vector<std::pair<std::reference_wrapper<const EvoGraph::Team>, std::set<uint64_t>>> availableActionsAllVertices;
    for(const EvoGraph::Team& vertex: activationTeams){
    
        std::set<uint64_t> availableActionOfVertex;
        const EvoGraph::Team& currentVertex = vertex;
        std::set<uint64_t> currentVertexAssessedActions = currentVertex.getAssessedActions();

        if(vertex.getIncomingEdges().size() == 0) {
            
            for(const EvoGraph::Action& action: availableActions){
                if(currentVertexAssessedActions.find(action.getActionID()) == currentVertexAssessedActions.end()) {
                    availableActionOfVertex.insert(action.getActionID());
                }
            }
            
        } else {
            // Get the assessed actions of all the vertex pointing to this vertex.
            for(const EvoGraph::Edge& edge: currentVertex.getIncomingEdges()) {
                const EvoGraph::Vertex& srcVertex = edge.getSource();
                std::set<uint64_t> srcAssessedActions = srcVertex.getAssessedActions();
                availableActionOfVertex.insert(srcAssessedActions.begin(), srcAssessedActions.end());
            }
        }

        

        // Add the set difference in the available action set.
        std::set<uint64_t> difference;
        std::set_difference(
            availableActionOfVertex.begin(), availableActionOfVertex.end(),
            currentVertexAssessedActions.begin(), currentVertexAssessedActions.end(),
            std::inserter(difference, difference.begin())
        );

        if(difference.size() > 0){
            availableActionsAllVertices.push_back({vertex, difference});
        }
    }

    if(availableActionsAllVertices.size() == 0){
        return false;
    }

    // Choose randomly an available team.
    const auto& pairPicked = availableActionsAllVertices.at(rng.getUnsignedInt64(0, availableActionsAllVertices.size() - 1));
    const auto& set = pairPicked.second;

    availableActions.erase(
        std::remove_if(
            availableActions.begin(),
            availableActions.end(),
            [&set](const EvoGraph::Action& action) {
                return set.find(action.getActionID()) == set.end();
            }
        ),
        availableActions.end()
    );

    // Weird way of getting equivalent of this vertex in the copy, but working.
    // Get the first outgoing edge of the search vertex. 
    // Find it's copy in the edge map
    // Get its source.
    const EvoGraph::Vertex* copyVertex;
    for(const auto& pair: edgeMap) {
        if(pair.second == pairPicked.first.get().getOutgoingEdges().front()) {
            copyVertex = &pair.first.get().getSource();
        }
    }

    graph.addNewEdge(*copyVertex, availableActions.at(rng.getUnsignedInt64(0, availableActions.size() - 1)));
    return true;
}



bool Algorithm::Species::SpeciesMutator::removeEdgeSpecies(const EvoGraph::Vertex& newRoot, AgentManager& manager, EvoGraph::Graph& graph, RNG::RNG& rng, std::map<std::reference_wrapper<const EvoGraph::Edge>, std::reference_wrapper<const EvoGraph::Edge>>& edgeMap)
{
    SpeciesManager& speciesManager = dynamic_cast<SpeciesManager&>(manager);
    auto edgeMapCopy(edgeMap);
    const std::set<std::reference_wrapper<const EvoGraph::Team>>& activationTeams(speciesManager.getActivationTeams());

    // Build a new set with only the edges that should NOT be removed
    std::set<std::reference_wrapper<const EvoGraph::Edge>> edges;
    for (const auto& pair : edgeMapCopy) {
        bool isValid;
        if (activationTeams.find((const EvoGraph::Team&)(pair.second.get().getSource())) == activationTeams.end()) {
            isValid = pair.second.get().getSource().getOutgoingEdges().size() >= 3;
        } else {
            isValid = pair.second.get().getSource().getOutgoingEdges().size() >= 2;
        }
        if (isValid) {
            edges.insert(pair.first);
        }
    }

    if(edges.size() == 0) {
        return false;
    }

    // Delete an action edge
    size_t pickedEdgeID = rng.getUnsignedInt64(0, edges.size() - 1);
    auto it = edges.begin();
    std::advance(it, pickedEdgeID);

    std::set<std::reference_wrapper<const EvoGraph::Edge>> removedEdges{*it};
    std::set<std::reference_wrapper<const EvoGraph::Vertex>> removedVertices;
    while(removedEdges.size() > 0) {
        const EvoGraph::Edge& removeEdge = *removedEdges.begin();
        removedEdges.erase(removedEdges.begin());

        // This vertex is gonna be removed too.
        const EvoGraph::Vertex& destination = removeEdge.getDestination();
        if(destination.getIncomingEdges().size() == 1 && dynamic_cast<const EvoGraph::Action*>(&destination) != nullptr) {
            const auto& destRemovedEdges = removeEdge.getDestination().getOutgoingEdges();
            removedEdges.insert(destRemovedEdges.begin(), destRemovedEdges.end());
            removedVertices.insert(destination);
        }

        // Remove the edge from the edge map
        edgeMap.erase(removeEdge);

        // Remove the edge from the graph
        graph.removeEdge(removeEdge);
    }

    for(const EvoGraph::Vertex& removedVertex: removedVertices) {
        graph.removeVertex(removedVertex);
    }
    return true;
}

bool Algorithm::Species::SpeciesMutator::extendSpecies(const EvoGraph::Vertex& newRoot, AgentManager& manager, EvoGraph::Graph& graph, RNG::RNG& rng, std::map<std::reference_wrapper<const EvoGraph::Edge>, std::reference_wrapper<const EvoGraph::Edge>>& edgeMap)
{
    // Get the activation vertex
    const auto& activationVertices = dynamic_cast<SpeciesManager&>(manager).getActivationTeams();

    // Do a weighted list of the vertices, to push the mutation to select a team with a lot of edges leading to actions.
    std::vector<std::reference_wrapper<const EvoGraph::Team>> weightedVertices;
    for(const EvoGraph::Team& team: activationVertices) {
        size_t nbActionEdge = 0;
        for(const EvoGraph::Edge& edge: team.getOutgoingEdges()) {
            if(auto action = dynamic_cast<const EvoGraph::Action*>(&edge.getDestination())) {
                nbActionEdge++;
            }
        }
        weightedVertices.insert(weightedVertices.end(), std::pow(nbActionEdge, 2), team);
    }

    // Randomly select a vertex
    const EvoGraph::Team& choosenVertex = weightedVertices.at(rng.getUnsignedInt64(0, weightedVertices.size() - 1));


    // Weird way of getting equivalent of this vertex in the copy, but working.
    // Get the first outgoing edge of the search vertex. 
    // Find it's copy in the edge map
    // Get its source.
    const EvoGraph::Vertex* copyVertexChoose;
    for(const auto& pair: edgeMap) {
        if(pair.second == choosenVertex.getOutgoingEdges().front()) {
            copyVertexChoose = &pair.first.get().getSource();
        }
    }

    // Create the new vertices
    const EvoGraph::Team& decisionTeam = graph.addNewTeam();
    const EvoGraph::Team& newActivationTeam0 = graph.addNewTeam();
    const EvoGraph::Team& newActivationTeam1 = graph.addNewTeam();

    // Add the new edges
    graph.addNewEdge(*copyVertexChoose, decisionTeam);
    graph.addNewEdge(decisionTeam, newActivationTeam0);
    graph.addNewEdge(decisionTeam, newActivationTeam1);

    std::set<std::reference_wrapper<const EvoGraph::Edge>> actionEdges;
    for(const EvoGraph::Edge& edge: copyVertexChoose->getOutgoingEdges()) {
        if(auto action = dynamic_cast<const EvoGraph::Action*>(&edge.getDestination())) {
            actionEdges.insert(edge);
        }
    }

    double probaExtensionEdge = 0.8;
    double proba = 1;
    while(actionEdges.size() > 0 && proba > rng.getDouble(0, 1)){
        
        // Get a random edge.
        auto itEdges = actionEdges.begin();
        std::advance(itEdges, rng.getUnsignedInt64(0, actionEdges.size()-1));
        
        // add the new edge, copiing the destination of the random choosen edge
        const EvoGraph::Edge& newEdge0 = graph.addNewEdge(newActivationTeam0, itEdges->get().getDestination());
        const EvoGraph::Edge& newEdge1 = graph.addNewEdge(newActivationTeam1, itEdges->get().getDestination());
        
        // Add the new edges in the edge map, link to the copied edge, so that the new programs will be dupplicated on this edge.
        edgeMap.insert({newEdge0, edgeMap.at(*itEdges)});
        edgeMap.insert({newEdge1, edgeMap.at(*itEdges)});

        edgeMap.erase(*itEdges);

        // Remove the edge from the graph
        graph.removeEdge(*itEdges);

        // Erase the edge and increase probability
        actionEdges.erase(itEdges);
        proba *= probaExtensionEdge;
    }

    return true;
}

const EvoGraph::Vertex& Algorithm::Species::SpeciesMutator::mutateSpeciesGraph(EvoGraph::Graph& graph, AgentManager& manager, const Learn::LearningParameters& params, RNG::RNG& rng, std::map<std::reference_wrapper<const EvoGraph::Edge>, std::reference_wrapper<const EvoGraph::Edge>>& edgeMap)
{
    double probaAddContext = params.mutation.tpg.pProgramMutation;
    double probaAddActivation = params.mutation.tpg.pEdgeDestinationIsAction;
    double probaDelete = params.mutation.tpg.probaContextOverActionProgram;
    double probaExtension = params.mutation.tpg.ratioTeamsOverActions;

    const EvoGraph::Vertex& newRoot = this->copyGraphSpecies(manager, graph, edgeMap);
    bool mutationHappened = false;
    do {
        double randomValue = rng.getDouble(0, probaAddActivation + probaAddContext + probaDelete + probaExtension);
        if(randomValue > probaAddActivation + probaDelete + probaExtension) {
            std::cout<<"  ADDCONTEXT   ";
            mutationHappened = this->addContextEdgeSpecies(newRoot, manager, graph, rng, edgeMap);
        } else if (randomValue > probaDelete + probaExtension){
            std::cout<<"  ADDACTIVATION    ";
            mutationHappened = this->addActivationEdgeSpecies(newRoot, manager, graph, rng, edgeMap);
        } else if (randomValue > probaExtension){
            std::cout<<"  DEL    ";
            mutationHappened = this->removeEdgeSpecies(newRoot, manager, graph, rng, edgeMap);
        }  else {
            std::cout<<"  EXTEND      ";
            mutationHappened = this->extendSpecies(newRoot, manager, graph, rng, edgeMap);
        }
    } while (!mutationHappened);

    graph.updateAllAssessedActions();
    return newRoot;
}

const EvoGraph::Vertex& Algorithm::Species::SpeciesMutator::copyGraphSpecies(AgentManager& manager, EvoGraph::Graph& graph, std::map<std::reference_wrapper<const EvoGraph::Edge>, std::reference_wrapper<const EvoGraph::Edge>>& edgeMap)
{
    std::map<std::reference_wrapper<const EvoGraph::Vertex>, std::reference_wrapper<const EvoGraph::Vertex>> vertexMap;
    edgeMap.clear();

    const auto& originTeams = dynamic_cast<SpeciesManager&>(manager).getTeams();
    const auto& originActions = dynamic_cast<SpeciesManager&>(manager).getActions();

    for(const EvoGraph::Team& team: originTeams) {
        vertexMap.insert({team, graph.addNewTeam()});
    }
    for(const EvoGraph::Action& action: originActions) {
        vertexMap.insert({action, action});
    }

    for(const auto& pair: vertexMap) {
        for(const EvoGraph::Edge& edge: pair.first.get().getOutgoingEdges()) {
            edgeMap.insert({graph.addNewEdge(vertexMap.at(pair.first), vertexMap.at(edge.getDestination())), edge});
        }
    }

    graph.updateAllAssessedActions();
    return vertexMap.at(dynamic_cast<SpeciesManager&>(manager).getRootVertex()).get();
}

void Algorithm::Species::SpeciesMutator::initAgentFromSpecies(const Agent& agent, EvoGraph::Graph& graph, AgentManager& manager, const Learn::LearningParameters& params, RNG::RNG& rng,std::map<std::reference_wrapper<const EvoGraph::Edge>, std::reference_wrapper<const EvoGraph::Edge>>& edgeMap)
{
    const SpeciesAgent& oldSpeciesAgent = dynamic_cast<const SpeciesAgent&>(agent);

    SpeciesManager& speciesManager = dynamic_cast<SpeciesManager&>(manager);
    const Agent& newAgent = speciesManager.createAgent(graph);

    // Get program mutator and manager
    Mutator& programMutator = this->getSubMutator(this->programAlgorithmID);
    AgentManager& programManager = manager.getSubManager(this->programAlgorithmID);

    for(const EvoGraph::Edge& edge: speciesManager.getEdges()) {
        

        // Edge is not found in the list of edges, meaning it is a new edge.
        // Instantiate a new program
        if(edgeMap.find(edge) == edgeMap.end()) {
            const Agent& programAgent = programMutator.initRandomAgent(graph, programManager, params, rng);
            speciesManager.setProgram(newAgent, edge, programAgent);
        } else {
            // Getting the program should not fail.
            const Agent& programAgent = programManager.copyAgent(oldSpeciesAgent.getProgram(edgeMap.at(edge)), graph);
            speciesManager.setProgram(newAgent, edge, programAgent);
        }
    }
}

void Algorithm::Species::SpeciesMutator::initRandomPopulation(EvoGraph::Graph& graph, AgentManager& manager, const Learn::LearningParameters& params, RNG::RNG& rng)
{
    const auto& outputs = manager.getOutputs();
    this->isConfigurationValid(params, outputs);
    this->initActionVertices(graph, manager.getOutputs().size());
    
    // Empty agent manager
    manager.clearAgents(graph);
    
    for (size_t idx = 0; idx < manager.getExpectedNbAgents(); idx++) {
        const Agent& agent = this->initRandomAgent(graph, manager, params, rng);
        if(!agent.isValid()) {
            throw std::runtime_error("SpeciesMutator::initRandomPopulation: agent should be valid after initialization");
        }
    }
}

void Algorithm::Species::SpeciesMutator::initRandomSpecificAgent(const Agent& agent, EvoGraph::Graph& graph, AgentManager& manager, const Learn::LearningParameters& params, RNG::RNG& rng)
{
    // First agent is initialized, check validity of the configuration.
    if(manager.getAgents().size() == 1){
        this->isConfigurationValid(params, manager.getOutputs());
        this->initActionVertices(graph, manager.getOutputs().size());
    }

    manager.emptyAgent(agent, graph);

    // Get program mutator and manager
    Mutator& programMutator = this->getSubMutator(this->programAlgorithmID);
    AgentManager& programManager = manager.getSubManager(this->programAlgorithmID);

    SpeciesManager& speciesManager = dynamic_cast<SpeciesManager&>(manager);
    for(const EvoGraph::Edge& edge: speciesManager.getEdges()) {
        
        // Initialize a program
        const Agent& programAgent = programMutator.initRandomAgent(graph, programManager, params, rng);

        speciesManager.setProgram(agent, edge, programAgent);
    }
}


void Algorithm::Species::SpeciesMutator::crossoverPrograms(
    std::array<std::reference_wrapper<const Agent>, 2> agents, const EvoGraph::Edge& edge, EvoGraph::Graph& graph, AgentManager& manager, std::vector<std::reference_wrapper<const Agent>>& newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng)
{
    const Agent& program0 = dynamic_cast<const SpeciesAgent&>(agents.at(0).get()).getProgram(edge);
    const Agent& program1 = dynamic_cast<const SpeciesAgent&>(agents.at(1).get()).getProgram(edge);

    // copy programs
    const Algorithm::Agent& newProgram0 = manager.getSubManager(program0.getAlgorithmID()).copyAgent(program0, graph);
    const Algorithm::Agent& newProgram1 = manager.getSubManager(program1.getAlgorithmID()).copyAgent(program1, graph);
    std::array<std::reference_wrapper<const Agent>, 2> newPrograms{newProgram0, newProgram1};

    // Do the crossover
    AgentManager& programManager = manager.getSubManager(this->programAlgorithmID);
    this->getSubMutator(newPrograms.front().get().getAlgorithmID()).crossoverAgents(newPrograms, graph, programManager, newSubAgents, params, rng);

    SpeciesManager& speciesManager = dynamic_cast<SpeciesManager&>(manager);
    speciesManager.setProgram(agents.at(0), edge, newProgram0);
    speciesManager.setProgram(agents.at(1), edge, newProgram1);
}

void Algorithm::Species::SpeciesMutator::crossoverEdges(
    std::array<std::reference_wrapper<const Agent>, 2> agents, const EvoGraph::Edge& edge, AgentManager& manager, std::vector<std::reference_wrapper<const Agent>>& newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng)
{
    const Agent& program0 = dynamic_cast<const SpeciesAgent&>(agents.at(0).get()).getProgram(edge);
    const Agent& program1 = dynamic_cast<const SpeciesAgent&>(agents.at(1).get()).getProgram(edge);

    SpeciesManager& speciesManager = dynamic_cast<SpeciesManager&>(manager);
    speciesManager.setProgram(agents.at(0), edge, program1);
    speciesManager.setProgram(agents.at(1), edge, program0);
}

void Algorithm::Species::SpeciesMutator::crossoverAgents(
    std::array<std::reference_wrapper<const Agent>, 2> agents, EvoGraph::Graph& graph, AgentManager& manager, std::vector<std::reference_wrapper<const Agent>>& newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng)
{
    // No crossover
    if (params.mutation.tpg.pCrossAgents == 0) {
        return;
    }

    // Get available edges
    const SpeciesManager& speciesManager = dynamic_cast<const SpeciesManager&>(manager);
    std::vector<std::reference_wrapper<const EvoGraph::Edge>> availableEdges(speciesManager.getEdges().begin(), speciesManager.getEdges().end());
    size_t remaining = availableEdges.size();

    // Always do at least one crossover
    // (mearning we don't want any crossover)
    double proba = 1;
    while (remaining > 0 &&
           proba > rng.getDouble(0.0, 1.0)) {

        // Pick uniformly from remaining values
        size_t pickIdx = rng.getUnsignedInt64(0, remaining - 1);
        const EvoGraph::Edge& pickEdge = availableEdges[pickIdx];

        // Remove picked element (swap with last)
        std::swap(availableEdges[pickIdx], availableEdges[remaining - 1]);
        --remaining;

        // A crossover at program level can be done only the both parents
        // assessed the action concerned
        if (params.mutation.tpg.pCrossPrograms > rng.getDouble(0, 1)) {
            this->crossoverPrograms(agents, pickEdge, graph, manager, newSubAgents, params, rng);
        }
        else {
            this->crossoverEdges(agents, pickEdge, manager, newSubAgents, params, rng);
        }
        proba *= params.mutation.tpg.pCrossAgents;
    }

}

void Algorithm::Species::SpeciesMutator::swapActionValues(const SpeciesAgent& agent, AgentManager& manager, RNG::RNG& rng)
{
    SpeciesManager& speciesManager = dynamic_cast<SpeciesManager&>(manager);
    const SpeciesAgent& speciesAgent = dynamic_cast<const SpeciesAgent&>(agent);
    
    // Randomly select two actions
    size_t index1 =
        rng.getUnsignedInt64(0, agent.getActionLinks().size() - 1);
    size_t index2 =
        rng.getUnsignedInt64(0, agent.getActionLinks().size() - 2);
    if (index2 == index1) {
        index2++;
    }

    // Get iterators to the selected edges
    auto it1 = agent.getActionLinks().begin();
    std::advance(it1, index1);
    auto it2 = agent.getActionLinks().begin();
    std::advance(it2, index2);

    // Extract and swap programs
    size_t actionValue1 = it1->second;
    size_t actionValue2 = it2->second;

    // Set the swapped action classes
    speciesManager.setActionValue(agent, it1->first, actionValue2);
    speciesManager.setActionValue(agent, it2->first, actionValue1);
}

void Algorithm::Species::SpeciesMutator::swapPrograms(const SpeciesAgent& agent, AgentManager& manager, RNG::RNG& rng)
{
    SpeciesManager& speciesManager = dynamic_cast<SpeciesManager&>(manager);
    double probaSwapContext = 0.5;

    // By default, if thre is less than two action edges, to a context swap, if there is less than two context edges, do a action swap
    bool doSwapActionEdges = true;
    if(speciesManager.getActionEdges().size() < 2) {
        doSwapActionEdges = false;
    } else if(speciesManager.getContextEdges().size() > 1 && probaSwapContext > rng.getDouble(0.0, 1.0)) {
        doSwapActionEdges = false;
    }

    // Get the corresponding edges
    const auto& edges = (doSwapActionEdges) ? speciesManager.getActionEdges() : speciesManager.getContextEdges();

    // Randomly select two edges
    size_t index1 =
        rng.getUnsignedInt64(0, edges.size() - 1);
    size_t index2 =
        rng.getUnsignedInt64(0, edges.size() - 2);
    if (index2 == index1) {
        index2++;
    }

    // Get iterators to the selected edges
    auto it1 = edges.begin();
    std::advance(it1, index1);
    auto it2 = edges.begin();
    std::advance(it2, index2);

    // Extract and swap programs
    const Agent& actionProgram1 = agent.getProgram(*it1);
    const Agent& actionProgram2 = agent.getProgram(*it2);

    // Set the swapped action classes
    speciesManager.setProgram(agent, (*it1), actionProgram2);
    speciesManager.setProgram(agent, (*it2), actionProgram1);
}

void Algorithm::Species::SpeciesMutator::mutateOutgoingEdge(
    const Agent& agent, EvoGraph::Graph& graph, const EvoGraph::Edge& edge,
    AgentManager& manager,
    std::vector<std::reference_wrapper<const Agent>>& newSubAgents,
    const Learn::LearningParameters& params, RNG::RNG& rng)
{
    // Get the origin program
    const Agent& originProgram = dynamic_cast<const SpeciesAgent&>(agent).getProgram(edge);

    // copy program
    const Algorithm::Agent& newProgram = manager.getSubManager(originProgram.getAlgorithmID()).copyAgent(originProgram, graph);

    // Set the mutated agent to the edge
    dynamic_cast<SpeciesManager&>(manager).setProgram(agent, edge, newProgram);

    // Add it to the list of new agent to be mutated.
    newSubAgents.push_back(newProgram);
}

void Algorithm::Species::SpeciesMutator::mutateAgent(
    const Agent& agent, EvoGraph::Graph& graph, AgentManager& manager, std::vector<std::reference_wrapper<const Agent>>& newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng)
{
    SpeciesManager& speciesManager = dynamic_cast<SpeciesManager&>(manager);
    const SpeciesAgent& speciesAgent = dynamic_cast<const SpeciesAgent&>(agent);

    // Swap randomly action values
    double probaSwapActionValues = 0.5;
    double proba = probaSwapActionValues;
    while(speciesAgent.getActionLinks().size() > 1 &&
            proba > rng.getDouble(0.0, 1.0)) {
        this->swapActionValues(speciesAgent, manager, rng);

        proba *= probaSwapActionValues;
    } 

    // 3. swap randomly selected edges
    // With at least two edges
    proba = params.mutation.tpg.pSwapActionProgram;
    while (speciesManager.getEdges().size() > 2 &&
            proba > rng.getDouble(0.0, 1.0)) {
        this->swapPrograms(speciesAgent, manager, rng);

        // Decrement the proba of swapping two edges
        proba *= params.mutation.tpg.pSwapActionProgram;
    }


    bool anyMutationDone = false;
    do {
        
        // Get available actions classes
        std::vector<std::reference_wrapper<const EvoGraph::Edge>> availableEdges(speciesManager.getEdges().begin(), speciesManager.getEdges().end());
        size_t remaining = availableEdges.size();

        // 4. mutate randomly selected program on action Edge.
        double proba = params.mutation.tpg.pMutateActionProgram;
        while (remaining > 0 &&
               proba > rng.getDouble(0.0, 1.0)) {

            // Pick uniformly from remaining values
            size_t pickIdx = rng.getUnsignedInt64(0, remaining - 1);
            const EvoGraph::Edge& pickEdge = availableEdges[pickIdx];

            // Remove picked element (swap with last)
            // By swapping, the order of availableActions is changed.
            std::swap(availableEdges[pickIdx], availableEdges[remaining - 1]);
            --remaining;

            this->mutateOutgoingEdge(agent, graph, pickEdge, manager, newSubAgents, params, rng);

            proba *= params.mutation.tpg.pMutateActionProgram;
            anyMutationDone = true;
        }
    } while (!anyMutationDone && params.mutation.tpg.pMutateActionProgram != 0.0);
}


void Algorithm::Species::SpeciesMutator::mutateProgramAgentAgainstArchive(
    const Agent& programAgent, EvoGraph::Graph& graph, 
    AgentManager& manager, const Learn::LearningParameters& params, 
    RNG::RNG& rng)
{
    Mutator& subMutator = this->getSubMutator(programAgent.getAlgorithmID());

    std::vector<std::reference_wrapper<const Agent>> newSubAgents; //TODOTODOTODO
    bool allUnique;

    // Check for uniqueness in archive
    const auto& archivedDataHandlers = archive.get().getDataHandlers();
    std::map<size_t, double> hashesAndResults;
    std::unique_ptr<Algorithm::ExecutionEngine> execEngine = manager.createExecutionEngine();
    execEngine->setExecutedAgent(programAgent);

    // Mutate behavior until it changes (against the archive).
    do {

        // Mutate until something is mutated (i.e. the function returns
        // true) And until the program behavior is changed
        subMutator.mutateAgent(programAgent, graph, manager, newSubAgents, params, rng);

        hashesAndResults.clear();
        for (std::pair<
                 size_t,
                 std::vector<std::reference_wrapper<const Data::DataHandler>>>
                 archiveDatahandler : archivedDataHandlers) {
            // Execute the mutated program on the archive data handlers
            execEngine->setDataSources(archiveDatahandler.second);
            double result = execEngine->execute().at(0);
            hashesAndResults.insert({archiveDatahandler.first, result});
        }

        // If the result is not unique, do another mutation.
        allUnique = archive.get().areProgramResultsUnique(hashesAndResults);

        // Archive
    } while (!allUnique);
}

void Algorithm::Species::SpeciesMutator::mutateSubAgents(
    std::vector<std::reference_wrapper<const Agent>>& agents, EvoGraph::Graph& graph, 
    AgentManager& manager, const Learn::LearningParameters& params, 
    RNG::RNG& rng, uint64_t maxNbThreads)
{
    // This is a computing intensive part of the mutation process
    // Hence the parallelization.
    if (maxNbThreads <= 1) {
        // Sequential (kept for determinism check mostly)
        for (const Algorithm::Agent& programAgent : agents) {
            AgentManager& subManager = manager.getSubManager(programAgent.getAlgorithmID());
            RNG::RNG privateRNG(rng.getUnsignedInt64(0, UINT64_MAX));
            this->mutateProgramAgentAgainstArchive(programAgent, graph, subManager, params,
                                                privateRNG);
        }
    }
    else {
        // Parallel
        // Create job list with Program pointers and seed
        std::queue<std::pair<std::reference_wrapper<const Agent>, uint64_t>>
            programsToMutate;
        for (auto programAgent : agents) {
            programsToMutate.push(
                {programAgent, rng.getUnsignedInt64(0, UINT64_MAX)});
        }

        std::mutex mutexMutation;

        // Function executed in threads
        auto parallelWorker = [this, &programsToMutate, &mutexMutation, &params, &graph, &manager]() {
            RNG::RNG privateRNG;
            // While there is work to be done
            bool jobDone;
            do {
                std::pair<std::optional<std::reference_wrapper<const Agent>>, uint64_t> job;
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
                    AgentManager& subManager = manager.getSubManager(job.first.value().get().getAlgorithmID());
                    this->mutateProgramAgentAgainstArchive(*job.first, graph, subManager, params, privateRNG);
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