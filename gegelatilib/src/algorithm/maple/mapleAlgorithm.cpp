
#include "algorithm/maple/mapleAlgorithm.h"

std::unique_ptr<Algorithm::Algorithm> Algorithm::Maple::MapleAlgorithm::copy() const
{
    return std::make_unique<MapleAlgorithm>(this->cGetSubAlgorithm(this->programAlgorithmID), 
        std::make_unique<AlgorithmParameters>(*this->params), this->algorithmName, this->algorithmColor);
}

void Algorithm::Maple::MapleAlgorithm::initManager()
{
    this->manager = std::make_unique<Maple::MapleManager>(*this->outputs, this->algorithmID);
}

void Algorithm::Maple::MapleAlgorithm::initMutator()
{
    this->mutator = std::make_unique<Maple::MapleMutator>(*this->selector, this->algorithmID, *this->archive);
}

void Algorithm::Maple::MapleAlgorithm::initSubAlgorithms(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph)
{
    // Initialize program algorithm.
    Algorithm& programAlgo = this->getSubAlgorithm(this->programAlgorithmID);

    // Program output is only size 1
    auto programOutput = std::make_shared<Output::OutputHandler>(Output::Output());
    programAlgo.initAlgorithm(rng, *programOutput, dataSource, graph);

    // Add program manager and mutator to TPG manager and mutator
    this->manager->addSubManager(programAlgo.getManager());
    MapleManager* mapleManager = dynamic_cast<MapleManager*>(this->manager.get());
    mapleManager->setProgramAlgorithmID(this->programAlgorithmID);

    this->mutator->addSubMutator(programAlgo.getMutator());
    MapleMutator* mapleMutator = dynamic_cast<MapleMutator*>(this->mutator.get());
    mapleMutator->setProgramAlgorithmID(this->programAlgorithmID);
}

void Algorithm::Maple::MapleAlgorithm::printCodeGenAgents(std::ofstream& fileMain, std::ofstream& fileMainH, const std::set<std::reference_wrapper<const Agent>>& agents, std::map<uint64_t, std::set<std::reference_wrapper<const Agent>>>& subAgents) const
{
    const Algorithm& programAlgo = this->cGetSubAlgorithm(this->programAlgorithmID);

    // set of all used vertex by the list of agents
    std::set<std::reference_wrapper<const EvoGraph::Vertex>> printedVertices;
    for(const Agent& agent: agents) {
        if(auto mapleAgent = dynamic_cast<const MapleAgent*>(&agent)) {

            fileMain
                << "void "<< this->algorithmName << this->algorithmID << "_" <<agent.getAgentID() << "(double* outputs) {\n";

            const EvoGraph::Vertex& vertex = mapleAgent->getVertex();
            for(const EvoGraph::Edge& edge: vertex.getOutgoingEdges()) {
                subAgents.at(this->programAlgorithmID).insert(edge.getProgram());

                size_t actionId = 0;
                if(auto action = dynamic_cast< const EvoGraph::Action*>(&edge.getDestination())) {
                    actionId = action->getActionID();
                } else {
                    throw std::runtime_error("MapleAlgorithm::printCodeGenAgents: destination vertices should be action vertices with maple");
                }


                fileMain
                    << "\t"<< programAlgo.getAlgorithmName() << programAlgo.getAlgorithmID() << "_" << edge.getProgram().getAgentID() << "(outputs + " <<actionId << ");\n";
            }

            fileMain
                << "}\n";
        } else {
            throw std::runtime_error("MapleAlgorithm::printCodeGenAgents: agent should be a maple agent");
        }
    }
}