#include "evolution/origintpgmodule.h"


Evolution::OriginTPGModule::OriginTPGModule(
    const Representations::TPG& TPGRep,
    const Representations::Representation& memberRep, 
    Evaluation::Problem& problem,
    size_t sizePopulation, size_t nbOffspring, size_t nbTrainingIterations, uint64_t seed, size_t nbThreads, 
    size_t archiveSize, bool isArchiveFixed, bool protectAggregated,
    std::shared_ptr<FitnessAssignment::Assigner> assigner,
    std::shared_ptr<Selection::Selector> parentSelection ,
    std::shared_ptr<Selection::Selector> survivingSelection,
    std::shared_ptr<Reproduction::Reproducer> breeder,
    std::shared_ptr<Mutation::NodeTransferMutation> mutation,
    std::shared_ptr<Mutation::Mutator> memberMutation
): BasicModule(TPGRep, problem, sizePopulation, nbOffspring, nbTrainingIterations, 
               seed, nbThreads, assigner, parentSelection, survivingSelection, breeder, 
               (mutation == nullptr) ? std::make_shared<Mutation::NodeTransferMutation>(0.1, 0.7, 0.7) : mutation),
   memberRep{memberRep}, archiveSize{archiveSize}, isArchiveFixed{isArchiveFixed}, protectAggregated{protectAggregated}
{
    if(!Dimensions::DimensionFlow::acceptsRequirements(TPGRep.getDimensionFlow().getInputDimensions(), this->memberRep.getDimensionFlow().getInputDimensions())) {
        throw std::runtime_error("Evolution::OriginTPGModule: Cannot build module with input requirements not compatible between representation and problem");
    }
    if(!this->memberRep.getDimensionFlow().getOutputDimension().isCompatibleWith(Dimensions::Requirement::scalar<double>())) {
        throw std::runtime_error("Evolution::OriginTPGModule: Cannot build module with output requirements not compatible between representation and problem");
    }


    if(this->memberMutation == nullptr) {
        this->memberMutation = std::make_shared<Mutation::PointMutator>(0.2, 0.5, 0.5);
    }

    GraphBased::NodeGenerator bidNodes;

    if(this->archiveSize > 0) {
        this->archive = 
            std::make_unique<Evaluation::DataSet>(this->representation.getDimensionFlow().getInputDimensions(), Dimensions::Requirement::array1d<double>(1));

        // Add input metric.
        this->metricTemplate->addRequiredMetric(std::make_unique<Metrics::InputMetric>());

        
        // Add archive individual generator
        bidNodes.addGenerator(Dimensions::IndividualValueArchiveGenerator(*this->archive, this->memberRep, this->memberRep.getGenotypeGenerator(), *this->memberMutation));
    } else {
        // Add simple individual generator
        bidNodes.addGenerator(Dimensions::IndividualValueGenerator(this->memberRep, this->memberRep.getGenotypeGenerator(), *this->memberMutation));
    }
    bidNodes.addGenerator(Dimensions::NumericUniformGenerator<size_t>(0, TPGRep.getNbActions() - 1));
    this->generator = std::make_unique<GraphBased::GenotypeGenerator>(bidNodes, std::make_pair(2U, 20U), std::make_pair(2U, 2U));

} 

void Evolution::OriginTPGModule::setBasicLoggerHeader()
{
    for(std::shared_ptr<Log::Basiclogger>& defLogger: this->basicloggers) {
        defLogger->addHeaderValues({"Gen", "NbVertex", "NbMember", "Fit_Min", "Fit_Avg", "Fit_Max", "T_Mut", "T_Eval", "T_Gen", "T_Total"});
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

void Evolution::OriginTPGModule::updateArchive()
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
            for(std::shared_ptr<const std::vector<Data::DataValue>> inputsMetricStep: inputMetric->getInputs()) {

                // Get combined hash of the inputs
                uint64_t currentInputHash = Data::DataValue::getCombinedHash(*inputsMetricStep);

                // Inputs are new, dupplicate them and add them to the allInputMetrics Map!
                if(allInputMetrics.find(currentInputHash) == allInputMetrics.end()) {
                    std::vector<Data::DataValue> inputMetricStepClone;
                    for(const Data::DataValue& input: *inputsMetricStep) {
                        inputMetricStepClone.push_back(input.clone());
                    }
                    allInputMetrics.insert(std::make_pair(currentInputHash, std::move(inputMetricStepClone)));
                }
            }
        }
    }

    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> members = this->getAllMembers();

    if(allInputMetrics.size() > this->archiveSize) {
        while(this->archive->size() != this->archiveSize) {

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

void Evolution::OriginTPGModule::updateArchiveOutputs() {
    
    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> members = this->getAllMembers();
    for(size_t idx = 0; idx < this->archive->size(); idx++) {
        const std::vector<Data::DataValue>& inputs = this->archive->getInputsAt(idx);
        this->archive->setOutputAt(idx, this->computeMemberAnswer(inputs, members));
    }

}

void Evolution::OriginTPGModule::initialize() 
{
    Evolution::BasicModule::initialize();

    if(this->archive != nullptr) {
        this->updateArchive();
    }

    if(this->isArchiveFixed) {
        // Reset the metric template to removed the input metric for efficiency
        this->initializeMetricTemplate();
    }

}

void Evolution::OriginTPGModule::updateProtectedIndividuals(std::vector<std::shared_ptr<const Individual>> populationBeforeGen)
{
    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> offspring(this->population);
    std::vector<std::shared_ptr<const Individual>> discardedPop;
    std::vector<std::shared_ptr<const Individual>> survivingPop;
    for(const std::shared_ptr<const Individual>& indiv: populationBeforeGen) {
        if(this->population.find(indiv) != this->population.end()) {
            survivingPop.push_back(indiv);
            offspring.erase(indiv);
        } else {
            discardedPop.push_back(indiv);
        }
    }

    for(const std::shared_ptr<const Individual>& os: offspring) {
        std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> subVertices = this->getSubVerticesOf(*os);
        for(const std::shared_ptr<const Individual>& subVertex: subVertices)
        if(this->protectedIndividuals.find(subVertex) != this->protectedIndividuals.end()) {
            this->protectedIndividuals.at(subVertex).push_back(os->getIndividualID());
        }
    }

    for(const std::shared_ptr<const Individual>& indiv: survivingPop) {
        if(this->protectedIndividuals.find(indiv) != this->protectedIndividuals.end()) {
            // Individual is now protected, it is removed from the population
            this->population.erase(indiv);
        }
    }
}

void Evolution::OriginTPGModule::doGeneration(size_t generationNumber) 
{

    GraphBased::NodeGenerator bidNodes;
    if(this->archive != nullptr) {
        // Add archive individual generator
        bidNodes.addGenerator(Dimensions::IndividualValueArchiveGenerator(*this->archive, this->memberRep, this->memberRep.getGenotypeGenerator(), *this->memberMutation));
    } else {
        // Add simple individual generator
        bidNodes.addGenerator(Dimensions::IndividualValueGenerator(this->memberRep, this->memberRep.getGenotypeGenerator(), *this->memberMutation));
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
    multi.addGenerator(actionGenerator, 0.99);
    multi.addGenerator(tangledGenerator, 0.01);

    bidNodes.addGenerator(multi);
    this->generator = std::make_unique<GraphBased::GenotypeGenerator>(bidNodes, std::make_pair(2U, 200U), std::make_pair(2U, 2U));

    std::vector<std::shared_ptr<const Individual>> potentialNodes;
    potentialNodes.insert(potentialNodes.begin(), this->population.begin(), this->population.end());
    dynamic_cast<Mutation::NodeTransferMutation*>(this->mutation.get())->setPotentialNodes(potentialNodes);

    std::vector<std::shared_ptr<const Individual>> populationBeforeGen = this->getPopulation();

    BasicModule::doGeneration(generationNumber);

    if(this->protectAggregated) {
        this->updateProtectedIndividuals(populationBeforeGen);
    }

    if(this->archive != nullptr && !this->isArchiveFixed) {
        if(this->isArchiveFixed) {
            this->updateArchiveOutputs();
        } else {
            this->updateArchive();
        }
    }

}

std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> Evolution::OriginTPGModule::getAllVertices() const
{
    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> vertices(population);
    std::vector<std::shared_ptr<const Individual>> toControl;
    toControl.insert(toControl.begin(), this->population.begin(), this->population.end());

    while(!toControl.empty()) {
        std::shared_ptr<const Individual> currentIndiv = *toControl.begin();
        std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> subVertices = this->getSubVerticesOf(*currentIndiv);
        for(const std::shared_ptr<const Individual>& vertex: subVertices) {
            if(vertices.find(vertex) == vertices.end()) {
                toControl.push_back(vertex);
                vertices.insert(vertex);
            }
        }
        toControl.erase(toControl.begin());
    }

    return std::move(vertices);
}

std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> Evolution::OriginTPGModule::getSubVerticesOf(const Individual& indiv) const
{
    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> vertices;
    std::vector<std::reference_wrapper<const GraphBased::GPNode>> nodesIndiv = indiv.getGenotype().getNodeGroup(0).getNodes();
    for(const GraphBased::GPNode& node: nodesIndiv) {
        if(node.getValue(1).getElementType() == typeid(std::shared_ptr<const Individual>)) {
            vertices.insert(node.getValue(1).getScalar<std::shared_ptr<const Individual>>());
        }
    }
    return std::move(vertices);
}

std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> Evolution::OriginTPGModule::getMembersOf(std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> individuals) const
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

std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> Evolution::OriginTPGModule::getAllMembers() const
{
    std::set<std::shared_ptr<const Individual>, SharedLess<Individual>> vertices = this->getAllVertices();
    return std::move(this->getMembersOf(vertices));
}

void Evolution::OriginTPGModule::setNewBasicLoggerRow(size_t generationNumber)
{
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

    std::vector<double> genCheckpoints = this->stopwatch.getCheckpoints();
    double mutTime = genCheckpoints.at(3);
    double evalTime = genCheckpoints.at(4);
    double generationTime = std::accumulate(genCheckpoints.begin(), genCheckpoints.end(), 0.0);

    for(std::shared_ptr<Log::Basiclogger>& defLogger: this->basicloggers) {
        defLogger->addGenerationValues<double>(
            {minFitness, averageFitness, maxFitness, mutTime, evalTime, generationTime, this->stopwatch.elapsed()});
    }
}