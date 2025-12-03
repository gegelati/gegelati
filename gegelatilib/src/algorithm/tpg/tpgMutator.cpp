
#include "algorithm/tpg/tpgMutator.h"


void Algorithm::TPG::TPGMutator::initRandomPopulation(std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<AgentManager> manager, const Learn::LearningParameters& params, RNG::RNG& rng, size_t nbActions)
{

    if (params.mutation.tpg.maxInitOutgoingEdges > nbActions) {
        throw std::runtime_error("Maximum initial number of outgoing edges "
                                    "cannot exceed the number of action");
    }
    if (nbActions < 2) {
        throw std::runtime_error(
            "A TPG with a single action makes no sense.");
    }
    if (params.mutation.tpg.maxInitOutgoingEdges < 2) {
        throw std::runtime_error(
            "A team should have at least two edges at initialisation.");
    }

    // Empty agent manager
    manager->clearAgents();

    // Create teams, programs and Actions
    std::vector<const EvoGraph::Action*> actions;
    std::vector<const EvoGraph::Vertex*> teams;
    std::vector<std::shared_ptr<Program::Program>> programs;


    for (size_t idx = 0; idx < nbActions; idx++) {
        actions.push_back(&(graph->addNewAction(idx)));
    }
    for (size_t idx = 0; idx < params.mutation.tpg.nbRoots; idx++) {
        auto agent = manager->createAgent(graph);
        teams.push_back(&std::dynamic_pointer_cast<const TPGAgent>(manager->createAgent(graph))->getVertex());
    }

    // Connect each team with two distinct actions, through two distinct
    // programs Association here are determinists since randomness would
    // uselessly complicate the code while bringing no real value since anyway,
    // Programs have been initialized randomly.
    for (size_t i = 0; i < 2 * params.mutation.tpg.nbRoots; i++) {
        // Create a program and specify context program
        programs.emplace_back(
            new Program::Program(graph->getEnvironment(), false));
        // RandomInit the Programs
        Mutator::ProgramMutator::initRandomProgram(*programs.back(), params,
                                                   rng);
        graph->addNewEdge(*teams.at(i / 2), *actions.at(i % nbActions),
                         programs.at(i));
    }

    // Add additional connections to TPG
    // Team-by-Team
    for (const EvoGraph::Vertex* team : teams) {
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
                std::vector<int> availableChoices(programs.size());
                std::iota(availableChoices.begin(), availableChoices.end(), 0);
                // Remove already connected ones
                auto iter = availableChoices.begin();
                while (iter < availableChoices.end()) {
                    if (std::count_if(
                            team->getOutgoingEdges().begin(),
                            team->getOutgoingEdges().end(),
                            [&iter, &programs](const EvoGraph::Edge* edge) {
                                return &edge->getProgram() ==
                                       programs.at(*iter).get();
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
                 programs.at(randomProgIndex[1]).use_count() <
                     programs.at(randomProgIndex[0]).use_count())
                    ? randomProgIndex[1]
                    : randomProgIndex[0];

            // Add the connection
            graph->addNewEdge(
                *team,
                *actions.at(rng.getUnsignedInt64(0, nbActions - 1)),
                programs.at(selectedProgramIndex));
        }
    }
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
    const std::list<EvoGraph::Edge*>& pickableEdges = team.getOutgoingEdges();

    // Note: No need to take special care of Actions. Since cycles can not
    // appear in TPG with the current mutation process, there is no need to
    // maintain an action within each team.

    // Pick a random edge
    auto iterSet = pickableEdges.begin();
    std::advance(iterSet, rng.getUnsignedInt64(0, pickableEdges.size() - 1));
    const EvoGraph::Edge* removedEdge = *iterSet;
    graph->removeEdge(*removedEdge);
}



void Algorithm::TPG::TPGMutator::addRandomEdge(
    std::shared_ptr<EvoGraph::Graph> graph, const EvoGraph::Team& team,
    const Selector::SelectionContext& context, RNG::RNG& rng)
{
    // Pick an edge (excluding ones from the team, edges with the team as a
    // destination and the edges that are action edges)
    auto pickableEdges(context.preExistingEdges);
    // cf erase-remove idiom
    pickableEdges.erase(
        std::remove_if(pickableEdges.begin(), pickableEdges.end(),
                       [&team](const EvoGraph::Edge* edge) -> bool {
                           return edge == nullptr ||
                                  dynamic_cast<const EvoGraph::ActionEdge*>(
                                      edge) != nullptr ||
                                  edge->getSource() == &team ||
                                  edge->getDestination() == &team;
                       }),
        pickableEdges.end());

    // Pick a pickable Edge
    // (This code assumes that the set of pickable edge is never empty..
    // otherwise it will throw an exception. Possible solution if needed
    // initialize an entirely new program and pick a random target.)
    std::list<const EvoGraph::Edge*>::iterator iter = pickableEdges.begin();
    std::advance(iter, rng.getUnsignedInt64(0, pickableEdges.size() - 1));
    const EvoGraph::Edge* pickedEdge = *iter;

    // Create new edge from team and with the same ProgramSharedPointer
    // But with the team as its source
    // throw std::runtime_error if the edge is not from the graph;
    const EvoGraph::Edge& newEdge = graph->cloneEdge(*pickedEdge);
    graph->setEdgeSource(newEdge, team);
}

void Algorithm::TPG::TPGMutator::mutateEdgeDestination(
    std::shared_ptr<EvoGraph::Graph> graph, const EvoGraph::Edge* edge,
    const Selector::SelectionContext& context,
    const Learn::LearningParameters& params, RNG::RNG& rng)
{
    // Pick an edge among preexisting vertices
    const EvoGraph::Vertex* target = nullptr;

    // Should the new target be an action or a team
    bool targetAction =
        rng.getDouble(0, 1) < params.mutation.tpg.pEdgeDestinationIsAction;

    // Pick any target
    // Note: Having an action in all teams is no longer enforced,
    // as the presence of cycle in TPGs is not possible according to the current
    // mutation process.
    if (targetAction) {
        target = context.preExistingActions.at(rng.getUnsignedInt64(
            0, context.preExistingActions.size() - 1));
    } else {
        target = context.preExistingTeams.at(
            rng.getUnsignedInt64(0, context.preExistingTeams.size() - 1));
    }

    // Change the target
    // Changing the target should not fail.
    graph->setEdgeDestination(*edge, *target);
}

void Algorithm::TPG::TPGMutator::mutateOutgoingEdge(
    std::shared_ptr<EvoGraph::Graph> graph, const EvoGraph::Edge* edge,
    const Selector::SelectionContext& context,
    std::vector<std::shared_ptr<const Agent>> newSubAgents,
    const Learn::LearningParameters& params, RNG::RNG& rng)
{

    // copy program
    std::shared_ptr<Program::Program> newProg(
        new Program::Program(edge->getProgram(), false));

    // Set the mutated program to the edge
    edge->setProgram(newProg);

    // Add it to the list of new Program to be mutated.
    newSubAgents.push_back(newProg);

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
    const EvoGraph::Vertex& vertex = std::dynamic_pointer_cast<const TPGAgent>(agent)->getVertex();
    const EvoGraph::Team& team = *dynamic_cast<const EvoGraph::Team*>(&vertex);

    // 1. Remove randomly selected edges
    {
        // Keep at least two edges (otherwise the team is useless)
        double proba = 1.0;
        while (team.getOutgoingEdges().size() > 2 &&
               proba > rng.getDouble(0.0, 1.0)) {

            this->removeRandomEdge(graph, team, rng);

            // Decrement the proba of removing another edge
            proba *= params.mutation.tpg.pEdgeDeletion;
        }
    }

    // 2. Add random duplicated edge with the team as its source
    {
        double proba = 1.0;
        while (team.getOutgoingEdges().size() < params.mutation.tpg.maxOutgoingEdges &&
               proba > rng.getDouble(0.0, 1.0)) {
            // Add an edge (by duplication of an existing one)
            this->addRandomEdge(graph, team, context, rng);
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
            for (EvoGraph::Edge* edge : team.getOutgoingEdges()) {
                // Edge->Program bid modification
                if (rng.getDouble(0.0, 1.0) < params.mutation.tpg.pProgramMutation) {
                    // Mutate the edge
                    this->mutateOutgoingEdge(graph, edge, context, newSubAgents,
                                       params, rng);
                    anyMutationDone = true;
                }
            }
        } while (!anyMutationDone);
    }
}
