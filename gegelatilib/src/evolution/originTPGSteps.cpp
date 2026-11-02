#include "evolution/originTPGSteps.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <queue>
#include <stdexcept>

#include "dimensions/dataValueGenerator.h"
#include "dimensions/individualArchiveValueGenerator.h"
#include "dimensions/individualValueGenerator.h"
#include "graphBased/genotypeGenerator.h"
#include "graphBased/nodeGenerator.h"
#include "metrics/inputMetric.h"
#include "metrics/outputMetric.h"
#include "mutation/mutator.h"

namespace {
    // Individuals referenced by the value at a given index of the nodes of the first node group.
    std::vector<std::shared_ptr<const Individual>> getReferencedBy(const Individual& vertex, size_t idxValue)
    {
        std::map<uint64_t, std::shared_ptr<const Individual>> referenced;
        const std::vector<std::reference_wrapper<const GraphBased::GPNode>> nodes = vertex.getGenotype().getNodeGroup(0).getNodes();
        for(const GraphBased::GPNode& node: nodes) {
            if(node.getValue(idxValue).getElementType() == typeid(std::shared_ptr<const Individual>)) {
                const std::shared_ptr<const Individual>& individual = node.getValue(idxValue).getScalar<std::shared_ptr<const Individual>>();
                referenced.emplace(individual->getIndividualID(), individual);
            }
        }

        std::vector<std::shared_ptr<const Individual>> result;
        for(const auto& [id, individual]: referenced) {
            result.push_back(individual);
        }
        return result;
    }

    // Index of the member in a node, and of the destination in a node.
    constexpr size_t memberValueIndex = 0;
    constexpr size_t destinationValueIndex = 1;

    // Generator of the genotypes of the vertices. Members are created from the archive if there is one, and destinations
    // are actions or, if there are vertices, tangled connections towards them. The number of nodes follows the
    // TPG representation, but new vertices start with the minimum.
    // The generator keeps references to its arguments: they must outlive it.
    std::unique_ptr<GraphBased::GenotypeGenerator> makeVertexGenerator(const Representations::TPG& representation,
                                                                       const Representations::Representation& memberRepresentation,
                                                                       const Mutation::Mutator& memberMutator,
                                                                       const std::vector<std::shared_ptr<const Individual>>& vertices,
                                                                       const Evaluation::DataSet* archive)
    {
        GraphBased::NodeGenerator bidNodes;
        if(archive != nullptr) {
            bidNodes.addGenerator(Dimensions::IndividualValueArchiveGenerator(*archive, memberRepresentation, memberRepresentation.getGenotypeGenerator(), memberMutator));
        } else {
            bidNodes.addGenerator(Dimensions::IndividualValueGenerator(memberRepresentation, memberRepresentation.getGenotypeGenerator(), memberMutator));
        }

        Dimensions::NumericUniformGenerator<size_t> actionGenerator(0, representation.getNbActions() - 1);
        if(vertices.empty()) {
            bidNodes.addGenerator(actionGenerator);
        } else {
            Dimensions::ListUniformGenerator<std::shared_ptr<const Individual>> tangledGenerator(vertices);
            Dimensions::MultiGenerator multi;
            multi.addGenerator(actionGenerator, 0.995);
            multi.addGenerator(tangledGenerator, 0.005);
            bidNodes.addGenerator(multi);
        }

        const std::pair<size_t, size_t> mutationRange = representation.getGenotypeGenerator().getMutationRangeAt(0);
        const std::pair<size_t, size_t> initRange{mutationRange.first, mutationRange.first};
        return std::make_unique<GraphBased::GenotypeGenerator>(bidNodes, mutationRange, initRange);
    }
}

Evolution::InitVerticesStep::InitVerticesStep(std::shared_ptr<const Population> population, std::shared_ptr<const Mutation::Mutator> mutator,
                                              std::shared_ptr<const Representations::Representation> memberRepresentation,
                                              std::shared_ptr<const Mutation::Mutator> memberMutator, std::string archive, std::string name)
    : Step(std::move(name)), population{std::move(population)}, mutator{std::move(mutator)}, memberRepresentation{std::move(memberRepresentation)},
      memberMutator{std::move(memberMutator)}, archive{std::move(archive)}
{
    if(this->population == nullptr || this->mutator == nullptr || this->memberRepresentation == nullptr || this->memberMutator == nullptr) {
        throw std::runtime_error("Evolution::InitVerticesStep: the population, the representation of the members and the mutators cannot be null");
    }
    if(std::dynamic_pointer_cast<const Representations::TPG>(this->population->getRepresentation()) == nullptr) {
        throw std::runtime_error("Evolution::InitVerticesStep: the representation of the population must be a TPG");
    }
}

std::set<std::string> Evolution::InitVerticesStep::providedData() const
{
    return {this->population->individuals()};
}

std::string Evolution::InitVerticesStep::details() const
{
    return std::to_string(this->population->getSize()) + " vertices";
}

void Evolution::InitVerticesStep::apply(EvolutionState& state)
{
    const auto& representation = static_cast<const Representations::TPG&>(*this->population->getRepresentation());
    const std::shared_ptr<const Evaluation::DataSet> archiveData = state.getIfAny<Evaluation::DataSet>(this->archive);
    const std::unique_ptr<GraphBased::GenotypeGenerator> generator =
        makeVertexGenerator(representation, *this->memberRepresentation, *this->memberMutator, {}, archiveData.get());

    std::vector<std::shared_ptr<Individual>> individuals = Mutation::initIndividuals(*this->mutator, representation, *generator, this->population->getSize(), state.rng);
    state.set(this->population->individuals(), std::make_shared<std::vector<std::shared_ptr<const Individual>>>(individuals.begin(), individuals.end()));
}

Evolution::MutateVerticesStep::MutateVerticesStep(std::string individuals, std::string roots, std::string archive,
                                                  std::shared_ptr<const Representations::TPG> representation,
                                                  std::shared_ptr<Mutation::NodeTransferMutation> mutator,
                                                  std::shared_ptr<const Representations::Representation> memberRepresentation,
                                                  std::shared_ptr<const Mutation::Mutator> memberMutator, size_t nbThreads, std::string name)
    : Step(std::move(name)), individuals{std::move(individuals)}, roots{std::move(roots)}, archive{std::move(archive)}, representation{std::move(representation)},
      mutator{std::move(mutator)}, memberRepresentation{std::move(memberRepresentation)}, memberMutator{std::move(memberMutator)}, nbThreads{nbThreads}
{
    if(this->representation == nullptr || this->mutator == nullptr || this->memberRepresentation == nullptr || this->memberMutator == nullptr) {
        throw std::runtime_error("Evolution::MutateVerticesStep: the representations and the mutators cannot be null");
    }
}

std::set<std::string> Evolution::MutateVerticesStep::requiredData() const
{
    return {this->individuals, this->roots};
}

std::string Evolution::MutateVerticesStep::details() const
{
    return this->individuals + ", nodes of " + this->roots + ", " + std::to_string(this->nbThreads) + " thread(s)";
}

void Evolution::MutateVerticesStep::apply(EvolutionState& state)
{
    const std::shared_ptr<const std::vector<std::shared_ptr<const Individual>>> currentRoots = state.get<std::vector<std::shared_ptr<const Individual>>>(this->roots);
    const std::vector<std::shared_ptr<const Individual>> vertices = getAllVertices(*currentRoots);
    const std::shared_ptr<const Evaluation::DataSet> archiveData = state.getIfAny<Evaluation::DataSet>(this->archive);
    const std::unique_ptr<GraphBased::GenotypeGenerator> generator =
        makeVertexGenerator(*this->representation, *this->memberRepresentation, *this->memberMutator, vertices, archiveData.get());

    // The roots are only shared with the mutation while mutating, so the step does not keep them alive.
    this->mutator->setPotentialNodes(currentRoots);
    Mutation::mutateIndividualsInParallel(*this->mutator, *state.get<std::vector<std::shared_ptr<Individual>>>(this->individuals), *generator, state.rng, this->nbThreads);
    this->mutator->setPotentialNodes(nullptr);
}

std::vector<std::shared_ptr<const Individual>> Evolution::getSubVertices(const Individual& vertex)
{
    return getReferencedBy(vertex, destinationValueIndex);
}

std::vector<std::shared_ptr<const Individual>> Evolution::getAllVertices(const std::vector<std::shared_ptr<const Individual>>& roots)
{
    std::map<uint64_t, std::shared_ptr<const Individual>> vertices;
    std::queue<std::shared_ptr<const Individual>> toControl;
    for(const std::shared_ptr<const Individual>& root: roots) {
        if(vertices.emplace(root->getIndividualID(), root).second) {
            toControl.push(root);
        }
    }

    while(!toControl.empty()) {
        const std::shared_ptr<const Individual> current = toControl.front();
        toControl.pop();
        for(const std::shared_ptr<const Individual>& subVertex: getSubVertices(*current)) {
            if(vertices.emplace(subVertex->getIndividualID(), subVertex).second) {
                toControl.push(subVertex);
            }
        }
    }

    std::vector<std::shared_ptr<const Individual>> result;
    for(const auto& [id, vertex]: vertices) {
        result.push_back(vertex);
    }
    return result;
}

std::vector<std::shared_ptr<const Individual>> Evolution::getMembersOf(const std::vector<std::shared_ptr<const Individual>>& vertices)
{
    std::map<uint64_t, std::shared_ptr<const Individual>> members;
    for(const std::shared_ptr<const Individual>& vertex: vertices) {
        for(const std::shared_ptr<const Individual>& member: getReferencedBy(*vertex, memberValueIndex)) {
            members.emplace(member->getIndividualID(), member);
        }
    }

    std::vector<std::shared_ptr<const Individual>> result;
    for(const auto& [id, member]: members) {
        result.push_back(member);
    }
    return result;
}

Evolution::CacheMemberOutputsStep::CacheMemberOutputsStep(std::vector<std::string> evaluated, std::string cache, std::shared_ptr<Evaluation::PredictionProblem> problem,
                                                          std::shared_ptr<const Evaluation::Evaluator> evaluator, size_t nbIterations, std::string name)
    : Step(std::move(name)), evaluated{std::move(evaluated)}, cache{std::move(cache)}, problem{std::move(problem)}, evaluator{std::move(evaluator)},
      nbIterations{nbIterations}
{
    if(this->problem == nullptr) {
        throw std::runtime_error("Evolution::CacheMemberOutputsStep: the problem cannot be null");
    }
}

std::set<std::string> Evolution::CacheMemberOutputsStep::requiredData() const
{
    return std::set<std::string>(this->evaluated.begin(), this->evaluated.end());
}

std::set<std::string> Evolution::CacheMemberOutputsStep::providedData() const
{
    return {this->cache};
}

std::string Evolution::CacheMemberOutputsStep::details() const
{
    return std::to_string(this->nbIterations) + " samples";
}

void Evolution::CacheMemberOutputsStep::apply(EvolutionState& state)
{
    // Outputs cached at the previous run are for samples of a previous generation.
    if(const std::shared_ptr<const std::vector<std::shared_ptr<const Individual>>> cached = state.getIfAny<std::vector<std::shared_ptr<const Individual>>>(this->cache)) {
        for(const std::shared_ptr<const Individual>& member: *cached) {
            member->clearCachedOutputs();
        }
    }

    std::vector<std::shared_ptr<const Individual>> candidates;
    for(const std::string& list: this->evaluated) {
        const std::vector<std::shared_ptr<const Individual>> individuals = state.getIndividuals(list);
        candidates.insert(candidates.end(), individuals.begin(), individuals.end());
    }
    const std::vector<std::shared_ptr<const Individual>> members = getMembersOf(candidates);
    state.set(this->cache, std::make_shared<std::vector<std::shared_ptr<const Individual>>>(members));
    if(members.empty()) {
        return;
    }

    auto memberTemplate = std::make_shared<Metrics::MetricMapTemplate>(std::make_unique<Metrics::OutputMetric>());
    std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>> memberResults = this->evaluator->evaluateIndividuals(
        members, *this->problem, memberTemplate, this->nbIterations, state.generation, Evaluation::Mode::TRAINING);

    // Hashes are the indices of the samples, and are iterated in the same order as the outputs were measured.
    const std::set<uint64_t> hashes = this->problem->computeEvaluationHashes(
        this->nbIterations, state.generation, Evaluation::Mode::TRAINING, this->problem->getProblemSeed());
    const Evaluation::DataSet& dataset = this->problem->getDataSet();

    for(const std::shared_ptr<const Individual>& member: members) {
        std::map<uint64_t, const Metrics::OutputMetric*> metrics =
            memberResults.at(member->getIndividualID())->getMetricValues<Metrics::OutputMetric>(Metrics::OutputMetric::staticHash());
        if(metrics.empty()) {
            continue;
        }

        const std::vector<Data::DataValue>& outputs = metrics.begin()->second->getOutputs();
        size_t idxOutput = 0;
        for(uint64_t hash: hashes) {
            if(idxOutput >= outputs.size()) {
                break;
            }
            member->cacheOutput(dataset.getInputsAt(hash).at(0).getPtr(), outputs.at(idxOutput).clone());
            idxOutput++;
        }
    }
}

size_t Evolution::ProtectAggregatedStep::getNbProtected() const
{
    return this->protectedVertices.size();
}

std::set<std::string> Evolution::ProtectAggregatedStep::requiredData() const
{
    return {this->roots};
}

std::set<std::string> Evolution::ProtectAggregatedStep::providedData() const
{
    return {this->roots};
}

void Evolution::ProtectAggregatedStep::apply(EvolutionState& state)
{
    if(state.generation == EvolutionState::initGeneration) {
        this->protectedVertices.clear();
        this->aggregators.clear();
        return;
    }

    std::vector<std::shared_ptr<const Individual>> roots(*state.get<std::vector<std::shared_ptr<const Individual>>>(this->roots));

    // Aggregators still alive are the ones reachable from the roots.
    std::set<uint64_t> alive;
    for(const std::shared_ptr<const Individual>& vertex: getAllVertices(roots)) {
        alive.insert(vertex->getIndividualID());
    }
    for(auto& [id, aggregatorIDs]: this->aggregators) {
        for(auto it = aggregatorIDs.begin(); it != aggregatorIDs.end();) {
            it = (alive.count(*it) == 0) ? aggregatorIDs.erase(it) : std::next(it);
        }
    }

    // Vertices without aggregator come back as roots.
    for(auto it = this->aggregators.begin(); it != this->aggregators.end();) {
        if(it->second.empty()) {
            roots.push_back(this->protectedVertices.at(it->first));
            this->protectedVertices.erase(it->first);
            it = this->aggregators.erase(it);
        } else {
            it++;
        }
    }

    // Roots referenced by another root, and protected vertices referenced by a new root, get a new aggregator.
    const std::unordered_map<uint64_t, std::shared_ptr<const Individual>> rootIndex = makeIndex(roots);
    for(const std::shared_ptr<const Individual>& root: roots) {
        for(const std::shared_ptr<const Individual>& subVertex: getSubVertices(*root)) {
            const uint64_t subID = subVertex->getIndividualID();
            if(subID == root->getIndividualID()) {
                continue;
            }
            if(rootIndex.count(subID) > 0) {
                this->protectedVertices[subID] = subVertex;
                this->aggregators[subID].insert(root->getIndividualID());
            } else if(this->protectedVertices.count(subID) > 0) {
                this->aggregators[subID].insert(root->getIndividualID());
            }
        }
    }

    // Protected vertices are not roots anymore, they do not take part in the selection.
    std::vector<std::shared_ptr<const Individual>> remaining;
    for(const std::shared_ptr<const Individual>& root: roots) {
        if(this->protectedVertices.count(root->getIndividualID()) == 0) {
            remaining.push_back(root);
        }
    }
    std::sort(remaining.begin(), remaining.end(), [](const std::shared_ptr<const Individual>& a, const std::shared_ptr<const Individual>& b) {
        return a->getIndividualID() < b->getIndividualID();
    });
    state.set(this->roots, std::make_shared<std::vector<std::shared_ptr<const Individual>>>(std::move(remaining)));
}

std::set<std::string> Evolution::CreateArchiveStep::providedData() const
{
    return {this->archive};
}

void Evolution::CreateArchiveStep::apply(EvolutionState& state)
{
    // Each answer of the archive is an array with the answer of each member.
    state.set(this->archive, std::make_shared<Evaluation::DataSet>(this->inputRequirements, Dimensions::Requirement(Data::DataType::type<double>({1}))));
}

std::set<std::string> Evolution::UpdateArchiveStep::requiredData() const
{
    return {this->roots, this->metrics, this->archive};
}

std::set<std::string> Evolution::UpdateArchiveStep::providedData() const
{
    return {this->archive};
}

std::string Evolution::UpdateArchiveStep::details() const
{
    return std::to_string(this->archiveSize) + " samples" + (this->isArchiveFixed ? ", fixed inputs" : "");
}

Data::DataValue Evolution::UpdateArchiveStep::computeMemberAnswer(const std::vector<Data::DataValue>& inputs, const std::vector<std::shared_ptr<const Individual>>& members)
{
    std::vector<double> answers;
    for(const std::shared_ptr<const Individual>& member: members) {
        answers.push_back(member->execute(inputs).getScalar<double>());
    }
    return Data::DataValue::array1d(answers);
}

void Evolution::UpdateArchiveStep::updateArchive(EvolutionState& state, Evaluation::DataSet& archive) const
{
    while(archive.size() > 0) {
        archive.removeData(0);
    }

    // Distinct inputs measured during the evaluation of the roots, indexed by their hash.
    const std::vector<std::shared_ptr<const Individual>>& roots = *state.get<std::vector<std::shared_ptr<const Individual>>>(this->roots);
    const std::unordered_map<uint64_t, std::shared_ptr<const Individual>> rootIndex = makeIndex(roots);
    std::map<uint64_t, std::vector<Data::DataValue>> allInputs;
    for(auto& [id, metricMap]: *state.get<std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>>(this->metrics)) {
        if(rootIndex.count(id) == 0) {
            continue;
        }
        std::map<uint64_t, const Metrics::InputMetric*> inputMetrics =
            metricMap->getMetricValues<Metrics::InputMetric>(Metrics::InputMetric::staticHash());

        for(const auto& [featureHash, inputMetric]: inputMetrics) {
            for(const std::shared_ptr<const std::vector<Data::DataValue>>& inputsStep: inputMetric->getInputs()) {
                const uint64_t inputsHash = Data::DataValue::getCombinedHash(*inputsStep);
                if(allInputs.find(inputsHash) == allInputs.end()) {
                    std::vector<Data::DataValue> inputsClone;
                    for(const Data::DataValue& input: *inputsStep) {
                        inputsClone.push_back(input.clone());
                    }
                    allInputs.insert(std::make_pair(inputsHash, std::move(inputsClone)));
                }
            }
        }
    }

    const std::vector<std::shared_ptr<const Individual>> members = getMembersOf(getAllVertices(roots));
    if(members.empty()) {
        return;
    }

    // Keep random inputs if there are more than the archive can store.
    const bool sampleRandomly = allInputs.size() > this->archiveSize;
    while(!allInputs.empty() && archive.size() < this->archiveSize) {
        auto it = allInputs.begin();
        if(sampleRandomly) {
            std::advance(it, state.rng.uniformSample<uint64_t>(0, allInputs.size() - 1));
        }

        Data::DataValue answer = computeMemberAnswer(it->second, members);
        archive.addData(std::move(it->second), std::move(answer));
        allInputs.erase(it);
    }
}

void Evolution::UpdateArchiveStep::updateArchiveOutputs(EvolutionState& state, Evaluation::DataSet& archive) const
{
    if(archive.size() == 0) {
        return;
    }

    const std::vector<std::shared_ptr<const Individual>> members = getMembersOf(getAllVertices(*state.get<std::vector<std::shared_ptr<const Individual>>>(this->roots)));
    for(size_t idx = 0; idx < archive.size(); idx++) {
        archive.setOutputAt(idx, computeMemberAnswer(archive.getInputsAt(idx), members));
    }
}

void Evolution::UpdateArchiveStep::apply(EvolutionState& state)
{
    const std::shared_ptr<Evaluation::DataSet> archive = state.get<Evaluation::DataSet>(this->archive);

    if(this->isArchiveFixed && state.generation != EvolutionState::initGeneration) {
        this->updateArchiveOutputs(state, *archive);
    } else {
        this->updateArchive(state, *archive);
    }
}
