
#include "algorithm/tpg/tpgManager.h"


std::shared_ptr<Algorithm::TPG::TPGAgent> Algorithm::TPG::TPGManager::getTPGAgentFromCst(std::shared_ptr<const Agent> agent)
{
    auto iterator = this->agents.find(agent);
    if(iterator == this->agents.end() || *iterator != agent){
        throw std::invalid_argument("TPGManager::getTPGAgentFromCst: the given agent is not managed by this manager.");
    }

    return std::dynamic_pointer_cast<TPGAgent>(*iterator);
}

const std::vector<std::shared_ptr<const Algorithm::Agent>> Algorithm::TPG::TPGManager::getAgents() const
{
    std::vector<std::shared_ptr<const Algorithm::Agent>> constAgents;

    // Transform each root from shared_ptr<Agent> to shared_ptr<const Agent>
    for(auto agent: this->agents){
        auto tpgAgent = std::dynamic_pointer_cast<const TPGAgent>(agent);
        if(tpgAgent == nullptr){
            throw std::runtime_error("TPGManager::getAgents: an agent managed by the TPGManager is not a TPGAgent.");
        } else if (tpgAgent->isRoot()){
            constAgents.push_back(std::const_pointer_cast<const Algorithm::Agent>(agent));
        }
    }

    return constAgents;
}


std::shared_ptr<const Algorithm::Agent> Algorithm::TPG::TPGManager::createAgent(std::shared_ptr<EvoGraph::Graph> graph)
{
    std::shared_ptr<const EvoGraph::Team> vertex = graph->addNewTeam();
    return this->createAgent(vertex);
}

std::shared_ptr<const Algorithm::Agent> Algorithm::TPG::TPGManager::createAgent(std::shared_ptr<const EvoGraph::Element> element)
{
    std::shared_ptr<const EvoGraph::Team> vertex = std::dynamic_pointer_cast<const EvoGraph::Team>(element);
    if(vertex == nullptr){
        throw std::runtime_error("TPGManager::createAgent: trying to create an agent on an element from the graph that is not a team.");
    }

    this->agents.insert(std::make_shared<TPGAgent>(vertex, this->getAlgorithmName()));
    return *this->agents.rbegin();
}

std::shared_ptr<const Algorithm::Agent> Algorithm::TPG::TPGManager::copyAgent(std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<const EvoGraph::Element> element)
{
    if(!this->containsAgent(agent)) {
        throw std::runtime_error("TPGManager::copyAgent: trying to copy an agent that is not managed by this manager.");
    }

    // If element is a null pointer, dupplicate original agent's vertex.
    if(element == nullptr){
        element = graph->cloneVertex(*std::dynamic_pointer_cast<const EvoGraph::Team>(agent->getElement()));
    }

    return this->createAgent(element);
}

void Algorithm::TPG::TPGManager::deleteAgent(std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph)
{
    // Do not remove action agents from the graph
    if(auto vertex = std::dynamic_pointer_cast<const EvoGraph::Team>(agent->getElement())){
        graph->removeVertex(*vertex);
    }

    auto iterator = this->agents.find(agent);
    this->agents.erase(iterator);   
}

void Algorithm::TPG::TPGManager::setElement(std::shared_ptr<const Agent> agent, std::shared_ptr<EvoGraph::Graph> graph, std::shared_ptr<const EvoGraph::Element> element)
{
    std::shared_ptr<const EvoGraph::Team> vertex = std::dynamic_pointer_cast<const EvoGraph::Team>(element);
    if(vertex == nullptr){
        throw std::runtime_error("TPGManager::setElement: trying to set an agent on an element from the graph that is not a team.");
    }

    // Set the element
    AgentManager::setElement(agent, graph, element);
}


std::unique_ptr<Algorithm::ExecutionEngine> Algorithm::TPG::TPGManager::createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources, bool isTraining) const
{
    auto engine = std::make_unique<TPG::TPGExecutionEngine>(this->outputs, this->algorithmName, this->archive, isTraining);

    engine->setProgramExecutionEngine(
        std::move(this->cGetSubManager(this->programAlgorithmName)->createExecutionEngine(dataSources, isTraining))
    );

    return engine;
}