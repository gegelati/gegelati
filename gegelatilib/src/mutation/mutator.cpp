#include "mutation/mutator.h"

std::unique_ptr<GraphBased::GPNode> Mutation::Mutator::createRandomNode(const GraphBased::NodeGenerator& nodeGenerator, RNG::RNG& rng) const
{
    if(nodeGenerator.size() == 0) {
        throw std::runtime_error("Mutation::Mutator::createRandomNode: NodeGenerator is empty.");
    }

    std::vector<Data::DataValue> values;
    for(size_t idxValue = 0; idxValue < nodeGenerator.size(); idxValue++) {

        values.push_back(nodeGenerator.getGeneratorAt(idxValue).initialize(rng));
    }
    return std::make_unique<GraphBased::GPNode>(values);
}


std::unique_ptr<GraphBased::Genotype> Mutation::Mutator::initRandomGenotype(const GraphBased::GenotypeGenerator& genotypeGenerator, RNG::RNG& rng) const
{
    if(genotypeGenerator.size() == 0) {
        throw std::runtime_error("Mutation::Mutator::initRandomGenotype: genotypeGenerator is empty.");
    }
    std::unique_ptr<GraphBased::Genotype> genotype = std::make_unique<GraphBased::Genotype>();

    for(size_t idxGenerator = 0; idxGenerator < genotypeGenerator.size(); idxGenerator++) {
        std::unique_ptr<GraphBased::NodeGroup> nodeGroup = std::make_unique<GraphBased::NodeGroup>();

        const std::pair<size_t, size_t>& range = genotypeGenerator.getInitRangeAt(idxGenerator);
        size_t nbNodesOfGenerator = rng.getUnsignedInt64(range.first, range.second);

        for(size_t idxNode = 0; idxNode < nbNodesOfGenerator; idxNode++) {
            nodeGroup->addNode(this->createRandomNode(genotypeGenerator.getNodeGeneratorAt(idxGenerator), rng));
        }
        genotype->addNodeGroup(std::move(nodeGroup));
    }
    return std::move(genotype);
}

std::set<std::shared_ptr<Individual>, SharedLess<Individual>> 
    Mutation::initIndividuals(
        const Mutator& mutator, const Representations::Representation& representation, 
        const GraphBased::GenotypeGenerator& generator, size_t nbIndividuals, RNG::RNG& rng)
{
    std::set<std::shared_ptr<Individual>, SharedLess<Individual>> individuals;
    for(size_t idx = 0; idx < nbIndividuals; idx++) {
        individuals.insert(std::make_shared<Individual>(
            representation, 
            mutator.initRandomGenotype(generator, rng)
        ));
    }
    return individuals;
}

void Mutation::mutateIndividuals(const Mutator& mutator, std::set<std::shared_ptr<Individual>, SharedLess<Individual>> individuals, const GraphBased::GenotypeGenerator& generator, RNG::RNG& rng)
{
    std::queue<size_t> rngSeeds;
    for(const std::shared_ptr<Individual>& indiv: individuals) {
        rngSeeds.emplace(rng.getUnsignedInt64(0, UINT64_MAX));
    }

    for(const std::shared_ptr<Individual>& indiv: individuals) {
        RNG::RNG localRng;
        localRng.setSeed(rngSeeds.front());
        indiv->setGenotype(std::move(mutator.mutateGenotype(indiv->getGenotype(), generator, localRng)));
        rngSeeds.pop();
    }
}

void Mutation::mutateIndividualsInParallel(
    const Mutator& mutator,
    std::set<std::shared_ptr<Individual>, SharedLess<Individual>> individuals,
    const GraphBased::GenotypeGenerator& generator,
    RNG::RNG& rng,
    size_t maxNbThreads)
{
    if (individuals.empty()) {
        return;
    } else if (maxNbThreads == 1 || individuals.size() == 1) {
        return Mutation::mutateIndividuals(mutator, individuals, generator, rng);
    }

    // Generate one independent seed per individual.
    std::queue<std::pair<uint64_t, std::shared_ptr<Individual>>> jobsQueue;
    for (auto it = individuals.begin(); it != individuals.end(); it++) {
        jobsQueue.emplace(std::make_pair(rng.getUnsignedInt64(0, UINT64_MAX), *it));
    }
    std::mutex jobMutex;

    auto worker = [&]() {
        while (true) {
            std::shared_ptr<Individual> indiv;
            uint64_t seed;

            {
                std::lock_guard<std::mutex> lock(jobMutex);
                if (jobsQueue.empty()) {
                    break;
                }

                seed = jobsQueue.front().first;
                indiv = jobsQueue.front().second;
                jobsQueue.pop();
            }

            RNG::RNG localRng;
            localRng.setSeed(seed);

            indiv->setGenotype(
                std::move(mutator.mutateGenotype(
                    indiv->getGenotype(), generator, localRng)));
        }
    };

    // Create worker threads.
    std::vector<std::thread> threads;
    const size_t nbThreads = std::min(maxNbThreads, individuals.size());

    for (size_t i = 0; i < nbThreads; ++i) {
        threads.emplace_back(worker);
    }
    for (auto& thread : threads) {
        thread.join();
    }
}