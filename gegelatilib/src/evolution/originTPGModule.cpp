#include "evolution/origintpgmodule.h"

Evolution::OriginTPGModule::OriginTPGModule(
    const Representations::TPG& TPGRep,
    const Representations::Representation& memberRep, 
    Evaluation::Problem& problem,
    size_t sizePopulation, size_t nbOffspring, size_t nbTrainingIterations, uint64_t seed, size_t nbThreads, 
    bool useArchive,
    std::shared_ptr<FitnessAssignment::Assigner> assigner,
    std::shared_ptr<Selection::Selector> parentSelection ,
    std::shared_ptr<Selection::Selector> survivingSelection,
    std::shared_ptr<Reproduction::Reproducer> breeder,
    std::shared_ptr<Mutation::Mutator> mutation
): BasicModule(TPGRep, problem, sizePopulation, nbOffspring, nbTrainingIterations, 
               seed, nbThreads, assigner, parentSelection, survivingSelection, breeder, mutation),
   memberRep{memberRep}
{
    if(!Dimensions::DimensionFlow::acceptsRequirements(TPGRep.getDimensionFlow().getInputDimensions(), memberRep.getDimensionFlow().getInputDimensions())) {
        throw std::runtime_error("Evolution::BasicModule: Cannot build module with input requirements not compatible between representation and problem");
    }
    if(!memberRep.getDimensionFlow().getOutputDimension().isCompatibleWith(Dimensions::Requirement::scalar<double>())) {
        throw std::runtime_error("Evolution::BasicModule: Cannot build module with output requirements not compatible between representation and problem");
    }

    GraphBased::NodeGenerator bidNodes;

    if(useArchive) {
        this->archive = 
            std::make_unique<Evaluation::DataSet>(this->representation.getDimensionFlow().getInputDimensions(), Dimensions::Requirement::array1d<double>(1));

        // Add input metric.
        this->metricTemplate->addRequiredMetric(std::make_unique<Metrics::InputMetric>());

        
        // Add archive individual generator
        bidNodes.addGenerator(Dimensions::IndividualValueArchiveGenerator(*this->archive, this->memberRep, this->memberRep.getGenotypeGenerator(), *this->mutation));
    } else {
        // Add simple individual generator
        bidNodes.addGenerator(Dimensions::IndividualValueGenerator(this->memberRep, this->memberRep.getGenotypeGenerator(), *this->mutation));
    }
    bidNodes.addGenerator(Dimensions::NumericUniformGenerator<size_t>(0, TPGRep.getNbActions() - 1));
    this->generator = std::make_unique<GraphBased::GenotypeGenerator>(bidNodes, std::make_pair(2U, 10U), std::make_pair(2U, 2U));

} 

void Evolution::OriginTPGModule::setBasicLoggerHeader()
{
    for(std::shared_ptr<Log::Basiclogger>& defLogger: this->basicloggers) {
        defLogger->addHeaderValues({"Gen", "NbVertex", "NbMember", "Fit_Min", "Fit_Avg", "Fit_Max", "T_Gen", "T_Total"});
    }
}

Data::DataValue Evolution::OriginTPGModule::computeMemberAnswer(
    const std::vector<Data::DataValue>& inputs, std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> members)
{
    std::vector<double> answers;
    for(const std::shared_ptr<const Individual>& member: members) {
        answers.push_back(member->execute(inputs).getScalar<double>());
    }

    return Data::DataValue::array1d(answers);
}

void Evolution::OriginTPGModule::updateArchiveInputs()
{
    // Empty data set
    while(this->archive->size() > 0) {
        this->archive->removeData(0);
    }

    // Map of future metrics
    std::map<uint64_t, std::vector<Data::DataValue>> allInputMetrics;

    // For each individual evaluation
    for(const auto& [indiv, metricMap]: this->populationMetrics) {

        // Get the input metrics
        std::map<uint64_t, const Metrics::InputMetric*> inputMetrics = metricMap->getMetricValues<Metrics::InputMetric>(Metrics::InputMetric::staticHash());

        // For each feature, get the input metric
        for(const auto& [featureHash, inputMetric]: inputMetrics) {

            // For each step in the evaluation feature hash, get the inputs
            for(const std::vector<Data::DataValue>& inputsMetricStep: inputMetric->getInputs()) {

                // Get combined hash of the inputs
                uint64_t currentInputHash = Data::DataValue::getCombinedHash(inputsMetricStep);

                // Inputs are new, dupplicate them and add them to the allInputMetrics Map!
                if(allInputMetrics.find(currentInputHash) == allInputMetrics.end()) {
                    std::vector<Data::DataValue> inputMetricStepClone;
                    for(const Data::DataValue& input: inputsMetricStep) {
                        inputMetricStepClone.push_back(input.clone());
                    }
                    allInputMetrics.insert(std::make_pair(currentInputHash, std::move(inputMetricStepClone)));
                }
            }
        }
    }

    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> members = this->getAllMembers();

    size_t dataSetMaxSize = 50;
    if(allInputMetrics.size() > dataSetMaxSize) {
        while(this->archive->size() != dataSetMaxSize) {

            auto it = allInputMetrics.begin();
            std::advance(it, rng.uniformSample<uint64_t>(0, allInputMetrics.size() - 1));

            this->archive->addData(std::move(it->second), this->computeMemberAnswer(it->second, members));
            allInputMetrics.erase(it);
        }
    } else {
        for(auto& [hash, inputs]: allInputMetrics) {
            this->archive->addData(std::move(inputs), this->computeMemberAnswer(inputs, members));
        }
    }
}

void Evolution::OriginTPGModule::doGeneration(size_t generationNumber) 
{

    GraphBased::NodeGenerator bidNodes;
    if(this->archive != nullptr) {
        // Add archive individual generator
        bidNodes.addGenerator(Dimensions::IndividualValueArchiveGenerator(*this->archive, this->memberRep, this->memberRep.getGenotypeGenerator(), *this->mutation));
    } else {
        // Add simple individual generator
        bidNodes.addGenerator(Dimensions::IndividualValueGenerator(this->memberRep, this->memberRep.getGenotypeGenerator(), *this->mutation));
    }

    // Action Generator
    Dimensions::NumericUniformGenerator<size_t> actionGenerator(0, dynamic_cast<const Representations::TPG&>(this->representation).getNbActions() - 1);
    // Tangled generator
    currentMemberForTangled.clear();
    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> vertices = this->getAllVertices();
    currentMemberForTangled.insert(currentMemberForTangled.begin(), vertices.begin(), vertices.end());
    Dimensions::ListUniformGenerator<std::shared_ptr<const Individual>> tangledGenerator(currentMemberForTangled);

    // Multi generator
    Dimensions::MultiGenerator multi;
    multi.addGenerator(actionGenerator, 0.975);
    multi.addGenerator(tangledGenerator, 0.025);

    bidNodes.addGenerator(multi);
    this->generator = std::make_unique<GraphBased::GenotypeGenerator>(bidNodes, std::make_pair(2U, 10U), std::make_pair(2U, 2U));

    BasicModule::doGeneration(generationNumber);

    if(this->archive != nullptr) {
        this->updateArchiveInputs();
        std::cout<<"\narchive size: "<<this->archive->size()<<std::endl;
    }
}

std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> Evolution::OriginTPGModule::getAllVertices()
{
    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> vertices(population);
    std::vector<std::shared_ptr<const Individual>> toControl;
    toControl.insert(toControl.begin(), this->population.begin(), this->population.end());

    while(!toControl.empty()) {
        std::shared_ptr<const Individual> currentIndiv = *toControl.begin();
        std::vector<std::reference_wrapper<const GraphBased::GPNode>> nodesIndiv = currentIndiv->getGenotype().getNodeGroup(0).getNodes();
        for(const GraphBased::GPNode& node: nodesIndiv) {
            if(node.getValue(1).getElementType() == typeid(std::shared_ptr<const Individual>)) {
                std::shared_ptr<const Individual> vertex = node.getValue(1).getScalar<std::shared_ptr<const Individual>>();
                if(vertices.find(vertex) == vertices.end()) {
                    vertices.insert(vertex);
                    toControl.push_back(vertex);
                }
            }
        }

        toControl.erase(toControl.begin());
    }

    return std::move(vertices);
}

std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> Evolution::OriginTPGModule::getMembersOf(std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> individuals)
{
    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> members;
    for(const std::shared_ptr<const Individual>& indiv: individuals) {
        std::vector<std::reference_wrapper<const GraphBased::GPNode>> nodesIndiv = indiv->getGenotype().getNodeGroup(0).getNodes();
        for(const GraphBased::GPNode& node: nodesIndiv) {
            if(node.getValue(0).getElementType() == typeid(std::shared_ptr<const Individual>)) {
                members.insert(node.getValue(0).getScalar<std::shared_ptr<const Individual>>());
            }
        }
    }
    return std::move(members);
}

std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> Evolution::OriginTPGModule::getAllMembers()
{
    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> vertices = this->getAllVertices();
    return std::move(this->getMembersOf(vertices));
}

void Evolution::OriginTPGModule::setNewBasicLoggerRow(size_t generationNumber)
{
    this->stopwatch.checkpoint();
    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> vertices = this->getAllVertices();
    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> members = this->getMembersOf(vertices);
    for(std::shared_ptr<Log::Basiclogger>& defLogger: this->basicloggers) {
        defLogger->addGenerationValues<size_t>({generationNumber, vertices.size(), members.size()});
    }

    double minFitness = std::numeric_limits<double>::max();
    double maxFitness = std::numeric_limits<double>::lowest();
    double averageFitness = 0.0;
    for (const auto& individual : this->population) {
        const double fitness = this->individualFitness.at(individual);
        minFitness = std::min(minFitness, fitness);
        maxFitness = std::max(maxFitness, fitness);
        averageFitness += fitness;
    }
    averageFitness /= this->population.size();

    double generationTime = this->stopwatch.getCheckpoints().at(0);
    for(std::shared_ptr<Log::Basiclogger>& defLogger: this->basicloggers) {
        defLogger->addGenerationValues<double>({minFitness, averageFitness, maxFitness, generationTime, this->stopwatch.elapsed()});
    }
}