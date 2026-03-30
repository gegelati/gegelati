
#include "algorithm/algorithm.h"
#include "file/graphDotExporter.h"
#include "file/graphDotImporter.h"
#include "codeGen/codeGenerationExporter.h"

// Declaration of static agent ID Counter in local here because it creates
// error in the .h file for MSVC compiler See:
// https://discourse.cmake.org/t/exporting-a-static-data-member-of-a-class-for-dll-using-msvc/5892
static uint64_t ALGORITHM_COUNTER_ID = 0;

uint64_t Algorithm::Algorithm::incrementeCounter()
{
    return ALGORITHM_COUNTER_ID++;
}

uint64_t Algorithm::Algorithm::getAlgorithmIDCounter()
{
    return ALGORITHM_COUNTER_ID;
}

void Algorithm::Algorithm::resetAlgorithmIDCounter()
{
    ALGORITHM_COUNTER_ID = 0;
}

uint64_t Algorithm::Algorithm::getAlgorithmID() const
{
    return this->algorithmID;
}

void Algorithm::Algorithm::setAlgorithmID(uint64_t newID)
{
    this->algorithmID = newID;

    // Update the ID counter if needed
    if (newID >= ALGORITHM_COUNTER_ID) {
        ALGORITHM_COUNTER_ID = newID + 1;
    }
}

Algorithm::Algorithm& Algorithm::Algorithm::getSubAlgorithm(uint64_t algorithmID)
{
    for (auto& subAlgorithm : this->subAlgorithms) {
        if (subAlgorithm->getAlgorithmID() == algorithmID) {
            return *subAlgorithm;
        }
    }
    throw std::runtime_error("No sub-algorithm with id " + std::to_string(algorithmID) + " found.");
}
const Algorithm::Algorithm& Algorithm::Algorithm::cGetSubAlgorithm(uint64_t algorithmID) const
{
    for (auto& subAlgorithm : this->subAlgorithms) {
        if (subAlgorithm->getAlgorithmID() == algorithmID) {
            return *subAlgorithm;
        }
    }
    throw std::runtime_error("No sub-algorithm with id " + std::to_string(algorithmID) + " found.");
}

void Algorithm::Algorithm::addSubAlgorithm(const Algorithm& subAlgorithm)
{
    // Throw if the sub-algorithm is already present
    for (auto& existingSubAlgorithm : this->subAlgorithms) {
        if (existingSubAlgorithm->getAlgorithmID() == subAlgorithm.getAlgorithmID()) {
            throw std::runtime_error("Algorithm::addSubAlgorithm: Sub-algorithm with name " + std::to_string(subAlgorithm.getAlgorithmID()) + " is already present.");
        }
    }
    this->subAlgorithms.push_back(subAlgorithm.copy());
}

void Algorithm::Algorithm::addAggregatedAlgorithm(const Algorithm& aggregatedAlgorithm){

    // Check that the types are the same
    if(typeid(*this) != typeid(aggregatedAlgorithm)){
        throw std::runtime_error("Algorithm::addAlgorithmAccess: Cannot add access to an algorithm of a different type.");
    }

    this->aggregatedAlgorithms.push_back(aggregatedAlgorithm);
}

const EvoGraph::Graph& Algorithm::Algorithm::getGraph() const
{
    return *this->graph;
}

const Algorithm::AgentManager& Algorithm::Algorithm::getManagerCst() const
{
    return *this->manager;
}

const Selector::Selector& Algorithm::Algorithm::getSelectorCst() const
{
    if(!this->hasSelector()) {
        throw std::runtime_error("Algorithm::getSelector: Selector is not set");
    }
    return *this->selector;
}

Algorithm::AgentManager& Algorithm::Algorithm::getManager()
{
    return *this->manager;
}


bool Algorithm::Algorithm::hasSelector() const
{
    return this->selector.has_value();
}

void Algorithm::Algorithm::setSelector(Selector::Selector& selector)
{
    this->selector = selector;
}

Selector::Selector& Algorithm::Algorithm::getSelector()
{
    if(!this->hasSelector()) {
        throw std::runtime_error("Algorithm::getSelector: Selector is not set");
    }
    return *this->selector;
}

Algorithm::Mutator& Algorithm::Algorithm::getMutator()
{
    return *this->mutator;
}

std::vector<std::reference_wrapper<const Algorithm::Algorithm>> Algorithm::Algorithm::cGetSubAlgorithms() const
{
    std::vector<std::reference_wrapper<const Algorithm>> subAlgorithmsRef;
    for(const auto& subAlgorithm : this->subAlgorithms){
        subAlgorithmsRef.push_back(*subAlgorithm);
    }
    return subAlgorithmsRef;
}

std::vector<std::reference_wrapper<Algorithm::Algorithm>> Algorithm::Algorithm::getSubAlgorithms()
{
    std::vector<std::reference_wrapper<Algorithm>> subAlgorithmsRef;
    for(auto& subAlgorithm : this->subAlgorithms){
        subAlgorithmsRef.push_back(*subAlgorithm);
    }
    return subAlgorithmsRef;
}

const std::vector<std::reference_wrapper<const Algorithm::Algorithm>>& Algorithm::Algorithm::getAggregatedAlgorithms() const
{
    return this->aggregatedAlgorithms;
}


const Algorithm::Algorithm& Algorithm::Algorithm::getAggregatedAlgorithm(uint64_t algorithmID) const
{
    for (const Algorithm& aggrAlgorithm : this->aggregatedAlgorithms) {
        if (aggrAlgorithm.getAlgorithmID() == algorithmID) {
            return aggrAlgorithm;
        }
    }
    throw std::runtime_error("No aggregated-algorithm with id " + std::to_string(algorithmID) + " found.");
}

size_t Algorithm::Algorithm::getNbAgents() const
{
    return this->manager->getAgents().size();   
}

const std::vector<std::reference_wrapper<const Algorithm::Agent>> Algorithm::Algorithm::getAgents() const
{
    return this->manager->getAgents();
}

bool Algorithm::Algorithm::containsAgent(const Agent& agent) const
{
    return this->manager->containsAgent(agent);
}

void Algorithm::Algorithm::initSelector()
{
    if(!this->hasSelector()) {
        std::cout<<"init selector"<<std::endl;
        this->savedDefaultSelector = std::move(Selector::selectorFactory());
        this->setSelector(*this->savedDefaultSelector);
    }
    this->getSelector().setManager(*this->manager);
    this->getSelector().setNbAgents(this->params.nbAgents);
}

void Algorithm::Algorithm::initSubAlgorithms(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph) {
    /*This method is not abstract, it is not necessary for an algorithm to have sub algorithms*/
}

void Algorithm::Algorithm::initAlgorithm(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph)
{
    this->outputs = std::make_unique<Output::OutputHandler>(outputs);
    this->graph = graph;

    this->initManager();

    this->initSelector();

    this->initMutator();

    this->initSubAlgorithms(rng, outputs, dataSource, graph);

    // Add the aggregated algorithm
    for(const Algorithm& aggregatedAlgorithm: this->aggregatedAlgorithms){
        if(!aggregatedAlgorithm.isInit()) {
            throw std::runtime_error("Aggregated algorithm is not initialize yet. It should be.");
        }
        this->manager->addAggregatedManager(aggregatedAlgorithm.getManagerCst());
    }

    // Clear the best agent in the selector
    this->getSelector().forgetPreviousResults();

    this->init = true;
}

bool Algorithm::Algorithm::isInit() const{
    return init;
}

const Output::OutputHandler& Algorithm::Algorithm::getOutputs() const
{
    return *this->outputs;
}

const Algorithm::AlgorithmParameters& Algorithm::Algorithm::getParams() const
{
    return this->params;
}

void Algorithm::Algorithm::clearAlgorithm()
{
    for(const auto& subAlgorithm: subAlgorithms){
        subAlgorithm->clearAlgorithm();
    }
    if(this->manager != nullptr) {
        this->manager->clearAgents(*this->graph);
    }
    this->mutator = nullptr;
    this->selector = std::nullopt;
    this->savedDefaultSelector = nullptr;
    this->manager = nullptr;
    this->init = false;
}

void Algorithm::Algorithm::initPopulation(RNG::RNG& rng)
{
    // Initialize a random population
    this->mutator->initRandomPopulation(*this->graph, *this->manager,
                                        this->params, rng);
}

void Algorithm::Algorithm::populate(RNG::RNG& rng, size_t maxNbThreads)
{
    this->mutator->mutatePopulation(*this->graph, *this->manager, this->params, rng, maxNbThreads);
    this->getSelector().updateAfterPopulate(*graph);

    this->clearUnusedSubAgents();
}

std::map<uint64_t, std::set<std::reference_wrapper<const Algorithm::Agent>>> Algorithm::Algorithm::getUsedSubAgents() const {
    // By default, return an empty map, meaning that no sub-agent is used by the algorithm.
    if(this->subAlgorithms.size() > 0){
        throw std::runtime_error("Algorithm::getUsedSubAgents: This method should be override by algorithms with sub-algorithms to return the used sub-agents.");
    }
    return std::map<uint64_t, std::set<std::reference_wrapper<const Agent>>>();
}

void Algorithm::Algorithm::clearUnusedSubAgents() {
    std::map<uint64_t, std::set<std::reference_wrapper<const Agent>>> usedSubAgents = this->getUsedSubAgents();
    
    for(const auto& subAlgorithm: subAlgorithms){
        subAlgorithm->clearUnusedSubAgents();

        auto subAlgorithmAgents = subAlgorithm->getManager().getAgents();
        for(const Agent& agent: subAlgorithmAgents){
            if(usedSubAgents[subAlgorithm->getAlgorithmID()].find(agent) == usedSubAgents[subAlgorithm->getAlgorithmID()].end()){
                subAlgorithm->manager->deleteAgent(agent, *this->graph);
            }
        }
    }
}



std::shared_ptr<Algorithm::Job> Algorithm::Algorithm::createJob(const Agent& agent, Learn::LearningMode mode,  RNG::RNG& rng, int idx) const
{
    if (!this->containsAgent(agent)) {
        throw std::runtime_error("LearningAgent::makeJob: Cannot create a job with an invalid agent or an agent not belonging to this algorithm.");
    }

    return std::make_shared<Job>(agent, idx);
}

void Algorithm::Algorithm::updateAfterEvaluation(const std::vector<std::shared_ptr<Job>>& jobs, Learn::LearningMode mode)
{
    // By default, do nothing
}

void Algorithm::Algorithm::linkAgentVertex(const Agent& agent, const EvoGraph::Vertex& vertex)
{
    throw std::runtime_error("Algorithm::linkAgentVertex: This method should not be called without being override by the specific algorithm.");
}

void Algorithm::Algorithm::exportBestAgentCodeGen(const std::string& filename, const std::string& path)
{
    const auto& pair = this->getSelector().getBestAgent();
    if(pair.first) {
        this->exportSpecificAgentCodeGen(*pair.first, filename, path);
    } else {
        throw std::runtime_error("Algorithm::exportBestAgentCodeGen: no best agent set.");
    }
}

void Algorithm::Algorithm::exportSpecificAgentCodeGen(const Agent& agent, const std::string& filename, const std::string& path)
{
    if(this->containsAgent(agent)) {
        std::string isDash = (filename == "") ? "_" : "";
        std::string filenameAlgo = filename + isDash + this->algorithmName + std::to_string(this->algorithmID);

        std::map<uint64_t, std::set<std::reference_wrapper<const Agent>>> subAgents;
        std::vector<std::string> subAlgoNames;
        for(const auto& subAlgo: this->subAlgorithms) {
            subAgents.insert({subAlgo->getAlgorithmID(), {}});
            subAlgoNames.push_back(filename + isDash + subAlgo->getAlgorithmName() + std::to_string(subAlgo->getAlgorithmID()));
        }

        CodeGen::CodeGenerationExporter codeGen(filenameAlgo, subAlgoNames, path);
        codeGen.exportMainAgent(agent, *this, subAgents);

        for (const auto& subAlgo : this->subAlgorithms) {
            subAlgo->exportSpecificAgentsCodeGen(subAgents.at(subAlgo->getAlgorithmID()), filename, path);
        }

    } else {
        throw std::runtime_error("Algorithm::exportSpecificAgentCodeGen: unknown agent.");
    }
}

void Algorithm::Algorithm::exportSpecificAgentsCodeGen(std::set<std::reference_wrapper<const Agent>> agents,const std::string& filename, const std::string& path)
{
    for(const Agent& agent: agents) {
        if(!this->containsAgent(agent)) {
            throw std::runtime_error("Algorithm::exportSpecificAgentsCodeGen: unknown agent.");
        }
    }
    std::string isDash = (filename == "") ? "_" : "";
    std::string filenameAlgo = filename + isDash + this->algorithmName + std::to_string(this->algorithmID);

    std::map<uint64_t, std::set<std::reference_wrapper<const Agent>>> subAgents;
    std::vector<std::string> subAlgoNames;
    for(const auto& subAlgo: this->subAlgorithms) {
        subAgents.insert({subAlgo->getAlgorithmID(), {}});
            subAlgoNames.push_back(filename + isDash + subAlgo->getAlgorithmName() + std::to_string(subAlgo->getAlgorithmID()));
    }

    CodeGen::CodeGenerationExporter codeGen(filenameAlgo, subAlgoNames, path);
    codeGen.exportAgents(agents, *this, subAgents);

    for (const auto& subAlgo : this->subAlgorithms) {
        subAlgo->exportSpecificAgentsCodeGen(subAgents.at(subAlgo->getAlgorithmID()), filename, path);
    }
}



void Algorithm::Algorithm::exportDotFile(const char* filePath)
{
    File::GraphDotExporter exporter;
    exporter.exportAlgorithm(filePath, *this);
}

void Algorithm::Algorithm::exportBestAgentDotFile(const char* filePath)
{
    const auto& pair = this->getSelector().getBestAgent();
    if(pair.first) {
        this->exportSpecificAgentDotFile(*pair.first, filePath);
    } else {
        throw std::runtime_error("Algorithm::exportBestAgentDotFile: no best agent set.");
    }
}

void Algorithm::Algorithm::exportSpecificAgentDotFile(const Agent& agent, const char* filePath)
{
    if(this->containsAgent(agent)) {
        File::GraphDotExporter exporter;
        exporter.exportAgent(filePath, agent, *this);
    } else {
        throw std::runtime_error("Algorithm::exportSpecificAgentDotFile: unknown agent.");
    }
}

                            
void Algorithm::Algorithm::importDotFile(const char* filePath)
{
    if(!this->init) {
        throw std::runtime_error("Algorithm::importDotFile: Importing should be done only after initialisation is done. TODO CHANGE");
    }
    File::GraphDotImporter importer(*this->graph, *this);
    importer.importGraph(filePath);
}


bool Algorithm::operator<(const Algorithm& a, const Algorithm& b)
{
    return a.getAlgorithmID() < b.getAlgorithmID();
}
bool Algorithm::operator==(const Algorithm& a, const Algorithm& b)
{
    return a.getAlgorithmID() == b.getAlgorithmID();
}
bool Algorithm::operator!=(const Algorithm& a, const Algorithm& b)
{
    return a.getAlgorithmID() != b.getAlgorithmID();
}