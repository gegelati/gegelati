
#include "algorithm/atpg/atpgManager.h"



void Algorithm::ATPG::ATPGManager::emptyAgent(const Agent& agent, std::shared_ptr<EvoGraph::Graph> graph) 
{
    std::vector<std::shared_ptr<const EvoGraph::Vertex>> verticesToDelete;
    // get vertex of agent to delete;
    auto vertex = this->getTPGAgentFromCst(agent)->getVertex();
    while(vertex->getOutgoingEdges().size() > 0){
        auto& edge = vertex->getOutgoingEdges().front();

        // Remove the program on the destination of the edge if it exist.
        std::shared_ptr<const Algorithm::Agent> locked = edge->getDestination()->getProgram().lock();
        if(locked && locked->getAlgorithmName() == this->actionProgramAlgorithmName && 
           edge->getDestination()->getIncomingEdges().size() == 1) {
            // Remove action vertex from the graph because it is only used by this team and it contains an action program.
            verticesToDelete.push_back(edge->getDestination());
        }
        // Remove the edge
        graph->removeEdge(*vertex->getOutgoingEdges().front());

    }

    // Remove the action vertices from the graph
    for(auto& vertex: verticesToDelete){
        graph->removeVertex(*vertex);
    }
}

std::unique_ptr<Algorithm::ExecutionEngine> Algorithm::ATPG::ATPGManager::createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources, bool isTraining) const
{
    auto engine = std::make_unique<ATPG::ATPGExecutionEngine>(this->outputs, this->algorithmName, isTraining);

    engine->setProgramExecutionEngine(
        std::move(this->cGetSubManager(this->programAlgorithmName)->createExecutionEngine(dataSources, isTraining))
    );

    engine->setActionProgramExecutionEngine(
        std::move(this->cGetSubManager(this->actionProgramAlgorithmName)->createExecutionEngine(dataSources, isTraining))
    );

    return engine;
}