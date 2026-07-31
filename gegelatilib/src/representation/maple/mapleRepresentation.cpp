
#include "representation/maple/mapleRepresentation.h"

std::unique_ptr<Representation::Representation> Representation::Maple::MapleRepresentation::copy() const
{
    return std::make_unique<MapleRepresentation>(this->cGetSubRepresentation(this->programRepresentationID), 
        std::make_unique<RepresentationParameters>(*this->params), this->representationName, this->representationColor);
}

void Representation::Maple::MapleRepresentation::initPopulation()
{
    this->population = std::make_unique<Maple::MaplePopulation>(*this->outputs, this->representationID);
}

void Representation::Maple::MapleRepresentation::initMutator()
{
    this->mutator = std::make_unique<Maple::MapleMutator>(*this->selector, this->representationID, *this->archive);
}


void Representation::Maple::MapleRepresentation::initRepresentation(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph)
{
    if(this->params->maple.nbActionEdgeInit == 0) {
        size_t nbOutputs = (outputs.sizeDiscrete() == 1) ? outputs.front().getNbValues() : outputs.size();
        this->params->maple.nbActionEdgeInit = nbOutputs;
    }
    Representation::Representation::initRepresentation(rng, outputs, dataSource, graph);
}


void Representation::Maple::MapleRepresentation::initSubRepresentations(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph)
{
    // Initialize program representation.
    Representation& programAlgo = this->getSubRepresentation(this->programRepresentationID);

    // Program output is only size 1
    auto programOutput = std::make_shared<Output::OutputHandler>(Output::Output());
    programAlgo.initRepresentation(rng, *programOutput, dataSource, graph);

    // Add program population and mutator to TPG population and mutator
    this->population->addSubPopulation(programAlgo.getPopulation());
    MaplePopulation* maplePopulation = dynamic_cast<MaplePopulation*>(this->population.get());
    maplePopulation->setProgramRepresentationID(this->programRepresentationID);

    this->mutator->addSubMutator(programAlgo.getMutator());
    MapleMutator* mapleMutator = dynamic_cast<MapleMutator*>(this->mutator.get());
    mapleMutator->setProgramRepresentationID(this->programRepresentationID);
}

void Representation::Maple::MapleRepresentation::printCodeGenAgents(std::ofstream& fileMain, std::ofstream& fileMainH, const std::set<std::reference_wrapper<const Individual>>& agents, std::map<uint64_t, std::set<std::reference_wrapper<const Individual>>>& subAgents) const
{
    const Representation& programAlgo = this->cGetSubRepresentation(this->programRepresentationID);

    // set of all used vertex by the list of agents
    std::set<std::reference_wrapper<const EvoGraph::Vertex>> printedVertices;
    for(const Individual& agent: agents) {
        if(auto mapleIndividual = dynamic_cast<const MapleIndividual*>(&agent)) {

            fileMain
                << "void "<< this->representationName << this->representationID << "_" <<agent.getAgentID() << "(double* outputs) {\n";

            const EvoGraph::Vertex& vertex = mapleIndividual->getVertex();
            for(const EvoGraph::Edge& edge: vertex.getOutgoingEdges()) {
                subAgents.at(this->programRepresentationID).insert(edge.getProgram());

                size_t actionId = 0;
                if(auto action = dynamic_cast< const EvoGraph::Action*>(&edge.getDestination())) {
                    actionId = action->getActionID();
                } else {
                    throw std::runtime_error("MapleRepresentation::printCodeGenAgents: destination vertices should be action vertices with maple");
                }


                fileMain
                    << "\t"<< programAlgo.getRepresentationName() << programAlgo.getRepresentationID() << "_" << edge.getProgram().getAgentID() << "(outputs + " <<actionId << ");\n";
            }

            fileMain
                << "}\n";
        } else {
            throw std::runtime_error("MapleRepresentation::printCodeGenAgents: agent should be a maple agent");
        }
    }
}