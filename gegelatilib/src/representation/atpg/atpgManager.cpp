
#include "representation/atpg/atpgManager.h"



void Representation::ATPG::ATPGManager::emptyAgent(const Agent& agent, EvoGraph::Graph& graph) 
{
    std::vector<std::reference_wrapper<const EvoGraph::Vertex>> verticesToDelete;
    // get vertex of agent to delete;
    const EvoGraph::Vertex& vertex = this->getTPGAgentFromCst(agent).getVertex();
    while(vertex.getOutgoingEdges().size() > 0){
        const EvoGraph::Edge& edge = vertex.getOutgoingEdges().front();

        // Remove the program on the destination of the edge if it exist.
        if(edge.getDestination().hasProgram() && edge.getDestination().getProgram().getRepresentationID() == this->actionProgramRepresentationID && 
           edge.getDestination().getIncomingEdges().size() == 1) {
            // Remove action vertex from the graph because it is only used by this team and it contains an action program.
            verticesToDelete.push_back(edge.getDestination());
        }
        // Remove the edge
        graph.removeEdge(vertex.getOutgoingEdges().front());

    }

    // Remove the action vertices from the graph
    for(const EvoGraph::Vertex& vertex: verticesToDelete){
        graph.removeVertex(vertex);
    }
}

std::unique_ptr<Representation::ExecutionEngine> Representation::ATPG::ATPGManager::createExecutionEngine(std::vector<std::reference_wrapper<const Data::DataHandler>> dataSources, bool isTraining) const
{
    auto engine = std::make_unique<ATPG::ATPGExecutionEngine>(this->outputs, this->representationID, isTraining);

    engine->setProgramExecutionEngine(
        std::move(this->cGetSubManager(this->programRepresentationID).createExecutionEngine(dataSources, isTraining))
    );

    engine->setActionProgramExecutionEngine(
        std::move(this->cGetSubManager(this->actionProgramRepresentationID).createExecutionEngine(dataSources, isTraining))
    );

    return engine;
}