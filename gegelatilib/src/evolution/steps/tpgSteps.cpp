#include "evolution/steps/tpgSteps.h"

#include <algorithm>
#include <iterator>
#include <stdexcept>

#include "util/individualUtils.h"
#include <unordered_map>

#include "metrics/inputMetric.h"
#include "metrics/outputMetric.h"
#include "representations/TPG.h"
#include "evolution/originTPGEvolution.h"

Evolution::MutateVerticesStep::MutateVerticesStep(Key<std::vector<std::shared_ptr<Individual>>> individuals, Key<std::vector<std::shared_ptr<Individual>>> roots, Key<GraphBased::GenotypeGenerator> generator,
                                                  std::shared_ptr<Mutation::NodeTransferMutation> mutator, size_t nbThreads, std::string name)
    : Module(std::move(name)), individuals{std::move(individuals)}, roots{std::move(roots)}, generator{std::move(generator)}, mutator{std::move(mutator)},
      nbThreads{nbThreads}
{
    this->require(this->individuals);
    this->require(this->roots);
    this->require(this->generator);
    if(this->mutator == nullptr) {
        throw std::runtime_error("Evolution::MutateVerticesStep: the mutator cannot be null");
    }
}

std::string Evolution::MutateVerticesStep::details() const
{
    return this->individuals.name() + ", nodes of " + this->roots.name() + ", " + std::to_string(this->nbThreads) + " thread(s)";
}

void Evolution::MutateVerticesStep::apply(EvolutionData& state)
{
    // The roots are only shared with the mutation while mutating, so the step does not keep them alive.
    const std::vector<std::shared_ptr<Individual>>& roots = *state.get(this->roots);
    this->mutator->setPotentialNodes(std::make_shared<const std::vector<std::shared_ptr<const Individual>>>(roots.begin(), roots.end()));
    Mutation::mutateIndividualsInParallel(*this->mutator, *state.get(this->individuals), *state.get(this->generator), state.rng, this->nbThreads);
    this->mutator->setPotentialNodes(nullptr);
}

Evolution::CacheMemberOutputsStep::CacheMemberOutputsStep(std::vector<Key<std::vector<std::shared_ptr<Individual>>>> evaluated, Key<std::vector<std::shared_ptr<const Individual>>> cache, std::shared_ptr<Evaluation::PredictionProblem> problem,
                                                          std::shared_ptr<const Evaluation::Evaluator> evaluator, size_t nbIterations, std::string name)
    : Module(std::move(name)), evaluated{std::move(evaluated)}, cache{std::move(cache)}, problem{std::move(problem)}, evaluator{std::move(evaluator)},
      nbIterations{nbIterations}
{
    for(const Key<std::vector<std::shared_ptr<Individual>>>& key: this->evaluated) {
        this->require(key);
    }
    this->provide(this->cache);
    if(this->problem == nullptr) {
        throw std::runtime_error("Evolution::CacheMemberOutputsStep: the problem cannot be null");
    }
}

std::string Evolution::CacheMemberOutputsStep::details() const
{
    return std::to_string(this->nbIterations) + " samples";
}

void Evolution::CacheMemberOutputsStep::apply(EvolutionData& state)
{
    // Outputs cached at the previous run are for samples of a previous generation.
    if(const std::shared_ptr<const std::vector<std::shared_ptr<const Individual>>> cached = state.getIfAny(this->cache)) {
        for(const std::shared_ptr<const Individual>& member: *cached) {
            member->clearCachedOutputs();
        }
    }

    std::vector<std::shared_ptr<const Individual>> candidates;
    for(const Key<std::vector<std::shared_ptr<Individual>>>& key: this->evaluated) {
        const std::vector<std::shared_ptr<Individual>>& list = *state.get(key);
        candidates.insert(candidates.end(), list.begin(), list.end());
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

void Evolution::CreateArchiveStep::apply(EvolutionData& state)
{
    // Each answer of the archive is an array with the answer of each member.
    state.set(this->archive, std::make_shared<Evaluation::DataSet>(this->inputRequirements, Dimensions::Requirement(Data::DataType::type<double>({1}))));
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

void Evolution::UpdateArchiveStep::updateArchive(EvolutionData& state, Evaluation::DataSet& archive) const
{
    while(archive.size() > 0) {
        archive.removeData(0);
    }

    // Distinct inputs measured during the evaluation of the roots, indexed by their hash.
    const std::vector<std::shared_ptr<Individual>>& mutableRoots = *state.get(this->roots);
    const std::vector<std::shared_ptr<const Individual>> roots(mutableRoots.begin(), mutableRoots.end());
    const std::unordered_map<uint64_t, std::shared_ptr<const Individual>> rootIndex = Util::makeIndex(roots);
    std::map<uint64_t, std::vector<Data::DataValue>> allInputs;
    for(auto& [id, metricMap]: *state.get(this->metrics)) {
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

void Evolution::UpdateArchiveStep::updateArchiveOutputs(EvolutionData& state, Evaluation::DataSet& archive) const
{
    if(archive.size() == 0) {
        return;
    }

    const std::vector<std::shared_ptr<Individual>>& roots = *state.get(this->roots);
    const std::vector<std::shared_ptr<const Individual>> members = getMembersOf(getAllVertices(std::vector<std::shared_ptr<const Individual>>(roots.begin(), roots.end())));
    for(size_t idx = 0; idx < archive.size(); idx++) {
        archive.setOutputAt(idx, computeMemberAnswer(archive.getInputsAt(idx), members));
    }
}

void Evolution::UpdateArchiveStep::apply(EvolutionData& state)
{
    const std::shared_ptr<Evaluation::DataSet> archive = state.get(this->archive);

    if(this->isArchiveFixed && state.generation != EvolutionData::initGeneration) {
        this->updateArchiveOutputs(state, *archive);
    } else {
        this->updateArchive(state, *archive);
    }
}

void Evolution::ProtectAggregatedStep::apply(EvolutionData& state)
{
    // A new evolution starts without protected vertices.
    if(state.generation == EvolutionData::initGeneration) {
        state.set(this->protectedVertices, std::make_shared<std::map<uint64_t, std::shared_ptr<Individual>>>());
        state.set(this->aggregators, std::make_shared<std::map<uint64_t, std::set<uint64_t>>>());
        return;
    }
    std::map<uint64_t, std::shared_ptr<Individual>>& protectedVertices = *state.get(this->protectedVertices);
    std::map<uint64_t, std::set<uint64_t>>& aggregators = *state.get(this->aggregators);

    std::vector<std::shared_ptr<Individual>> roots(*state.get(this->roots));
    const auto constRoots = [&roots]() { return std::vector<std::shared_ptr<const Individual>>(roots.begin(), roots.end()); };

    // Aggregators still alive are the ones reachable from the roots.
    std::set<uint64_t> alive;
    for(const std::shared_ptr<const Individual>& vertex: getAllVertices(constRoots())) {
        alive.insert(vertex->getIndividualID());
    }
    for(auto& [id, aggregatorIDs]: aggregators) {
        for(auto it = aggregatorIDs.begin(); it != aggregatorIDs.end();) {
            it = (alive.count(*it) == 0) ? aggregatorIDs.erase(it) : std::next(it);
        }
    }

    // Vertices without aggregator come back as roots.
    for(auto it = aggregators.begin(); it != aggregators.end();) {
        if(it->second.empty()) {
            roots.push_back(protectedVertices.at(it->first));
            protectedVertices.erase(it->first);
            it = aggregators.erase(it);
        } else {
            it++;
        }
    }

    // Roots referenced by another root, and protected vertices referenced by a new root, get a new aggregator.
    std::unordered_map<uint64_t, std::shared_ptr<Individual>> rootIndex;
    for(const std::shared_ptr<Individual>& root: roots) {
        rootIndex.emplace(root->getIndividualID(), root);
    }
    for(const std::shared_ptr<Individual>& root: roots) {
        for(const std::shared_ptr<const Individual>& subVertex: getSubVertices(*root)) {
            const uint64_t subID = subVertex->getIndividualID();
            if(subID == root->getIndividualID()) {
                continue;
            }
            if(rootIndex.count(subID) > 0) {
                protectedVertices[subID] = rootIndex.at(subID);
                aggregators[subID].insert(root->getIndividualID());
            } else if(protectedVertices.count(subID) > 0) {
                aggregators[subID].insert(root->getIndividualID());
            }
        }
    }

    // Protected vertices are not roots anymore, they do not take part in the selection.
    std::vector<std::shared_ptr<Individual>> remaining;
    for(const std::shared_ptr<Individual>& root: roots) {
        if(protectedVertices.count(root->getIndividualID()) == 0) {
            remaining.push_back(root);
        }
    }
    std::sort(remaining.begin(), remaining.end(), [](const std::shared_ptr<Individual>& a, const std::shared_ptr<Individual>& b) {
        return a->getIndividualID() < b->getIndividualID();
    });
    state.set(this->roots, std::make_shared<std::vector<std::shared_ptr<Individual>>>(std::move(remaining)));
}
