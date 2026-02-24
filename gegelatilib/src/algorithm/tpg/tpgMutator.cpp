

#include "algorithm/tpg/tpgMutator.h"


void Algorithm::TPG::TPGMutator::setArchive(const Archive& archive)
{
    this->archive = archive;
}

void Algorithm::TPG::TPGMutator::updateSpecificContext(
    EvoGraph::Graph& graph, AgentManager& manager,
    const Learn::LearningParameters& params,
    RNG::RNG& rng)
{
    // Call parent method to update currentContext
    Algorithm::Mutator::updateSpecificContext(graph, manager, params, rng);

    // Update pre-existing elements
    this->preExistingTeams.clear();
    this->preExistingActions.clear();
    this->preExistingEdges.clear();

    std::set<std::reference_wrapper<const EvoGraph::Edge>> usableEdges;
    std::set<std::reference_wrapper<const EvoGraph::Vertex>> usableVertices;
    std::queue<std::reference_wrapper<const EvoGraph::Vertex>> toVisit;

    // Initialize queue with vertices from all pre-existing agents
    for (const Algorithm::Agent& agentPtr : this->currentContext->preExistingAgents) {
        if(auto tpgAgent = dynamic_cast<const TPGAgent*>(&agentPtr)) {
            toVisit.push(tpgAgent->getVertex());
        } else {
            throw std::runtime_error("TPGMutator::updateSpecificContext: an agent in the current context is not a TPGAgent.");

        }

    }

    // BFS to collect all vertices reachable from pre-existing agents
    while (!toVisit.empty()) {
        const EvoGraph::Vertex& vertex = toVisit.front();
        toVisit.pop();

        // Skip if already visited
        if (usableVertices.find(vertex) != usableVertices.end()) {
            continue;
        }
        usableVertices.insert(vertex);

        // Add all connected vertices to the queue
        // Outgoing edges: vertices that this vertex points to
        for (const EvoGraph::Edge& edge : vertex.getOutgoingEdges()) {
            const EvoGraph::Vertex& destination = edge.getDestination();
            if (usableVertices.find(destination) == usableVertices.end()) {
                toVisit.push(destination);
            }
        }
    }

    for(const EvoGraph::Vertex& vertex: usableVertices){
        if(dynamic_cast<const EvoGraph::Team*>(&vertex) != nullptr){
            this->preExistingTeams.push_back(vertex);
        } else if (dynamic_cast<const EvoGraph::Action*>(&vertex) != nullptr){
            this->preExistingActions.push_back(vertex);
        } else {
            throw std::runtime_error("TPGMutator::updateSpecificContext: a vertex should be either a team or an action.");
        }

        usableEdges.insert(vertex.getOutgoingEdges().begin(), vertex.getOutgoingEdges().end());
    }

    this->preExistingEdges.insert(this->preExistingEdges.end(), usableEdges.begin(), usableEdges.end());
}

bool Algorithm::TPG::TPGMutator::isConfigurationValid(const Learn::LearningParameters& params, const Output::OutputHandler& outputs) const
{
    if(outputs.sizeContinuous() != 0 && outputs.sizeDiscrete() != 0){
        throw std::runtime_error("TPGMutator::initRandomPopulation: TPG does not support mixed discrete and continuous outputs.");
    } else if (outputs.sizeContinuous() != 0){

        if(outputs.size() > params.nbRegisters + 1){
            throw std::runtime_error("TPGMutator::initRandomPopulation: Number of continuous outputs exceeds the number of registers plus one.");
        }

    } else if (outputs.sizeDiscrete() != 0){
        
        if (params.mutation.tpg.maxInitOutgoingEdges > outputs.front().getNbValues()) {
            throw std::runtime_error("Maximum initial number of outgoing edges "
                                        "cannot exceed the number of outputs");
        }

        if (outputs.front().getNbValues() < 2) {
            throw std::runtime_error(
                "A TPG with a single output makes no sense.");
        }

        if (outputs.size() != 1) {
            throw std::runtime_error(
                "TPG for discrete actions only supports one action"
            );
        }
    } else {
        throw std::runtime_error("TPGMutator::initRandomPopulation: No outputs defined.");
    }
    
    if (params.mutation.tpg.maxInitOutgoingEdges < 2) {
        throw std::runtime_error(
            "A team should have at least two edges at initialisation.");
    }
    return true;
}

void Algorithm::TPG::TPGMutator::addAditionnalEdges(
            EvoGraph::Graph& graph,
            std::vector<std::reference_wrapper<const EvoGraph::Vertex>> leafVertices,
            std::vector<std::reference_wrapper<const EvoGraph::Vertex>> rootVertices,
            std::vector<std::reference_wrapper<const Agent>> programAgent,
            const Learn::LearningParameters& params, RNG::RNG& rng)
{
    
    // Add additional connections to TPG
    // Team-by-Team
    for (const EvoGraph::Vertex& rootVertex : rootVertices) {
        // Pick a number of additional outedge
        size_t nbAdditionalEdges =
            rng.getUnsignedInt64(0, params.mutation.tpg.maxInitOutgoingEdges - 2);

        if (nbAdditionalEdges > 0) {
            // Copy the list of programs
            std::vector<int> availableChoices(programAgent.size());
            std::iota(availableChoices.begin(), availableChoices.end(), 0);
            // Remove already connected ones
            auto iter = availableChoices.begin();
            while (iter < availableChoices.end()) {
                if (std::count_if(
                        rootVertex.getOutgoingEdges().begin(),
                        rootVertex.getOutgoingEdges().end(),
                        [&iter, &programAgent](
                            const EvoGraph::Edge& edge) {
                            return edge.getProgram() ==
                                   programAgent.at(*iter).get();
                        }) > 0) {
                    iter = availableChoices.erase(iter);
                }
                else {
                    iter++;
                }
            }

            // For each additional edge to add
            for (uint64_t i = 0;
                 i < nbAdditionalEdges && availableChoices.size() > 0; i++) {
                // Pick nbAdditionalEdges availabe programs and add them to the
                // team
                uint64_t progIndex =
                    rng.getUnsignedInt64(0, availableChoices.size() - 1);

                // Add the connection
                graph.addNewEdge(
                    rootVertex,
                    leafVertices.at(
                        rng.getUnsignedInt64(0, leafVertices.size() - 1)),
                    programAgent.at(availableChoices.at(progIndex)));

                availableChoices.erase(availableChoices.begin() + progIndex);
            }
        }
    }
}

void Algorithm::TPG::TPGMutator::initRandomPopulation(EvoGraph::Graph& graph, AgentManager& manager, const Learn::LearningParameters& params, RNG::RNG& rng)
{
    auto outputs = manager.getOutputs();
    this->isConfigurationValid(params, outputs);
    
    // Empty agent manager
    manager.clearAgents(graph);

    // Number of action vertices needed
    size_t nbActionVertices = (outputs.sizeDiscrete() == 0) ? 1 : outputs.front().getNbValues();

    // Create teams, programs and Actions
    std::vector<std::reference_wrapper<const EvoGraph::Action>> actions(this->initActionVertices(graph, nbActionVertices));
    std::vector<std::reference_wrapper<const EvoGraph::Vertex>> teams;
    std::vector<std::reference_wrapper<const Agent>> programAgents;


    for (size_t idx = 0; idx < manager.getExpectedNbAgents(); idx++) {
        teams.push_back(dynamic_cast<const TPGAgent&>(manager.createAgent(graph)).getVertex());
    }

    // Connect each team with two distinct actions, through two distinct
    // programs Association here are determinists since randomness would
    // uselessly complicate the code while bringing no real value since anyway,
    // Programs have been initialized randomly.
    Mutator& programMutator = this->getSubMutator(this->programAlgorithmID);
    AgentManager& programManager = manager.getSubManager(this->programAlgorithmID);
    for (size_t i = 0; i < 2 * manager.getExpectedNbAgents(); i++) {

        // Create a program agent
        programAgents.push_back(programMutator.initRandomAgent(graph, programManager, params, rng));

        // Add the edge
        graph.addNewEdge(teams.at(i / 2), actions.at(i % actions.size()),
                         programAgents.at(i));
    }

    std::vector<std::reference_wrapper<const EvoGraph::Vertex>> actionsVertex(actions.begin(), actions.end());
    this->addAditionnalEdges(graph, actionsVertex, teams, programAgents, params, rng);
}

void Algorithm::TPG::TPGMutator::initRandomSpecificAgent(const Agent& agent, EvoGraph::Graph& graph, AgentManager& manager, const Learn::LearningParameters& params, RNG::RNG& rng)
{
    // First agent is initialized, check validity of the configuration.
    if(manager.getAgents().size() == 1){
        this->isConfigurationValid(params, manager.getOutputs());
        // Number of action vertices needed
        size_t nbActionVertices = (manager.getOutputs().sizeDiscrete() == 0) ? 1 : manager.getOutputs().front().getNbValues();
        this->initActionVertices(graph, nbActionVertices);
    }

    manager.emptyAgent(agent, graph);
    const EvoGraph::Vertex& vertex = dynamic_cast<const TPGAgent&>(agent).getVertex();

    Mutator& programMutator = this->getSubMutator(this->programAlgorithmID);
    AgentManager& programManager = manager.getSubManager(this->programAlgorithmID);

    // Get the actions vertices.
    auto actions = graph.getActions();

    size_t nbEdges = rng.getUnsignedInt64(2, params.mutation.tpg.maxInitOutgoingEdges);
    for(size_t idx = 0; idx < nbEdges; idx++){
        // Add edge
        graph.addNewEdge(vertex, actions.at(rng.getUnsignedInt64(0, actions.size() - 1)),
                            programMutator.initRandomAgent(graph, programManager, params, rng));
    }
}

void Algorithm::TPG::TPGMutator::removeRandomEdge(EvoGraph::Graph& graph,
                                                const EvoGraph::Vertex& vertex,
                                                RNG::RNG& rng)
{
    // Pick an outgoing edge randomly,
    auto pickableEdges = vertex.getOutgoingEdges();

    // Note: No need to take special care of Actions. Since cycles can not
    // appear in TPG with the current mutation process, there is no need to
    // maintain an action within each team.

    // Pick a random edge
    auto iterSet = pickableEdges.begin();
    std::advance(iterSet, rng.getUnsignedInt64(0, pickableEdges.size() - 1));
    const EvoGraph::Edge& removedEdge = *iterSet;
    graph.removeEdge(removedEdge);
}



void Algorithm::TPG::TPGMutator::addRandomEdge(
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
                           return &edge == nullptr ||
                                  edge.getSource() == team ||
                                  edge.getDestination() == team;
                       }),
        pickableEdges.end());

    // Pick a pickable Edge
    // (This code assumes that the set of pickable edge is never empty..
    // otherwise it will throw an exception. Possible solution if needed
    // initialize an entirely new program and pick a random target.)
    auto iter = pickableEdges.begin();
    std::advance(iter, rng.getUnsignedInt64(0, pickableEdges.size() - 1));
    const EvoGraph::Edge& pickedEdge = *iter;

    // Create new edge from team and with the same ProgramSharedPointer
    // But with the team as its source
    // throw std::runtime_error if the edge is not from the graph;
    graph.setEdgeSource(graph.cloneEdge(pickedEdge), team);
}

void Algorithm::TPG::TPGMutator::mutateEdgeDestination(
    EvoGraph::Graph& graph, const EvoGraph::Edge& edge,
    const Learn::LearningParameters& params, RNG::RNG& rng)
{
    // Should the new target be an action or a team
    bool targetAction =
        rng.getDouble(0, 1) < params.mutation.tpg.pEdgeDestinationIsAction;

    // Pick any target
    // Note: Having an action in all teams is no longer enforced,
    // as the presence of cycle in TPGs is not possible according to the current
    // mutation process.
    auto& list = (targetAction) ? this->preExistingActions : this->preExistingTeams;
    const EvoGraph::Vertex& target = list.at(rng.getUnsignedInt64(0, list.size() - 1));


    // Change the target
    // Changing the target should not fail.
    graph.setEdgeDestination(edge, target);
}

void Algorithm::TPG::TPGMutator::mutateOutgoingEdge(
    EvoGraph::Graph& graph, const EvoGraph::Edge& edge,
    AgentManager& manager,
    std::vector<std::reference_wrapper<const Agent>>& newSubAgents,
    const Learn::LearningParameters& params, RNG::RNG& rng)
{
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
        mutateEdgeDestination(graph, edge, params, rng);
    }
}

void Algorithm::TPG::TPGMutator::mutateAgent(
    const Agent& agent, EvoGraph::Graph& graph, AgentManager& manager, std::vector<std::reference_wrapper<const Agent>>& newSubAgents, const Learn::LearningParameters& params, RNG::RNG& rng)
{
    const EvoGraph::Vertex& vertex = dynamic_cast<const TPGAgent&>(agent).getVertex();
    const EvoGraph::Team& team = dynamic_cast<const EvoGraph::Team&>(vertex);

    // 1. Remove randomly selected edges
    // Keep at least two edges (otherwise the team is useless)
    double proba = params.mutation.tpg.pEdgeDeletion;
    while (team.getOutgoingEdges().size() > 2 &&
            proba > rng.getDouble(0.0, 1.0)) {
        this->removeRandomEdge(graph, team, rng);

        // Decrement the proba of removing another edge
        proba *= params.mutation.tpg.pEdgeDeletion;
    }

    // 2. Add random duplicated edge with the team as its source
    proba = params.mutation.tpg.pEdgeAddition;
    while (team.getOutgoingEdges().size() < params.mutation.tpg.maxOutgoingEdges &&
            proba > rng.getDouble(0.0, 1.0)) {
        // Add an edge (by duplication of an existing one)
        this->addRandomEdge(graph, team, rng);
        // Decrement the proba of adding another edge
        proba *= params.mutation.tpg.pEdgeAddition;
    }

    // 3. Mutate edges of the team
    bool anyMutationDone = false;
    do {
        // Process edge-by-edge
        // And possibly modify their target
        for (const EvoGraph::Edge& edge : team.getOutgoingEdges()) {
            // Edge->Program bid modification
            if (rng.getDouble(0.0, 1.0) < params.mutation.tpg.pProgramMutation) {
                // Mutate the edge
                this->mutateOutgoingEdge(graph, edge, manager, newSubAgents,
                                    params, rng);
                anyMutationDone = true;
            }
        }
    } while (!anyMutationDone && params.mutation.tpg.pProgramMutation > 0);
}


void Algorithm::TPG::TPGMutator::mutateProgramAgentAgainstArchive(
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

void Algorithm::TPG::TPGMutator::mutateSubAgents(
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