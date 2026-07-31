
#include "representation/tpg/tpgPopulation.h"

Representation::TPG::TpgIndividual& Representation::TPG::TpgPopulation::getTpgIndividualFromCst(const Individual& agent)
{
    return dynamic_cast<TpgIndividual&>(**this->getAgentFromCst(agent));
}

const std::vector<std::reference_wrapper<const Representation::Individual>> Representation::TPG::TpgPopulation::getAgents() const
{
    std::vector<std::reference_wrapper<const Representation::Individual>> constAgents;

    // Transform each root from shared_ptr<Individual> to shared_ptr<const Individual>
    for(auto it = this->agents.begin(); it != this->agents.end(); ){
        const TpgIndividual* tpgIndividual = dynamic_cast<const TpgIndividual*>((*it).get());
        if(tpgIndividual == nullptr){
            throw std::runtime_error("TpgPopulation::getAgents: an agent managed by the TpgPopulation is not a TpgIndividual.");
        } else if (tpgIndividual->isRoot()){
            constAgents.push_back(**it);
        }
        it++;
    }

    return constAgents;
}


const Representation::Individual& Representation::TPG::TpgPopulation::createAgent(EvoGraph::Graph& graph)
{
    return this->createAgent(graph.addNewTeam());
}

const Representation::Individual& Representation::TPG::TpgPopulation::createAgent(std::optional<std::reference_wrapper<const EvoGraph::Vertex>> vertex)
{
    this->agents.insert(std::make_unique<TpgIndividual>(vertex, this->getRepresentationID()));
    return **this->agents.rbegin();
}

const Representation::Individual& Representation::TPG::TpgPopulation::createEmptyAgent()
{
    std::optional<std::reference_wrapper<const EvoGraph::Vertex>> vertex = std::nullopt;
    return this->createAgent(vertex);
}

const Representation::Individual& Representation::TPG::TpgPopulation::copyAgent(const Individual& agent, EvoGraph::Graph& graph)
{
    const TpgIndividual& castedAgent = dynamic_cast<const TpgIndividual&>(agent);
    if(&castedAgent == nullptr){
        throw std::runtime_error("Representation::TPG::TpgPopulation::copyAgent: trying to copy an agent that is not a TpgIndividual.");
    }

    // Set to castedAgent to avoid unset references
    std::reference_wrapper<const EvoGraph::Vertex> newVertex = castedAgent.getVertex();

    if(agent.getRepresentationID() != this->getRepresentationID()){
        // Since the agent dupplicated is not from the same representation, we also need to dupplicate the sub agents on the edge of the vertex.
        newVertex = graph.addNewTeam();
        for(const EvoGraph::Edge& edge: castedAgent.getVertex().getOutgoingEdges()){
            const Representation::Individual& newSubAgent = this->getSubPopulation(this->programRepresentationID).copyAgent(edge.getProgram(), graph);
            graph.addNewEdge(newVertex, edge.getDestination(), newSubAgent);
        }
        
    } else {
        newVertex = graph.cloneVertex(castedAgent.getVertex());
    }

    return this->createAgent(newVertex);
}

void Representation::TPG::TpgPopulation::deleteAgent(const Individual& agent, EvoGraph::Graph& graph)
{
    this->emptyAgent(agent, graph);
    // Do not remove action agents from the graph
    if(auto vertex = dynamic_cast<const EvoGraph::Team*>(&this->getTpgIndividualFromCst(agent).getVertex())){
        graph.removeVertex(*vertex);
    }

    auto iterator = this->agents.find(&agent);
    this->agents.erase(iterator);   
}
void Representation::TPG::TpgPopulation::emptyAgent(const Individual& agent, EvoGraph::Graph& graph)
{
    // Do not remove action agents from the graph
    if(auto vertex = dynamic_cast<const EvoGraph::Team*>(&this->getTpgIndividualFromCst(agent).getVertex())){
        while(vertex->getOutgoingEdges().size() > 0){
            graph.removeEdge(vertex->getOutgoingEdges().front());
        }
    }
}

void Representation::TPG::TpgPopulation::setVertex(const Individual& agent, const EvoGraph::Vertex& vertex)
{
    const EvoGraph::Team& team = dynamic_cast<const EvoGraph::Team&>(vertex);
    if(&team == nullptr){
        throw std::runtime_error("TpgPopulation::setVertex: trying to set an agent on a vertex from the graph that is not a team.");
    }

    // Set the element
    this->getTpgIndividualFromCst(agent).setVertex(vertex);
}


std::unique_ptr<Representation::ExecutionEngine> Representation::TPG::TpgPopulation::createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources, bool isTraining) const
{
    auto engine = std::make_unique<TPG::TPGExecutionEngine>(this->outputs, this->representationID, isTraining);

    engine->setProgramExecutionEngine(
        std::move(this->cGetSubPopulation(this->programRepresentationID).createExecutionEngine(dataSources, isTraining))
    );

    return engine;
}