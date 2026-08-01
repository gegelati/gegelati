
#include "representation/representation.h"
#include "file/graphDotExporter.h"
#include "file/graphDotImporter.h"
#include "codeGen/codeGenerationExporter.h"

// Declaration of static individual ID Counter in local here because it creates
// error in the .h file for MSVC compiler See:
// https://discourse.cmake.org/t/exporting-a-static-data-member-of-a-class-for-dll-using-msvc/5892
static uint64_t REPRESENTATION_COUNTER_ID = 0;

uint64_t Representation::Representation::incrementeCounter()
{
    return REPRESENTATION_COUNTER_ID++;
}

uint64_t Representation::Representation::getRepresentationIDCounter()
{
    return REPRESENTATION_COUNTER_ID;
}

void Representation::Representation::resetRepresentationIDCounter()
{
    REPRESENTATION_COUNTER_ID = 0;
}

uint64_t Representation::Representation::getRepresentationID() const
{
    return this->representationID;
}

void Representation::Representation::setRepresentationID(uint64_t newID)
{
    this->representationID = newID;

    // Update the ID counter if needed
    if (newID >= REPRESENTATION_COUNTER_ID) {
        REPRESENTATION_COUNTER_ID = newID + 1;
    }
}

Representation::Representation& Representation::Representation::getSubRepresentation(uint64_t representationID)
{
    for (auto& subRepresentation : this->subRepresentations) {
        if (subRepresentation->getRepresentationID() == representationID) {
            return *subRepresentation;
        }
    }
    throw std::runtime_error("No sub-representation with id " + std::to_string(representationID) + " found.");
}
const Representation::Representation& Representation::Representation::cGetSubRepresentation(uint64_t representationID) const
{
    for (auto& subRepresentation : this->subRepresentations) {
        if (subRepresentation->getRepresentationID() == representationID) {
            return *subRepresentation;
        }
    }
    throw std::runtime_error("No sub-representation with id " + std::to_string(representationID) + " found.");
}

void Representation::Representation::addSubRepresentation(const Representation& subRepresentation)
{
    // Throw if the sub-representation is already present
    for (auto& existingSubRepresentation : this->subRepresentations) {
        if (existingSubRepresentation->getRepresentationID() == subRepresentation.getRepresentationID()) {
            throw std::runtime_error("Representation::addSubRepresentation: Sub-representation with name " + std::to_string(subRepresentation.getRepresentationID()) + " is already present.");
        }
    }
    this->subRepresentations.push_back(subRepresentation.copy());
}

void Representation::Representation::addAggregatedRepresentation(const Representation& aggregatedRepresentation){

    // Check that the types are the same
    if(typeid(*this) != typeid(aggregatedRepresentation)){
        throw std::runtime_error("Representation::addRepresentationAccess: Cannot add access to an representation of a different type.");
    }

    this->aggregatedRepresentations.push_back(aggregatedRepresentation);
}

const EvoGraph::Graph& Representation::Representation::getGraph() const
{
    return *this->graph;
}

const Representation::Population& Representation::Representation::getPopulationCst() const
{
    return *this->population;
}

const Selector::Selector& Representation::Representation::getSelectorCst() const
{
    if(!this->hasSelector()) {
        throw std::runtime_error("Representation::getSelector: Selector is not set");
    }
    return *this->selector;
}

Representation::Population& Representation::Representation::getPopulation()
{
    return *this->population;
}


bool Representation::Representation::hasSelector() const
{
    return this->selector.has_value();
}

void Representation::Representation::setSelector(Selector::Selector& selector)
{
    this->selector = selector;
}

Selector::Selector& Representation::Representation::getSelector()
{
    if(!this->hasSelector()) {
        throw std::runtime_error("Representation::getSelector: Selector is not set");
    }
    return *this->selector;
}

Representation::Mutator& Representation::Representation::getMutator()
{
    return *this->mutator;
}

std::vector<std::reference_wrapper<const Representation::Representation>> Representation::Representation::cGetSubRepresentations() const
{
    std::vector<std::reference_wrapper<const Representation>> subRepresentationsRef;
    for(const auto& subRepresentation : this->subRepresentations){
        subRepresentationsRef.push_back(*subRepresentation);
    }
    return subRepresentationsRef;
}

std::vector<std::reference_wrapper<Representation::Representation>> Representation::Representation::getSubRepresentations()
{
    std::vector<std::reference_wrapper<Representation>> subRepresentationsRef;
    for(auto& subRepresentation : this->subRepresentations){
        subRepresentationsRef.push_back(*subRepresentation);
    }
    return subRepresentationsRef;
}

const std::vector<std::reference_wrapper<const Representation::Representation>>& Representation::Representation::getAggregatedRepresentations() const
{
    return this->aggregatedRepresentations;
}


const Representation::Representation& Representation::Representation::getAggregatedRepresentation(uint64_t representationID) const
{
    for (const Representation& aggrRepresentation : this->aggregatedRepresentations) {
        if (aggrRepresentation.getRepresentationID() == representationID) {
            return aggrRepresentation;
        }
    }
    throw std::runtime_error("No aggregated-representation with id " + std::to_string(representationID) + " found.");
}

size_t Representation::Representation::getNbIndividuals() const
{
    return this->population->getIndividuals().size();   
}

const std::vector<std::reference_wrapper<const Representation::Individual>> Representation::Representation::getIndividuals() const
{
    return this->population->getIndividuals();
}

bool Representation::Representation::containsIndividual(const Individual& individual) const
{
    return this->population->containsIndividual(individual);
}

void Representation::Representation::initSelector()
{
    if(!this->hasSelector()) {
        this->savedDefaultSelector = std::move(Selector::selectorFactory());
        this->setSelector(*this->savedDefaultSelector);
    }
    this->getSelector().setPopulation(*this->population);
    this->getSelector().setNbIndividuals(this->params->nbIndividuals);
}

void Representation::Representation::initSubRepresentations(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph) {
    /*This method is not abstract, it is not necessary for an representation to have sub representations*/
}

void Representation::Representation::initRepresentation(RNG::RNG& rng, const Output::OutputHandler& outputs, const std::vector<std::reference_wrapper<const Data::DataHandler>>& dataSource, std::shared_ptr<EvoGraph::Graph> graph)
{
    this->outputs = std::make_unique<Output::OutputHandler>(outputs);
    this->graph = graph;

    this->initPopulation();

    this->initSelector();

    this->initMutator();

    this->initSubRepresentations(rng, outputs, dataSource, graph);

    // Add the aggregated representation
    for(const Representation& aggregatedRepresentation: this->aggregatedRepresentations){
        if(!aggregatedRepresentation.isInit()) {
            throw std::runtime_error("Aggregated representation is not initialize yet. It should be.");
        }
        this->population->addAggregatedPopulation(aggregatedRepresentation.getPopulationCst());
    }

    // Clear the best individual in the selector
    this->getSelector().forgetPreviousResults();

    this->init = true;
}

bool Representation::Representation::isInit() const{
    return init;
}

const Output::OutputHandler& Representation::Representation::getOutputs() const
{
    return *this->outputs;
}

const Representation::RepresentationParameters& Representation::Representation::getParams() const
{
    return *this->params;
}

void Representation::Representation::clearRepresentation()
{
    for(const auto& subRepresentation: subRepresentations){
        subRepresentation->clearRepresentation();
    }
    if(this->population != nullptr) {
        this->population->clearIndividuals(*this->graph);
    }
    this->mutator = nullptr;
    this->selector = std::nullopt;
    this->savedDefaultSelector = nullptr;
    this->population = nullptr;
    this->init = false;
}

void Representation::Representation::initPopulation(RNG::RNG& rng)
{
    // Initialize a random population
    this->mutator->initRandomPopulation(*this->graph, *this->population,
                                        *this->params, rng);
}

void Representation::Representation::populate(RNG::RNG& rng, size_t maxNbThreads)
{
    this->mutator->mutatePopulation(*this->graph, *this->population, *this->params, rng, maxNbThreads);
    this->getSelector().updateAfterPopulate(*graph);

    this->clearUnusedSubIndividuals();
}

std::map<uint64_t, std::set<std::reference_wrapper<const Representation::Individual>>> Representation::Representation::getUsedSubIndividuals() const {
    // By default, return an empty map, meaning that no sub-individual is used by the representation.
    if(this->subRepresentations.size() > 0){
        throw std::runtime_error("Representation::getUsedSubIndividuals: This method should be override by representations with sub-representations to return the used sub-individuals.");
    }
    return std::map<uint64_t, std::set<std::reference_wrapper<const Individual>>>();
}

void Representation::Representation::clearUnusedSubIndividuals() {
    std::map<uint64_t, std::set<std::reference_wrapper<const Individual>>> usedSubIndividuals = this->getUsedSubIndividuals();
    
    for(const auto& subRepresentation: subRepresentations){
        subRepresentation->clearUnusedSubIndividuals();

        auto subRepresentationIndividuals = subRepresentation->getPopulation().getIndividuals();
        for(const Individual& individual: subRepresentationIndividuals){
            if(usedSubIndividuals[subRepresentation->getRepresentationID()].find(individual) == usedSubIndividuals[subRepresentation->getRepresentationID()].end()){
                subRepresentation->population->deleteIndividual(individual, *this->graph);
            }
        }
    }
}



std::shared_ptr<Representation::Job> Representation::Representation::createJob(const Individual& individual, Learn::LearningMode mode,  RNG::RNG& rng, int idx) const
{
    if (!this->containsIndividual(individual)) {
        throw std::runtime_error("LearningAgent::makeJob: Cannot create a job with an invalid individual or an individual not belonging to this representation.");
    }

    return std::make_shared<Job>(individual, idx);
}

void Representation::Representation::updateAfterEvaluation(const std::vector<std::shared_ptr<Job>>& jobs, Learn::LearningMode mode)
{
    // By default, do nothing
}

void Representation::Representation::linkIndividualVertex(const Individual& individual, const EvoGraph::Vertex& vertex)
{
    throw std::runtime_error("Representation::linkIndividualVertex: This method should not be called without being override by the specific representation.");
}

void Representation::Representation::exportBestIndividualCodeGen(const std::string& filename, const std::string& path)
{
    const auto& pair = this->getSelector().getBestIndividual();
    if(pair.first) {
        this->exportSpecificIndividualCodeGen(*pair.first, filename, path);
    } else {
        throw std::runtime_error("Representation::exportBestIndividualCodeGen: no best individual set.");
    }
}

void Representation::Representation::exportSpecificIndividualCodeGen(const Individual& individual, const std::string& filename, const std::string& path)
{
    if(this->containsIndividual(individual)) {
        std::string isDash = (filename == "") ? "_" : "";
        std::string filenameAlgo = filename + isDash + this->representationName + std::to_string(this->representationID);

        std::map<uint64_t, std::set<std::reference_wrapper<const Individual>>> subIndividuals;
        std::vector<std::string> subAlgoNames;
        for(const auto& subAlgo: this->subRepresentations) {
            subIndividuals.insert({subAlgo->getRepresentationID(), {}});
            subAlgoNames.push_back(filename + isDash + subAlgo->getRepresentationName() + std::to_string(subAlgo->getRepresentationID()));
        }

        CodeGen::CodeGenerationExporter codeGen(filenameAlgo, subAlgoNames, path);
        codeGen.exportMainIndividual(individual, *this, subIndividuals);

        for (const auto& subAlgo : this->subRepresentations) {
            subAlgo->exportSpecificIndividualsCodeGen(subIndividuals.at(subAlgo->getRepresentationID()), filename, path);
        }

    } else {
        throw std::runtime_error("Representation::exportSpecificIndividualCodeGen: unknown individual.");
    }
}

void Representation::Representation::exportSpecificIndividualsCodeGen(std::set<std::reference_wrapper<const Individual>> individuals,const std::string& filename, const std::string& path)
{
    for(const Individual& individual: individuals) {
        if(!this->containsIndividual(individual)) {
            throw std::runtime_error("Representation::exportSpecificIndividualsCodeGen: unknown individual.");
        }
    }
    std::string isDash = (filename == "") ? "_" : "";
    std::string filenameAlgo = filename + isDash + this->representationName + std::to_string(this->representationID);

    std::map<uint64_t, std::set<std::reference_wrapper<const Individual>>> subIndividuals;
    std::vector<std::string> subAlgoNames;
    for(const auto& subAlgo: this->subRepresentations) {
        subIndividuals.insert({subAlgo->getRepresentationID(), {}});
            subAlgoNames.push_back(filename + isDash + subAlgo->getRepresentationName() + std::to_string(subAlgo->getRepresentationID()));
    }

    CodeGen::CodeGenerationExporter codeGen(filenameAlgo, subAlgoNames, path);
    codeGen.exportIndividuals(individuals, *this, subIndividuals);

    for (const auto& subAlgo : this->subRepresentations) {
        subAlgo->exportSpecificIndividualsCodeGen(subIndividuals.at(subAlgo->getRepresentationID()), filename, path);
    }
}



void Representation::Representation::exportDotFile(const char* filePath)
{
    File::GraphDotExporter exporter;
    exporter.exportRepresentation(filePath, *this);
}

void Representation::Representation::exportBestIndividualDotFile(const char* filePath)
{
    const auto& pair = this->getSelector().getBestIndividual();
    if(pair.first) {
        this->exportSpecificIndividualDotFile(*pair.first, filePath);
    } else {
        throw std::runtime_error("Representation::exportBestIndividualDotFile: no best individual set.");
    }
}

void Representation::Representation::exportSpecificIndividualDotFile(const Individual& individual, const char* filePath)
{
    if(this->containsIndividual(individual)) {
        File::GraphDotExporter exporter;
        exporter.exportIndividual(filePath, individual, *this);
    } else {
        throw std::runtime_error("Representation::exportSpecificIndividualDotFile: unknown individual.");
    }
}

                            
void Representation::Representation::importDotFile(const char* filePath)
{
    if(!this->init) {
        throw std::runtime_error("Representation::importDotFile: Importing should be done only after initialisation is done. TODO CHANGE");
    }
    File::GraphDotImporter importer(*this->graph, *this);
    importer.importGraph(filePath);
}


bool Representation::operator<(const Representation& a, const Representation& b)
{
    return a.getRepresentationID() < b.getRepresentationID();
}
bool Representation::operator==(const Representation& a, const Representation& b)
{
    return a.getRepresentationID() == b.getRepresentationID();
}
bool Representation::operator!=(const Representation& a, const Representation& b)
{
    return a.getRepresentationID() != b.getRepresentationID();
}