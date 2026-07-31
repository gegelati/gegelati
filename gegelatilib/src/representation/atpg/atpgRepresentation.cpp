
#include "representation/atpg/atpgRepresentation.h"


std::unique_ptr<Representation::Representation> Representation::ATPG::ATPGRepresentation::copy() const
{
    return std::make_unique<ATPGRepresentation>( 
        this->cGetSubRepresentation(this->programRepresentationID), 
        this->cGetSubRepresentation(this->actionProgramRepresentationID), 
        std::make_unique<RepresentationParameters>(*this->params),
        this->representationName, this->representationColor
    );
}

void Representation::ATPG::ATPGRepresentation::setActionProgramRepresentation(const Representation& actionProgramRepresentation)
{
    Representation::Representation::addSubRepresentation(actionProgramRepresentation);

    // Set action program representation name
    this->actionProgramRepresentationID = this->subRepresentations.back()->getRepresentationID();
}

void Representation::ATPG::ATPGRepresentation::addAggregatedActionProgramRepresentation(const Representation& programRepresentation)
{
    // Get existing action program representation
    Representation& actionProgramAlgo = this->getSubRepresentation(this->actionProgramRepresentationID);

    // Set action program representation name
    actionProgramAlgo.addAggregatedRepresentation(programRepresentation);
}


void Representation::ATPG::ATPGRepresentation::initManager()
{
    this->manager = std::make_unique<ATPG::ATPGManager>(*this->outputs, this->representationID);
}

void Representation::ATPG::ATPGRepresentation::initMutator()
{
    this->mutator = std::make_unique<ATPG::ATPGMutator>(*this->selector, this->representationID, *this->archive);
}

std::map<uint64_t, std::set<std::reference_wrapper<const Representation::Agent>>> Representation::ATPG::ATPGRepresentation::getUsedSubAgents() const
{
    std::map<uint64_t, std::set<std::reference_wrapper<const Agent>>> usedSubAgents = TPGRepresentation::getUsedSubAgents();
    usedSubAgents[this->actionProgramRepresentationID] = std::set<std::reference_wrapper<const Agent>>();

    for(const EvoGraph::Vertex& vertex: this->graph->getVertices()){
        if(vertex.hasProgram() && vertex.getProgram().getRepresentationID() == this->actionProgramRepresentationID){
            usedSubAgents[this->actionProgramRepresentationID].insert(vertex.getProgram());
        }
    }

    return usedSubAgents;
}



void Representation::ATPG::ATPGRepresentation::initSubRepresentations(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph)
{
    // Initialize program representation.
    Representation& programAlgo = this->getSubRepresentation(this->programRepresentationID);
    // Program output is only size 1
    auto programOutput = std::make_shared<Output::OutputHandler>(Output::Output());
    programAlgo.initRepresentation(rng, *programOutput, dataSource, graph);

    
    // Initialize action program representation.
    Representation& actionProgramAlgo = this->getSubRepresentation(this->actionProgramRepresentationID);

    // Program output are the same but range min/max inf, cast is done by atpg.
    Output::OutputHandler actionProgramOutput(std::vector<Output::Output>(outputs.sizeContinuous(), Output::Output()));
    if(outputs.sizeDiscrete() > 0) {
        actionProgramOutput = outputs;
    }
    actionProgramAlgo.initRepresentation(rng, actionProgramOutput, dataSource, graph);

    // Add program manager and mutator to TPG manager and mutator
    this->manager->addSubManager(actionProgramAlgo.getManager());
    this->manager->addSubManager(programAlgo.getManager());
    ATPG::ATPGManager* atpgManager = dynamic_cast<ATPGManager*>(this->manager.get());
    atpgManager->setActionProgramRepresentationID(this->actionProgramRepresentationID);
    atpgManager->setProgramRepresentationID(this->programRepresentationID);

    this->mutator->addSubMutator(actionProgramAlgo.getMutator());
    this->mutator->addSubMutator(programAlgo.getMutator());
    ATPG::ATPGMutator* atpgMutator = dynamic_cast<ATPGMutator*>(this->mutator.get());
    atpgMutator->setActionProgramRepresentationID(this->actionProgramRepresentationID);
    atpgMutator->setProgramRepresentationID(this->programRepresentationID);
}


std::shared_ptr<Representation::PolicyStats> Representation::ATPG::ATPGRepresentation::createPolicyStats() const
{
    std::map<uint64_t, std::shared_ptr<PolicyStats>> subPolicyStatsMap;
    subPolicyStatsMap[this->programRepresentationID] = this->cGetSubRepresentation(this->programRepresentationID).createPolicyStats();
    subPolicyStatsMap[this->actionProgramRepresentationID] = this->cGetSubRepresentation(this->actionProgramRepresentationID).createPolicyStats();
    return std::make_shared<ATPGPolicyStats>(this->representationName, this->representationID, subPolicyStatsMap);
}


void Representation::ATPG::ATPGRepresentation::printCodeGenAgents(std::ofstream& fileMain, std::ofstream& fileMainH, const std::set<std::reference_wrapper<const Agent>>& agents, std::map<uint64_t, std::set<std::reference_wrapper<const Agent>>>& subAgents) const
{
    fileMain
        << "int bestProgram_"<<this->representationName <<this->representationID <<"(double *results, int nb) {\n"
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
            throw std::runtime_error("TPGRepresentation::printCodeGenAgents: agent should be a tpg agent");
        }
    }

    while(verticesToVisit.size() > 0) {
        const EvoGraph::Vertex& vertex = verticesToVisit.front();
        verticesToVisit.erase(verticesToVisit.begin());

        if(vertex.hasProgram()) {
            subAgents.at(this->actionProgramRepresentationID).insert(vertex.getProgram());
        } else {
            for(const EvoGraph::Edge& edge: vertex.getOutgoingEdges()) {
                if(printedVertices.find(edge.getDestination()) == printedVertices.end()) {
                    printedVertices.insert(edge.getDestination());
                    verticesToVisit.push_back(edge.getDestination());
                }
                subAgents.at(this->programRepresentationID).insert(edge.getProgram());
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

    const Representation& programAlgo = this->cGetSubRepresentation(this->programRepresentationID);
    const Representation& actionProgramAlgo = this->cGetSubRepresentation(this->actionProgramRepresentationID);
    fileMainH << "typedef void (* "<< programAlgo.getRepresentationName() << programAlgo.getRepresentationID() <<"_Program)(double*);\n";


    for(const Agent& agent: agents) {
        const TPG::TPGAgent& tpgAgent = dynamic_cast<const TPG::TPGAgent&>(agent);
        fileMain 
            << "void " << this->representationName << this->representationID << "_" << agent.getAgentID() << "(double* outputs) {\n"
            << "\tswitch_"<< this->representationName << this->representationID <<"(T" << tpgAgent.getVertex().getVertexID()<<", outputs);\n"
            << "}\n"
            << std::endl;
    }



    fileMain 
        << "\nvoid switch_"<< this->representationName << this->representationID << "(enum vertices currentVertex, double* outputs) {\n";

    fileMain
        << "\t while(1) {\n"
        << "\t\tswitch (currentVertex) {\n";
        
    for(const EvoGraph::Vertex& vertex: printedVertices) {
        if(dynamic_cast<const EvoGraph::Team*>(&vertex) && vertex.hasProgram()) {

            fileMain 
                << "\t\t\tcase T" << vertex.getVertexID() <<": {\n"
                << "\t\t\t\t" << actionProgramAlgo.getRepresentationName() << actionProgramAlgo.getRepresentationID() << "_" << vertex.getProgram().getAgentID() << "(outputs);\n"
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

            fileMain << "\t\t\t\t" << programAlgo.getRepresentationName() << programAlgo.getRepresentationID() <<"_Program programs[" << nbEdge << "] = {"; 
            for(const EvoGraph::Edge& edge: vertex.getOutgoingEdges()) {
                fileMain << programAlgo.getRepresentationName() << programAlgo.getRepresentationID() << "_" << edge.getProgram().getAgentID() << ", ";
            }

            fileMain 
                << "};\n"
                << "\t\t\t\tdouble T" << vertex.getVertexID() << "Scores[" << nbEdge << "];\n"
                << "\n"
                << "\t\t\t\tfor(int idx = 0; idx < " << nbEdge << "; idx++) {\n"
                << "\t\t\t\t\tprograms[idx](&T" << vertex.getVertexID() << "Scores[idx]);\n"
                << "\t\t\t\t}\n"
                << "\n"
                << "\t\t\t\tint best = bestProgram_" << this->representationName <<this->representationID << "(T" << vertex.getVertexID() << "Scores, " << nbEdge <<");\n"
                << "\t\t\t\tcurrentVertex = next[best];\n\n"
                << "\t\t\t\tbreak;\n"
                << "\t\t\t}\n";

        // If discrete environment
        } else {

            throw std::runtime_error("ATPGRepresentation::printCodeGenAgents: Should not reach an action vertex");
        }
    }

    fileMain 
        << "\t\t}\n"
        << "\t}\n"
        << "}"<<std::endl;
}