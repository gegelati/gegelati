#include "algorithm/tpg/tpgAlgorithm.h"

std::unique_ptr<Algorithm::Algorithm> Algorithm::TPG::TPGAlgorithm::copy() const
{
    return std::make_unique<TPGAlgorithm>(this->params, this->cGetSubAlgorithm(this->programAlgorithmID), this->algorithmName);
}

void Algorithm::TPG::TPGAlgorithm::setProgramAlgorithm(const Algorithm& programAlgorithm)
{
    Algorithm::Algorithm::addSubAlgorithm(programAlgorithm);

    // Set program algorithm name
    this->programAlgorithmID = this->subAlgorithms.back()->getAlgorithmID();
}


const Archive& Algorithm::TPG::TPGAlgorithm::getArchive() const
{
    return *this->archive;
}

void Algorithm::TPG::TPGAlgorithm::initManager()
{
    this->manager = std::make_unique<TPG::TPGManager>(*this->outputs, this->algorithmID);
}

void Algorithm::TPG::TPGAlgorithm::initMutator()
{
    this->mutator = std::make_unique<TPG::TPGMutator>(*this->selector, this->algorithmID, *this->archive);
}

void Algorithm::TPG::TPGAlgorithm::initSubAlgorithms(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph)
{
    // Initialize program algorithm.
    Algorithm& programAlgo = this->getSubAlgorithm(this->programAlgorithmID);

    // Program output is only size 1, except for continuous outputs where we create more outputs (one per continuous output of the TPG)
    auto programOutput = std::make_shared<Output::OutputHandler>(Output::Output());
    for(size_t idx = 0; idx < this->outputs->sizeContinuous(); idx++){
        programOutput->addOutput(Output::Output());
    }

    // Init program algorithm
    programAlgo.initAlgorithm(rng, *programOutput, dataSource, graph);

    // Add program manager and mutator to TPG manager and mutator
    this->manager->addSubManager(programAlgo.getManager());
    TPGManager* tpgManager = dynamic_cast<TPGManager*>(this->manager.get());
    tpgManager->setProgramAlgorithmID(this->programAlgorithmID);

    this->mutator->addSubMutator(programAlgo.getMutator());
    TPGMutator* tpgMutator = dynamic_cast<TPGMutator*>(this->mutator.get());
    tpgMutator->setProgramAlgorithmID(this->programAlgorithmID);
}

std::shared_ptr<Algorithm::Job> Algorithm::TPG::TPGAlgorithm::createJob(const Agent& agent, Learn::LearningMode mode, RNG::RNG& rng, int idx) const
{
    if(!this->containsAgent(agent)){
        throw std::runtime_error("LearningAgent::makeJob: Cannot create a job with a null agent or an agent not belonging to this algorithm.");
    }

    // Before each agent evaluation, set a new seed for the archive in
    // TRAINING Mode Else, archiving should be deactivate anyway
    std::unique_ptr<Archive> jobArchive = nullptr;
    if (mode == Learn::LearningMode::TRAINING) {
        size_t archiveSeed = rng.getUnsignedInt64(0, UINT64_MAX);
        jobArchive = std::make_unique<Archive>(this->params.archiveSize, this->params.archivingProbability, archiveSeed);
    }

    return std::make_shared<TPGJob>(agent, idx, std::move(jobArchive));
}

std::shared_ptr<Algorithm::PolicyStats> Algorithm::TPG::TPGAlgorithm::createPolicyStats() const
{
    std::map<uint64_t, std::shared_ptr<PolicyStats>> subPolicyStatsMap;
    subPolicyStatsMap[this->programAlgorithmID] = this->cGetSubAlgorithm(this->programAlgorithmID).createPolicyStats();
    return std::make_shared<TPGPolicyStats>(this->algorithmName, this->algorithmID, subPolicyStatsMap);
}

void Algorithm::TPG::TPGAlgorithm::updateAfterEvaluation(const std::vector<std::shared_ptr<Job>>& jobs, Learn::LearningMode mode)
{
    // Merge the archives
    if (mode == Learn::LearningMode::TRAINING) {
        // Build archive map
        std::map<uint64_t, std::reference_wrapper<Archive>> archiveMap;
        for (const auto& jobPtr : jobs) {
            std::shared_ptr<const TPGJob> tpgJob = std::dynamic_pointer_cast<const TPGJob>(jobPtr);
            if(tpgJob == nullptr){
                throw std::runtime_error("Algorithm::TPG::TPGAlgorithm::updateAfterEvaluation trying to update after evaluation with a job which is not a TPGJob");
            }
            std::reference_wrapper<Archive> archiveRef = tpgJob->getArchive();
            archiveMap.insert({jobPtr->getIdx(), archiveRef});
        }


        // Scan the archives backward, starting from the last to identify the
        // last params.archiveSize recordings to keep (or less).
        auto reverseIterator = archiveMap.rbegin();

        uint64_t nbRecordings = 0;
        while (nbRecordings < this->params.archiveSize &&
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
            while (nbRecordings > this->params.archiveSize) {
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


std::map<uint64_t, std::set<std::reference_wrapper<const Algorithm::Agent>>> Algorithm::TPG::TPGAlgorithm::getUsedSubAgents() const
{
    std::map<uint64_t, std::set<std::reference_wrapper<const Agent>>> usedSubAgents;
    usedSubAgents[this->programAlgorithmID] = std::set<std::reference_wrapper<const Agent>>();

    for(const EvoGraph::Edge& edge: this->graph->getEdges()){
        if(edge.getProgram().getAlgorithmID() == this->programAlgorithmID){
            usedSubAgents[this->programAlgorithmID].insert(edge.getProgram());
        }
    }
    return usedSubAgents;
}

void Algorithm::TPG::TPGAlgorithm::printAgent(const Agent& agent, FILE* pFile, std::string offset, std::set<uint64_t>& printedAgentID, std::vector<std::reference_wrapper<const EvoGraph::Element>>& elementsToPrint) const
{
    if(printedAgentID.find(agent.getAgentID()) == printedAgentID.end() && this->containsAgent(agent)){
        printedAgentID.insert(agent.getAgentID());

        // Get vertex of the TPGAgent
        const EvoGraph::Vertex& vertex = dynamic_cast<const TPGAgent&>(agent).getVertex();
        elementsToPrint.push_back(vertex);
    
        fprintf(pFile,
                "%sP%" PRIu64 " [fillcolor=\"%s\" shape=diamond margin=0.03 "
                "width=0 height=0 label=\"%s.%" PRIu64 "\"]\n",
                offset.c_str(), agent.getAgentID(), this->algorithmColor.c_str(), this->algorithmName.c_str(), this->algorithmID);
    }   
}

const Algorithm::Agent& Algorithm::TPG::TPGAlgorithm::readAgent(std::smatch& matches)
{
    return dynamic_cast<TPGManager&>(*this->manager).createEmptyAgent();
}

void Algorithm::TPG::TPGAlgorithm::linkAgentVertex(const Agent& agent, const EvoGraph::Vertex& vertex)
{
    dynamic_cast<TPGManager&>(*this->manager).setVertex(agent, vertex);
}

void Algorithm::TPG::TPGAlgorithm::printCodeGenAgents(std::ofstream& fileMain, std::ofstream& fileMainH, const std::set<std::reference_wrapper<const Agent>>& agents, std::map<uint64_t, std::set<std::reference_wrapper<const Agent>>>& subAgents) const
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
        if(auto tpgAgent = dynamic_cast<const TPGAgent*>(&agent)) {
            printedVertices.insert(tpgAgent->getVertex());
            verticesToVisit.push_back(tpgAgent->getVertex());
        } else {
            throw std::runtime_error("TPGAlgorithm::printCodeGenAgents: agent should be a tpg agent");
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
            subAgents.at(this->programAlgorithmID).insert(edge.getProgram());
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
    fileMainH << "typedef void (* "<< programAlgo.getAlgorithmName() << programAlgo.getAlgorithmID() <<"_Program)(double*);\n";


    for(const Agent& agent: agents) {
        const TPGAgent& tpgAgent = dynamic_cast<const TPGAgent&>(agent);
        fileMain 
            << "void " << this->algorithmName << this->algorithmID << "_" << agent.getAgentID() << "(double* outputs) {\n"
            << "\tswitch_"<< this->algorithmName << this->algorithmID <<"(T" << tpgAgent.getVertex().getVertexID()<<", outputs);\n"
            << "}\n"
            << std::endl;
    }



    fileMain 
        << "\nvoid switch_"<< this->algorithmName << this->algorithmID << "(enum vertices currentVertex, double* outputs) {\n";

    if(this->outputs->sizeContinuous() > 0) {
        fileMain
            << "\tdouble programOutputs["<<this->outputs->sizeContinuous()+1<<"] = {";
        for(size_t idx = 0; idx < this->outputs->sizeContinuous() > 0; idx++) {
            fileMain << "0";
            if(idx < this->outputs->sizeContinuous() - 1) {
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
                        << "\t\t\t\t"<<programAlgo.getAlgorithmName() << programAlgo.getAlgorithmID() << "_" << edge.getProgram().getAgentID()<< "(&T" << vertex.getVertexID() << "Scores["<<idx<<"]);\n";
                } else {

                    fileMain 
                        << "\t\t\t\t"<<programAlgo.getAlgorithmName() << programAlgo.getAlgorithmID() << "_" << edge.getProgram().getAgentID()<< "(&programOutputs);\n"
                        << "\t\t\t\tT" << vertex.getVertexID() << "Scores["<<idx<<"] = programOutputs[0];\n";
                }
                idx ++;
            }

            fileMain
                << "\t\t\t\tint best = bestProgram_" << this->algorithmName <<this->algorithmID << "(T" << vertex.getVertexID() << "Scores, " << nbEdge <<");\n"
                << "\t\t\t\tcurrentVertex = next[best];\n\n";

            if(this->outputs->sizeContinuous() > 1) {

                

                fileMain    
                    << "\t\t\t\tif(currentVertex == A0) {\n" 
                    << "\t\t\t\t\t" << programAlgo.getAlgorithmName() << programAlgo.getAlgorithmID() <<"_Program programs[" << nbEdge << "] = {"; 
                for(const EvoGraph::Edge& edge: vertex.getOutgoingEdges()) {
                    fileMain << programAlgo.getAlgorithmName() << programAlgo.getAlgorithmID() << "_" << edge.getProgram().getAgentID() << ", ";
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