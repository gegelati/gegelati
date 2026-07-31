#include "representation/tpg/tpgRepresentation.h"

std::unique_ptr<Representation::Representation> Representation::TPG::TPGRepresentation::copy() const
{
    return std::make_unique<TPGRepresentation>(this->cGetSubRepresentation(this->programRepresentationID), std::make_unique<RepresentationParameters>(*this->params), this->representationName, this->representationColor);
}

void Representation::TPG::TPGRepresentation::setProgramRepresentation(const Representation& programRepresentation)
{
    Representation::Representation::addSubRepresentation(programRepresentation);

    // Set program representation name
    this->programRepresentationID = this->subRepresentations.back()->getRepresentationID();
}


const Representation::TPG::TPGArchive& Representation::TPG::TPGRepresentation::getArchive() const
{
    return *this->archive;
}

void Representation::TPG::TPGRepresentation::initManager()
{
    this->manager = std::make_unique<TPG::TPGManager>(*this->outputs, this->representationID);
}

void Representation::TPG::TPGRepresentation::initMutator()
{
    this->mutator = std::make_unique<TPG::TPGMutator>(*this->selector, this->representationID, *this->archive);
}

void Representation::TPG::TPGRepresentation::initSubRepresentations(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph)
{
    // Initialize program representation.
    Representation& programAlgo = this->getSubRepresentation(this->programRepresentationID);

    // Program output is only size 1, except for continuous outputs where we create more outputs (one per continuous output of the TPG)
    auto programOutput = std::make_shared<Output::OutputHandler>(Output::Output());
    for(size_t idx = 0; idx < this->outputs->sizeContinuous(); idx++){
        programOutput->addOutput(Output::Output());
    }

    // Init program representation
    programAlgo.initRepresentation(rng, *programOutput, dataSource, graph);

    // Add program manager and mutator to TPG manager and mutator
    this->manager->addSubManager(programAlgo.getManager());
    TPGManager* tpgManager = dynamic_cast<TPGManager*>(this->manager.get());
    tpgManager->setProgramRepresentationID(this->programRepresentationID);

    this->mutator->addSubMutator(programAlgo.getMutator());
    TPGMutator* tpgMutator = dynamic_cast<TPGMutator*>(this->mutator.get());
    tpgMutator->setProgramRepresentationID(this->programRepresentationID);
}

std::shared_ptr<Representation::Job> Representation::TPG::TPGRepresentation::createJob(const Agent& agent, Learn::LearningMode mode, RNG::RNG& rng, int idx) const
{
    if(!this->containsAgent(agent)){
        throw std::runtime_error("LearningAgent::makeJob: Cannot create a job with a null agent or an agent not belonging to this representation.");
    }

    // Before each agent evaluation, set a new seed for the archive in
    // TRAINING Mode Else, archiving should be deactivate anyway
    std::unique_ptr<TPGArchive> jobArchive = nullptr;
    if (mode == Learn::LearningMode::TRAINING) {
        size_t archiveSeed = rng.getUnsignedInt64(0, UINT64_MAX);
        jobArchive = std::make_unique<TPGArchive>(this->params->tpg.archiveSize, this->params->tpg.archivingProbability, archiveSeed);
    }

    return std::make_shared<TPGJob>(agent, idx, std::move(jobArchive));
}

std::shared_ptr<Representation::PolicyStats> Representation::TPG::TPGRepresentation::createPolicyStats() const
{
    std::map<uint64_t, std::shared_ptr<PolicyStats>> subPolicyStatsMap;
    subPolicyStatsMap[this->programRepresentationID] = this->cGetSubRepresentation(this->programRepresentationID).createPolicyStats();
    return std::make_shared<TPGPolicyStats>(this->representationName, this->representationID, subPolicyStatsMap);
}

void Representation::TPG::TPGRepresentation::updateAfterEvaluation(const std::vector<std::shared_ptr<Job>>& jobs, Learn::LearningMode mode)
{
    // Merge the archives
    if (mode == Learn::LearningMode::TRAINING) {
        // Build archive map
        std::map<uint64_t, std::reference_wrapper<TPGArchive>> archiveMap;
        for (const auto& jobPtr : jobs) {
            std::shared_ptr<const TPGJob> tpgJob = std::dynamic_pointer_cast<const TPGJob>(jobPtr);
            if(tpgJob == nullptr){
                throw std::runtime_error("Representation::TPG::TPGRepresentation::updateAfterEvaluation trying to update after evaluation with a job which is not a TPGJob");
            }
            std::reference_wrapper<TPGArchive> archiveRef = tpgJob->getArchive();
            archiveMap.insert({jobPtr->getIdx(), archiveRef});
        }


        // Scan the archives backward, starting from the last to identify the
        // last params->representation.tpg.archiveSize recordings to keep (or less).
        auto reverseIterator = archiveMap.rbegin();

        uint64_t nbRecordings = 0;
        while (nbRecordings < this->params->tpg.archiveSize &&
            reverseIterator != archiveMap.rend()) {
            nbRecordings += reverseIterator->second.get().getNbRecordings();
            reverseIterator++;
        }

        // Insert identified recordings into this->archive
        while (reverseIterator != archiveMap.rbegin()) {
            reverseIterator--;

            auto i = reverseIterator->first;

            // Skip recordings in the first archive if needed
            uint64_t recordingIdx = 0;
            while (nbRecordings > this->params->tpg.archiveSize) {
                recordingIdx++;
                nbRecordings--;
            }

            // Insert remaining recordings
            while (recordingIdx < reverseIterator->second.get().getNbRecordings()) {
                // Access in reverse order
                const ArchiveRecording& recording =
                    reverseIterator->second.get().at(recordingIdx);
                // forced Insertion
                this->archive->addRecording(
                    *recording.agent,
                    reverseIterator->second.get().getDataHandlers().at(
                        recording.dataHash),
                    recording.result, true);
                recordingIdx++;

            }
        }
    }
}


std::map<uint64_t, std::set<std::reference_wrapper<const Representation::Agent>>> Representation::TPG::TPGRepresentation::getUsedSubAgents() const
{
    std::map<uint64_t, std::set<std::reference_wrapper<const Agent>>> usedSubAgents;
    usedSubAgents[this->programRepresentationID] = std::set<std::reference_wrapper<const Agent>>();

    for(const EvoGraph::Edge& edge: this->graph->getEdges()){
        if(edge.getProgram().getRepresentationID() == this->programRepresentationID){
            usedSubAgents[this->programRepresentationID].insert(edge.getProgram());
        }
    }
    return usedSubAgents;
}

void Representation::TPG::TPGRepresentation::printAgent(const Agent& agent, FILE* pFile, std::string offset, std::set<uint64_t>& printedAgentID, std::vector<std::reference_wrapper<const EvoGraph::Element>>& elementsToPrint) const
{
    if(printedAgentID.find(agent.getAgentID()) == printedAgentID.end() && this->containsAgent(agent)){
        printedAgentID.insert(agent.getAgentID());

        // Get vertex of the TPGAgent
        const EvoGraph::Vertex& vertex = dynamic_cast<const TPGAgent&>(agent).getVertex();
        elementsToPrint.push_back(vertex);
    
        fprintf(pFile,
                "%sP%" PRIu64 " [fillcolor=\"%s\" shape=diamond margin=0.03 "
                "width=0 height=0 label=\"%s.%" PRIu64 "\"]\n",
                offset.c_str(), agent.getAgentID(), this->representationColor.c_str(), this->representationName.c_str(), this->representationID);
    }   
}

const Representation::Agent& Representation::TPG::TPGRepresentation::readAgent(std::smatch& matches)
{
    return dynamic_cast<TPGManager&>(*this->manager).createEmptyAgent();
}

void Representation::TPG::TPGRepresentation::linkAgentVertex(const Agent& agent, const EvoGraph::Vertex& vertex)
{
    dynamic_cast<TPGManager&>(*this->manager).setVertex(agent, vertex);
}

void Representation::TPG::TPGRepresentation::printCodeGenAgents(std::ofstream& fileMain, std::ofstream& fileMainH, const std::set<std::reference_wrapper<const Agent>>& agents, std::map<uint64_t, std::set<std::reference_wrapper<const Agent>>>& subAgents) const
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
        if(auto tpgAgent = dynamic_cast<const TPGAgent*>(&agent)) {
            printedVertices.insert(tpgAgent->getVertex());
            verticesToVisit.push_back(tpgAgent->getVertex());
        } else {
            throw std::runtime_error("TPGRepresentation::printCodeGenAgents: agent should be a tpg agent");
        }
    }

    while(verticesToVisit.size() > 0) {
        const EvoGraph::Vertex& vertex = verticesToVisit.front();
        verticesToVisit.erase(verticesToVisit.begin());

        for(const EvoGraph::Edge& edge: vertex.getOutgoingEdges()) {
            if(printedVertices.find(edge.getDestination()) == printedVertices.end()) {
                printedVertices.insert(edge.getDestination());
                verticesToVisit.push_back(edge.getDestination());
            }
            subAgents.at(this->programRepresentationID).insert(edge.getProgram());
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
    fileMainH << "typedef void (* "<< programAlgo.getRepresentationName() << programAlgo.getRepresentationID() <<"_Program)(double*);\n";


    for(const Agent& agent: agents) {
        const TPGAgent& tpgAgent = dynamic_cast<const TPGAgent&>(agent);
        fileMain 
            << "void " << this->representationName << this->representationID << "_" << agent.getAgentID() << "(double* outputs) {\n"
            << "\tswitch_"<< this->representationName << this->representationID <<"(T" << tpgAgent.getVertex().getVertexID()<<", outputs);\n"
            << "}\n"
            << std::endl;
    }



    fileMain 
        << "\nvoid switch_"<< this->representationName << this->representationID << "(enum vertices currentVertex, double* outputs) {\n";

    if(this->outputs->sizeContinuous() > 0) {
        fileMain
            << "\tdouble programOutputs["<<this->outputs->sizeContinuous()+1<<"] = {";
        for(size_t idx = 0; idx < this->outputs->sizeContinuous() + 1; idx++) {
            fileMain << "0";
            if(idx < this->outputs->sizeContinuous()) {
                fileMain << ", ";
            }
        }
        fileMain
            << "};\n";
    }
    fileMain
        << "\t while(1) {\n"
        << "\t\tswitch (currentVertex) {\n";
        
    for(const EvoGraph::Vertex& vertex: printedVertices) {
        if(dynamic_cast<const EvoGraph::Team*>(&vertex)) {

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


            fileMain 
                << "\t\t\t\tdouble T" << vertex.getVertexID() << "Scores[" << nbEdge << "];\n"
                << "\n";

            size_t idx = 0;
            for(const EvoGraph::Edge& edge: vertex.getOutgoingEdges()) {
                if(this->outputs->sizeContinuous() == 0) {
                    fileMain 
                        << "\t\t\t\t"<<programAlgo.getRepresentationName() << programAlgo.getRepresentationID() << "_" << edge.getProgram().getAgentID()<< "(&T" << vertex.getVertexID() << "Scores["<<idx<<"]);\n";
                } else {

                    fileMain 
                        << "\t\t\t\t"<<programAlgo.getRepresentationName() << programAlgo.getRepresentationID() << "_" << edge.getProgram().getAgentID()<< "(&programOutputs);\n"
                        << "\t\t\t\tT" << vertex.getVertexID() << "Scores["<<idx<<"] = programOutputs[0];\n";
                }
                idx ++;
            }

            fileMain
                << "\t\t\t\tint best = bestProgram_" << this->representationName <<this->representationID << "(T" << vertex.getVertexID() << "Scores, " << nbEdge <<");\n"
                << "\t\t\t\tcurrentVertex = next[best];\n\n";

            if(this->outputs->sizeContinuous() > 1) {

                

                fileMain    
                    << "\t\t\t\tif(currentVertex == A0) {\n" 
                    << "\t\t\t\t\t" << programAlgo.getRepresentationName() << programAlgo.getRepresentationID() <<"_Program programs[" << nbEdge << "] = {"; 
                for(const EvoGraph::Edge& edge: vertex.getOutgoingEdges()) {
                    fileMain << programAlgo.getRepresentationName() << programAlgo.getRepresentationID() << "_" << edge.getProgram().getAgentID() << ", ";
                }
                fileMain
                    << "};\n"
                    << "\t\t\t\t\tprograms[best](programOutputs);\n";
                    


                for(int idxAction = 0; idxAction < this->outputs->sizeContinuous(); idxAction++ ) {
                    fileMain << "\t\t\t\t\toutputs["<<idxAction<<"] = programOutputs["<<idxAction + 1<<"];\n";
                }
                    
                fileMain
                    << "\t\t\t\t\treturn;\n"
                    << "\t\t\t\t}\n";
            }

            fileMain
                << "\t\t\t\tbreak;\n"
                << "\t\t\t}\n";

        // If discrete environment
        } else if(this->outputs->sizeDiscrete() > 0) {

            fileMain 
                << "\t\t\tcase A" << vertex.getVertexID() <<": {\n"
                << "\t\t\t\toutputs[0] = " << dynamic_cast<const EvoGraph::Action&>(vertex).getActionID() << ";\n"
                << "\t\t\t\treturn;\n"
                << "\t\t\t}\n";
        }
    }

    fileMain 
        << "\t\t}\n"
        << "\t}\n"
        << "}"<<std::endl;
}