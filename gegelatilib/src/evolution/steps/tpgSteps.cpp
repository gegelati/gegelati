#include "evolution/steps/tpgSteps.h"

#include <algorithm>
#include <iterator>
#include <stdexcept>

#include "util/individualUtils.h"
#include <unordered_map>

#include "metrics/inputMetric.h"
#include "metrics/outputMetric.h"
#include "representations/TPG.h"
#include "evolution/originTPGEvolutionConfig.h"

Evolution::MutateVerticesStep::MutateVerticesStep(std::string individuals, std::string roots, std::string generator,
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
    return this->individuals + ", nodes of " + this->roots + ", " + std::to_string(this->nbThreads) + " thread(s)";
}

void Evolution::MutateVerticesStep::apply(EvolutionData& state)
{
    // The roots are only shared with the mutation while mutating, so the step does not keep them alive.
    const std::vector<std::shared_ptr<Individual>>& roots = *state.get<std::vector<std::shared_ptr<Individual>>>(this->roots);
    this->mutator->setPotentialNodes(std::make_shared<const std::vector<std::shared_ptr<const Individual>>>(roots.begin(), roots.end()));
    Mutation::mutateIndividualsInParallel(*this->mutator, *state.get<std::vector<std::shared_ptr<Individual>>>(this->individuals), *state.get<GraphBased::GenotypeGenerator>(this->generator), state.rng, this->nbThreads);
    this->mutator->setPotentialNodes(nullptr);
}

Evolution::CacheMemberOutputsStep::CacheMemberOutputsStep(std::vector<std::string> evaluated, std::string cache, std::shared_ptr<Evaluation::PredictionProblem> problem,
                                                          std::shared_ptr<const Evaluation::Evaluator> evaluator, size_t nbIterations, Evaluation::Mode mode, std::string name)
    : Module(std::move(name)), evaluated{std::move(evaluated)}, cache{std::move(cache)}, problem{std::move(problem)}, evaluator{std::move(evaluator)},
      nbIterations{nbIterations}, mode{mode}
{
    for(const std::string& key: this->evaluated) {
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

namespace {
    // Execute the individuals once on the samples of the evaluation and cache their outputs in them (Individual::cacheOutput).
    // Individuals whose own sub-individuals are already cached run faster, so callers order them accordingly.
    void cacheOutputsOf(const std::vector<std::shared_ptr<const Individual>>& individuals, Evaluation::PredictionProblem& problem,
                        const Evaluation::Evaluator& evaluator, size_t nbIterations, uint64_t generation, Evaluation::Mode mode)
    {
        if(individuals.empty()) {
            return;
        }

        auto outputTemplate = std::make_shared<Metrics::MetricMapTemplate>(std::make_unique<Metrics::OutputMetric>());
        std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>> results =
            evaluator.evaluateIndividuals(individuals, problem, outputTemplate, nbIterations, generation, mode);

        // Hashes are the indices of the samples, and are iterated in the same order as the outputs were measured.
        const std::set<uint64_t> hashes = problem.computeEvaluationHashes(nbIterations, generation, mode, problem.getProblemSeed());
        const Evaluation::DataSet& dataset = problem.getDataSet();

        for(const std::shared_ptr<const Individual>& individual: individuals) {
            std::map<uint64_t, const Metrics::OutputMetric*> metrics =
                results.at(individual->getIndividualID())->getMetricValues<Metrics::OutputMetric>(Metrics::OutputMetric::staticHash());
            if(metrics.empty()) {
                continue;
            }

            const std::vector<Data::DataValue>& outputs = metrics.begin()->second->getOutputs();
            size_t idxOutput = 0;
            for(uint64_t hash: hashes) {
                if(idxOutput >= outputs.size()) {
                    break;
                }
                individual->cacheOutput(dataset.getInputsAt(hash).at(0).getPtr(), outputs.at(idxOutput).clone());
                idxOutput++;
            }
        }
    }

    // Gather the individuals of the lists of the state.
    std::vector<std::shared_ptr<const Individual>> gatherIndividuals(const Evolution::EvolutionData& state, const std::vector<std::string>& keys)
    {
        std::vector<std::shared_ptr<const Individual>> gathered;
        for(const std::string& key: keys) {
            const std::vector<std::shared_ptr<Individual>>& list = *state.get<std::vector<std::shared_ptr<Individual>>>(key);
            gathered.insert(gathered.end(), list.begin(), list.end());
        }
        return gathered;
    }

    // Clear the outputs cached at the previous run: they are for samples of a previous generation.
    void clearPreviousCache(const Evolution::EvolutionData& state, const std::string& cache)
    {
        if(state.has(cache)) {
            for(const std::shared_ptr<const Individual>& cached: *state.get<std::vector<std::shared_ptr<const Individual>>>(cache)) {
                cached->clearCachedOutputs();
            }
        }
    }
}

void Evolution::CacheMemberOutputsStep::apply(EvolutionData& state)
{
    clearPreviousCache(state, this->cache);

    const std::vector<std::shared_ptr<const Individual>> members = getMembersOf(gatherIndividuals(state, this->evaluated));
    state.set(this->cache, std::make_shared<std::vector<std::shared_ptr<const Individual>>>(members));
    cacheOutputsOf(members, *this->problem, *this->evaluator, this->nbIterations, state.generation, this->mode);
}

Evolution::CacheVertexOutputsStep::CacheVertexOutputsStep(std::vector<std::string> evaluated, std::string cache, std::shared_ptr<Evaluation::PredictionProblem> problem,
                                                          std::shared_ptr<const Evaluation::Evaluator> evaluator, size_t nbIterations, Evaluation::Mode mode, std::string name)
    : Module(std::move(name)), evaluated{std::move(evaluated)}, cache{std::move(cache)}, problem{std::move(problem)}, evaluator{std::move(evaluator)},
      nbIterations{nbIterations}, mode{mode}
{
    for(const std::string& key: this->evaluated) {
        this->require(key);
    }
    this->provide(this->cache);
    if(this->problem == nullptr) {
        throw std::runtime_error("Evolution::CacheVertexOutputsStep: the problem cannot be null");
    }
}

std::string Evolution::CacheVertexOutputsStep::details() const
{
    return std::to_string(this->nbIterations) + " samples";
}

std::vector<std::vector<std::shared_ptr<const Individual>>> Evolution::CacheVertexOutputsStep::getSharedVerticesByHeight(
    const std::vector<std::shared_ptr<const Individual>>& evaluated)
{
    const std::vector<std::shared_ptr<const Individual>> vertices = getAllVertices(evaluated);

    // Number of executions of a vertex for a sample: once as an evaluated individual, and once per vertex referencing it.
    std::unordered_map<uint64_t, size_t> nbExecutions;
    std::unordered_map<uint64_t, std::vector<std::shared_ptr<const Individual>>> subVertices;
    for(const std::shared_ptr<const Individual>& vertex: evaluated) {
        nbExecutions[vertex->getIndividualID()]++;
    }
    for(const std::shared_ptr<const Individual>& vertex: vertices) {
        std::vector<std::shared_ptr<const Individual>>& subs = subVertices[vertex->getIndividualID()];
        subs = getSubVertices(*vertex);
        for(const std::shared_ptr<const Individual>& sub: subs) {
            nbExecutions[sub->getIndividualID()]++;
        }
    }

    // Height of a vertex: 0 if it only points to actions, else one more than its highest sub-vertex.
    // Vertices are visited iteratively (depth-first, children before parents), so deep graphs cannot overflow the stack.
    std::unordered_map<uint64_t, size_t> heights;
    std::set<uint64_t> inProgress;
    for(const std::shared_ptr<const Individual>& start: vertices) {
        std::vector<std::pair<std::shared_ptr<const Individual>, size_t>> stack{{start, 0}};
        while(!stack.empty()) {
            const std::shared_ptr<const Individual> vertex = stack.back().first;
            const uint64_t id = vertex->getIndividualID();
            if(heights.count(id) > 0) {
                stack.pop_back();
                continue;
            }
            const std::vector<std::shared_ptr<const Individual>>& subs = subVertices.at(id);
            size_t& nextSub = stack.back().second;
            if(nextSub == 0) {
                if(!inProgress.insert(id).second) {
                    throw std::runtime_error("Evolution::CacheVertexOutputsStep: a cycle was found in the graph");
                }
            }
            if(nextSub < subs.size()) {
                const std::shared_ptr<const Individual> sub = subs.at(nextSub++);
                if(heights.count(sub->getIndividualID()) == 0) {
                    stack.emplace_back(sub, 0);
                }
                continue;
            }
            size_t height = 0;
            for(const std::shared_ptr<const Individual>& sub: subs) {
                height = std::max(height, heights.at(sub->getIndividualID()) + 1);
            }
            heights[id] = height;
            inProgress.erase(id);
            stack.pop_back();
        }
    }

    // Only vertices executed several times for a sample gain from the cache.
    std::vector<std::vector<std::shared_ptr<const Individual>>> layers;
    for(const std::shared_ptr<const Individual>& vertex: vertices) {
        if(nbExecutions.at(vertex->getIndividualID()) < 2) {
            continue;
        }
        const size_t height = heights.at(vertex->getIndividualID());
        if(layers.size() <= height) {
            layers.resize(height + 1);
        }
        layers.at(height).push_back(vertex);
    }
    return layers;
}

void Evolution::CacheVertexOutputsStep::apply(EvolutionData& state)
{
    clearPreviousCache(state, this->cache);

    // Layers are executed from the lowest one: the shared vertices they point to have their outputs cached already,
    // so each vertex only costs the bids of its own nodes.
    std::vector<std::shared_ptr<const Individual>> cached;
    for(const std::vector<std::shared_ptr<const Individual>>& layer: getSharedVerticesByHeight(gatherIndividuals(state, this->evaluated))) {
        cacheOutputsOf(layer, *this->problem, *this->evaluator, this->nbIterations, state.generation, this->mode);
        cached.insert(cached.end(), layer.begin(), layer.end());
    }
    state.set(this->cache, std::make_shared<std::vector<std::shared_ptr<const Individual>>>(std::move(cached)));
}

void Evolution::CreateArchiveStep::apply(EvolutionData& state)
{
    // Each answer of the archive is an array with the answer of each member.
    state.add(this->archive, std::make_shared<Evaluation::DataSet>(this->inputRequirements, Dimensions::Requirement(Data::DataType::type<double>({1}))));
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
    const std::vector<std::shared_ptr<Individual>>& mutableRoots = *state.get<std::vector<std::shared_ptr<Individual>>>(this->roots);
    const std::vector<std::shared_ptr<const Individual>> roots(mutableRoots.begin(), mutableRoots.end());
    const std::unordered_map<uint64_t, std::shared_ptr<const Individual>> rootIndex = Util::makeIndex(roots);
    std::map<uint64_t, std::vector<Data::DataValue>> allInputs;
    for(auto& [id, metricMap]: *state.get<std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>>(this->metrics)) {
        if(rootIndex.count(id) == 0) {
            continue;
        }
        std::map<uint64_t, const Metrics::InputMetric*> inputMetrics =
            metricMap->getMetricValues<Metrics::InputMetric>(Metrics::InputMetric::staticHash());

        for(const auto& [featureHash, inputMetric]: inputMetrics) {
            for(const std::vector<Data::DataValue>& inputsStep: inputMetric->getInputs()) {
                const uint64_t inputsHash = Data::DataValue::getCombinedHash(inputsStep);
                if(allInputs.find(inputsHash) == allInputs.end()) {
                    std::vector<Data::DataValue> inputsClone;
                    for(const Data::DataValue& input: inputsStep) {
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

    const std::vector<std::shared_ptr<Individual>>& roots = *state.get<std::vector<std::shared_ptr<Individual>>>(this->roots);
    const std::vector<std::shared_ptr<const Individual>> members = getMembersOf(getAllVertices(std::vector<std::shared_ptr<const Individual>>(roots.begin(), roots.end())));
    for(size_t idx = 0; idx < archive.size(); idx++) {
        archive.setOutputAt(idx, computeMemberAnswer(archive.getInputsAt(idx), members));
    }
}

void Evolution::UpdateArchiveStep::apply(EvolutionData& state)
{
    const std::shared_ptr<Evaluation::DataSet> archive = state.get<Evaluation::DataSet>(this->archive);

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
        state.add(this->protectedVertices, std::make_shared<std::map<uint64_t, std::shared_ptr<Individual>>>());
        state.add(this->aggregators, std::make_shared<std::map<uint64_t, std::set<uint64_t>>>());
        return;
    }
    std::map<uint64_t, std::shared_ptr<Individual>>& protectedVertices = *state.get<std::map<uint64_t, std::shared_ptr<Individual>>>(this->protectedVertices);
    std::map<uint64_t, std::set<uint64_t>>& aggregators = *state.get<std::map<uint64_t, std::set<uint64_t>>>(this->aggregators);

    std::vector<std::shared_ptr<Individual>> roots(*state.get<std::vector<std::shared_ptr<Individual>>>(this->roots));
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
