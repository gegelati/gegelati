
#include "algorithm/atpg/atpgAlgorithm.h"


std::unique_ptr<Algorithm::Algorithm> Algorithm::ATPG::ATPGAlgorithm::copy() const
{
    return std::make_unique<ATPGAlgorithm>( 
        this->cGetSubAlgorithm(this->programAlgorithmID), 
        this->cGetSubAlgorithm(this->actionProgramAlgorithmID), 
        std::make_unique<AlgorithmParameters>(*this->params),
        this->algorithmName, this->algorithmColor
    );
}

void Algorithm::ATPG::ATPGAlgorithm::setActionProgramAlgorithm(const Algorithm& actionProgramAlgorithm)
{
    Algorithm::Algorithm::addSubAlgorithm(actionProgramAlgorithm);

    // Set action program algorithm name
    this->actionProgramAlgorithmID = this->subAlgorithms.back()->getAlgorithmID();
}

void Algorithm::ATPG::ATPGAlgorithm::addAggregatedActionProgramAlgorithm(const Algorithm& programAlgorithm)
{
    // Get existing action program algorithm
    Algorithm& actionProgramAlgo = this->getSubAlgorithm(this->actionProgramAlgorithmID);

    // Set action program algorithm name
    actionProgramAlgo.addAggregatedAlgorithm(programAlgorithm);
}


void Algorithm::ATPG::ATPGAlgorithm::initManager()
{
    this->manager = std::make_unique<ATPG::ATPGManager>(*this->outputs, this->algorithmID);
}

void Algorithm::ATPG::ATPGAlgorithm::initMutator()
{
    this->mutator = std::make_unique<ATPG::ATPGMutator>(*this->selector, this->algorithmID, *this->archive);
}

std::map<uint64_t, std::set<std::reference_wrapper<const Algorithm::Agent>>> Algorithm::ATPG::ATPGAlgorithm::getUsedSubAgents() const
{
    std::map<uint64_t, std::set<std::reference_wrapper<const Agent>>> usedSubAgents = TPGAlgorithm::getUsedSubAgents();
    usedSubAgents[this->actionProgramAlgorithmID] = std::set<std::reference_wrapper<const Agent>>();

    for(const EvoGraph::Vertex& vertex: this->graph->getVertices()){
        if(vertex.hasProgram() && vertex.getProgram().getAlgorithmID() == this->actionProgramAlgorithmID){
            usedSubAgents[this->actionProgramAlgorithmID].insert(vertex.getProgram());
        }
    }

    return usedSubAgents;
}



void Algorithm::ATPG::ATPGAlgorithm::initSubAlgorithms(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph)
{
    // Initialize program algorithm.
    Algorithm& programAlgo = this->getSubAlgorithm(this->programAlgorithmID);
    // Program output is only size 1
    auto programOutput = std::make_shared<Output::OutputHandler>(Output::Output());
    programAlgo.initAlgorithm(rng, *programOutput, dataSource, graph);

    
    // Initialize action program algorithm.
    Algorithm& actionProgramAlgo = this->getSubAlgorithm(this->actionProgramAlgorithmID);

    // Program output are the same but range min/max inf, cast is done by atpg.
    Output::OutputHandler actionProgramOutput(std::vector<Output::Output>(outputs.sizeContinuous(), Output::Output()));
    if(outputs.sizeDiscrete() > 0) {
        actionProgramOutput = outputs;
    }
    actionProgramAlgo.initAlgorithm(rng, actionProgramOutput, dataSource, graph);

    // Add program manager and mutator to TPG manager and mutator
    this->manager->addSubManager(actionProgramAlgo.getManager());
    this->manager->addSubManager(programAlgo.getManager());
    ATPG::ATPGManager* atpgManager = dynamic_cast<ATPGManager*>(this->manager.get());
    atpgManager->setActionProgramAlgorithmID(this->actionProgramAlgorithmID);
    atpgManager->setProgramAlgorithmID(this->programAlgorithmID);

    this->mutator->addSubMutator(actionProgramAlgo.getMutator());
    this->mutator->addSubMutator(programAlgo.getMutator());
    ATPG::ATPGMutator* atpgMutator = dynamic_cast<ATPGMutator*>(this->mutator.get());
    atpgMutator->setActionProgramAlgorithmID(this->actionProgramAlgorithmID);
    atpgMutator->setProgramAlgorithmID(this->programAlgorithmID);
}


std::shared_ptr<Algorithm::PolicyStats> Algorithm::ATPG::ATPGAlgorithm::createPolicyStats() const
{
    std::map<uint64_t, std::shared_ptr<PolicyStats>> subPolicyStatsMap;
    subPolicyStatsMap[this->programAlgorithmID] = this->cGetSubAlgorithm(this->programAlgorithmID).createPolicyStats();
    subPolicyStatsMap[this->actionProgramAlgorithmID] = this->cGetSubAlgorithm(this->actionProgramAlgorithmID).createPolicyStats();
    return std::make_shared<ATPGPolicyStats>(this->algorithmName, this->algorithmID, subPolicyStatsMap);
}


void Algorithm::ATPG::ATPGAlgorithm::printCodeGenAgents(std::ofstream& fileMain, std::ofstream& fileMainH, const std::set<std::reference_wrapper<const Agent>>& agents, std::map<uint64_t, std::set<std::reference_wrapper<const Agent>>>& subAgents) const
{
    fileMain
        << "int bestProgram_"<<this->algorithmName <<this->algorithmID <<"(double *results, int nb) {\n"
        << "\tint bestProgram = 0;\n"
        << "\tdouble bestScore = (isnan(results[0]))? -INFINITY : results[0];\n"
        << "\tfor (int i = 1; i < nb; i++) {\n"
        << "\t\tdouble challengerScore = (isnan(results[i]))? -INFINITY : "
           "results[i];\n"
        << "\t\tif (challengerScore >= bestScore) {\n"
        << "\t\t\tbestProgram = i;\n"
        << "\t\t\tbestScore = challengerScore;\n"
        << "\t\t}\n"
        << "\t}\n"
        << "\treturn bestProgram;\n"
        << "}\n"
        << std::endl;

    // set of all used vertex by the list of agents
    std::set<std::reference_wrapper<const EvoGraph::Vertex>> printedVertices;
    std::vector<std::reference_wrapper<const EvoGraph::Vertex>> verticesToVisit;
    for(const Agent& agent: agents) {
        if(auto tpgAgent = dynamic_cast<const TPG::TPGAgent*>(&agent)) {
            printedVertices.insert(tpgAgent->getVertex());
            verticesToVisit.push_back(tpgAgent->getVertex());
        } else {
            throw std::runtime_error("TPGAlgorithm::printCodeGenAgents: agent should be a tpg agent");
        }
    }

    while(verticesToVisit.size() > 0) {
        const EvoGraph::Vertex& vertex = verticesToVisit.front();
        verticesToVisit.erase(verticesToVisit.begin());

        if(vertex.hasProgram()) {
            subAgents.at(this->actionProgramAlgorithmID).insert(vertex.getProgram());
        } else {
            for(const EvoGraph::Edge& edge: vertex.getOutgoingEdges()) {
                if(printedVertices.find(edge.getDestination()) == printedVertices.end()) {
                    printedVertices.insert(edge.getDestination());
                    verticesToVisit.push_back(edge.getDestination());
                }
                subAgents.at(this->programAlgorithmID).insert(edge.getProgram());
            }
        }
    }

    fileMain << "enum vertices {";
    for(const EvoGraph::Vertex& vertex: printedVertices) {
        if(dynamic_cast<const EvoGraph::Team*>(&vertex)) {
            fileMain << "T";
        } else {
            fileMain << "A";
        }
        fileMain << vertex.getVertexID() <<", ";
    }
    fileMain << "};"<<std::endl;

    const Algorithm& programAlgo = this->cGetSubAlgorithm(this->programAlgorithmID);
    const Algorithm& actionProgramAlgo = this->cGetSubAlgorithm(this->actionProgramAlgorithmID);
    fileMainH << "typedef void (* "<< programAlgo.getAlgorithmName() << programAlgo.getAlgorithmID() <<"_Program)(double*);\n";


    for(const Agent& agent: agents) {
        const TPG::TPGAgent& tpgAgent = dynamic_cast<const TPG::TPGAgent&>(agent);
        fileMain 
            << "void " << this->algorithmName << this->algorithmID << "_" << agent.getAgentID() << "(double* outputs) {\n"
            << "\tswitch_"<< this->algorithmName << this->algorithmID <<"(T" << tpgAgent.getVertex().getVertexID()<<", outputs);\n"
            << "}\n"
            << std::endl;
    }



    fileMain 
        << "\nvoid switch_"<< this->algorithmName << this->algorithmID << "(enum vertices currentVertex, double* outputs) {\n";

    fileMain
        << "\t while(1) {\n"
        << "\t\tswitch (currentVertex) {\n";
        
    for(const EvoGraph::Vertex& vertex: printedVertices) {
        if(dynamic_cast<const EvoGraph::Team*>(&vertex) && vertex.hasProgram()) {

            fileMain 
                << "\t\t\tcase T" << vertex.getVertexID() <<": {\n"
                << "\t\t\t\t" << actionProgramAlgo.getAlgorithmName() << actionProgramAlgo.getAlgorithmID() << "_" << vertex.getProgram().getAgentID() << "(outputs);\n"
                << "\t\t\t\treturn;\n"
                << "\t\t\t}"<<std::endl;

        } else if (dynamic_cast<const EvoGraph::Team*>(&vertex)) {

            

            size_t nbEdge = vertex.getOutgoingEdges().size();

            fileMain << "\t\t\tcase T" << vertex.getVertexID() <<": {\n";
            fileMain << "\t\t\t\tconst enum vertices next[" << nbEdge << "] = {";
            for(const EvoGraph::Edge& edge: vertex.getOutgoingEdges()) {
                if(dynamic_cast<const EvoGraph::Team*>(&edge.getDestination())) {
                    fileMain <<"T"<<edge.getDestination().getVertexID() <<", ";
                } else {
                    fileMain <<"A"<<edge.getDestination().getVertexID() <<", ";
                }
            }
            fileMain << "};\n";

            fileMain << "\t\t\t\t" << programAlgo.getAlgorithmName() << programAlgo.getAlgorithmID() <<"_Program programs[" << nbEdge << "] = {"; 
            for(const EvoGraph::Edge& edge: vertex.getOutgoingEdges()) {
                fileMain << programAlgo.getAlgorithmName() << programAlgo.getAlgorithmID() << "_" << edge.getProgram().getAgentID() << ", ";
            }

            fileMain 
                << "};\n"
                << "\t\t\t\tdouble T" << vertex.getVertexID() << "Scores[" << nbEdge << "];\n"
                << "\n"
                << "\t\t\t\tfor(int idx = 0; idx < " << nbEdge << "; idx++) {\n"
                << "\t\t\t\t\tprograms[idx](&T" << vertex.getVertexID() << "Scores[idx]);\n"
                << "\t\t\t\t}\n"
                << "\n"
                << "\t\t\t\tint best = bestProgram_" << this->algorithmName <<this->algorithmID << "(T" << vertex.getVertexID() << "Scores, " << nbEdge <<");\n"
                << "\t\t\t\tcurrentVertex = next[best];\n\n"
                << "\t\t\t\tbreak;\n"
                << "\t\t\t}\n";

        // If discrete environment
        } else {

            throw std::runtime_error("ATPGAlgorithm::printCodeGenAgents: Should not reach an action vertex");
        }
    }

    fileMain 
        << "\t\t}\n"
        << "\t}\n"
        << "}"<<std::endl;
}