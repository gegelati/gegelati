

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

    const EvoGraph::Team& team = graph.addNewTeam();
    for(const EvoGraph::Action& action: actions){
        graph.addNewEdge(team, action);
    }
    //const EvoGraph::Action& action = actions.at(rng.getUnsignedInt64(0, nbActionVertices - 1));
    return team;
}

void Algorithm::Species::SpeciesMutator::initRandomPopulation(EvoGraph::Graph& graph, AgentManager& manager, const Learn::LearningParameters& params, RNG::RNG& rng)
{
    const auto& outputs = manager.getOutputs();
    this->isConfigurationValid(params, outputs);
    this->initActionVertices(graph, manager.getOutputs().size());
    
    // Empty agent manager
    manager.clearAgents(graph);
    
    for (size_t idx = 0; idx < params.mutation.tpg.nbRoots; idx++) {
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
    const SpeciesManager& speciesManager = dynamic_cast<const SpeciesManager&>(manager);

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