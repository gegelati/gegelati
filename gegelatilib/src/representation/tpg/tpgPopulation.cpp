
#include "representation/tpg/tpgPopulation.h"

Representation::TPG::TpgIndividual& Representation::TPG::TpgPopulation::getTpgIndividualFromCst(const Individual& individual)
{
    return dynamic_cast<TpgIndividual&>(**this->getIndividualFromCst(individual));
}

const std::vector<std::reference_wrapper<const Representation::Individual>> Representation::TPG::TpgPopulation::getIndividuals() const
{
    std::vector<std::reference_wrapper<const Representation::Individual>> constIndividuals;

    // Transform each root from shared_ptr<Individual> to shared_ptr<const Individual>
    for(auto it = this->individuals.begin(); it != this->individuals.end(); ){
        const TpgIndividual* tpgIndividual = dynamic_cast<const TpgIndividual*>((*it).get());
        if(tpgIndividual == nullptr){
            throw std::runtime_error("TpgPopulation::getIndividuals: an individual managed by the TpgPopulation is not a TpgIndividual.");
        } else if (tpgIndividual->isRoot()){
            constIndividuals.push_back(**it);
        }
        it++;
    }

    return constIndividuals;
}


const Representation::Individual& Representation::TPG::TpgPopulation::createIndividual(EvoGraph::Graph& graph)
{
    return this->createIndividual(graph.addNewTeam());
}

const Representation::Individual& Representation::TPG::TpgPopulation::createIndividual(std::optional<std::reference_wrapper<const EvoGraph::Vertex>> vertex)
{
    this->individuals.insert(std::make_unique<TpgIndividual>(vertex, this->getRepresentationID()));
    return **this->individuals.rbegin();
}

const Representation::Individual& Representation::TPG::TpgPopulation::createEmptyIndividual()
{
    std::optional<std::reference_wrapper<const EvoGraph::Vertex>> vertex = std::nullopt;
    return this->createIndividual(vertex);
}

const Representation::Individual& Representation::TPG::TpgPopulation::copyIndividual(const Individual& individual, EvoGraph::Graph& graph)
{
    const TpgIndividual& castedIndividual = dynamic_cast<const TpgIndividual&>(individual);
    if(&castedIndividual == nullptr){
        throw std::runtime_error("Representation::TPG::TpgPopulation::copyIndividual: trying to copy an individual that is not a TpgIndividual.");
    }

    // Set to castedIndividual to avoid unset references
    std::reference_wrapper<const EvoGraph::Vertex> newVertex = castedIndividual.getVertex();

    if(individual.getRepresentationID() != this->getRepresentationID()){
        // Since the individual dupplicated is not from the same representation, we also need to dupplicate the sub individuals on the edge of the vertex.
        newVertex = graph.addNewTeam();
        for(const EvoGraph::Edge& edge: castedIndividual.getVertex().getOutgoingEdges()){
            const Representation::Individual& newSubIndividual = this->getSubPopulation(this->programRepresentationID).copyIndividual(edge.getProgram(), graph);
            graph.addNewEdge(newVertex, edge.getDestination(), newSubIndividual);
        }
        
    } else {
        newVertex = graph.cloneVertex(castedIndividual.getVertex());
    }

    return this->createIndividual(newVertex);
}

void Representation::TPG::TpgPopulation::deleteIndividual(const Individual& individual, EvoGraph::Graph& graph)
{
    this->emptyIndividual(individual, graph);
    // Do not remove action individuals from the graph
    if(auto vertex = dynamic_cast<const EvoGraph::Team*>(&this->getTpgIndividualFromCst(individual).getVertex())){
        graph.removeVertex(*vertex);
    }

    auto iterator = this->individuals.find(&individual);
    this->individuals.erase(iterator);   
}
void Representation::TPG::TpgPopulation::emptyIndividual(const Individual& individual, EvoGraph::Graph& graph)
{
    // Do not remove action individuals from the graph
    if(auto vertex = dynamic_cast<const EvoGraph::Team*>(&this->getTpgIndividualFromCst(individual).getVertex())){
        while(vertex->getOutgoingEdges().size() > 0){
            graph.removeEdge(vertex->getOutgoingEdges().front());
        }
    }
}

void Representation::TPG::TpgPopulation::setVertex(const Individual& individual, const EvoGraph::Vertex& vertex)
{
    const EvoGraph::Team& team = dynamic_cast<const EvoGraph::Team&>(vertex);
    if(&team == nullptr){
        throw std::runtime_error("TpgPopulation::setVertex: trying to set an individual on a vertex from the graph that is not a team.");
    }

    // Set the element
    this->getTpgIndividualFromCst(individual).setVertex(vertex);
}


std::unique_ptr<Representation::ExecutionEngine> Representation::TPG::TpgPopulation::createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources, bool isTraining) const
{
    auto engine = std::make_unique<TPG::TPGExecutionEngine>(this->outputs, this->representationID, isTraining);

    engine->setProgramExecutionEngine(
        std::move(this->cGetSubPopulation(this->programRepresentationID).createExecutionEngine(dataSources, isTraining))
    );

    return engine;
}