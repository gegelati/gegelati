
#ifndef MUTATION_MUTATOR_H
#define MUTATION_MUTATOR_H

#include <vector>
#include <string>
#include <array>
#include <set>

#include <mutex>
#include <queue>
#include <thread>

#include "individual.h"
#include "rng/rng.h"


 namespace Mutation {
    /**
     * \brief Class representing a Mutator.
     */
    class Mutator
    {
    protected:

    public:

        /// Default polymorphic destructor
        virtual ~Mutator() = default;

        // Disable copying to avoid accidental copies (use references or pointers instead).
        Mutator(const Mutator&) = delete;
        Mutator& operator=(const Mutator&) = delete;
    
        /**
         * \brief Main Mutator constructor.
         */
        Mutator() {};

        /**
         * \brief Create a random Node.
         *
         * \param[in] nodeGenerator node Generator of the created node
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual std::unique_ptr<GraphBased::GPNode> createRandomNode(const GraphBased::NodeGenerator& nodeGenerator, RNG::RNG& rng) const;
        

        /**
         * \brief Initialize a random Genotype.
         *
         * \param[in] genotypeGenerator genotype Generator of the genotype
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual std::unique_ptr<GraphBased::Genotype> initRandomGenotype(const GraphBased::GenotypeGenerator& genotypeGenerator, RNG::RNG& rng) const;

        /**
         * \brief return a mutated copy of a GPNode
         *
         * \param[in] node the node to mutate.
         * \param[in] nodeGenerator node Generator of the node
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void mutateNode(GraphBased::GPNode& node, const GraphBased::NodeGenerator& nodeGenerator, RNG::RNG& rng) const = 0;

        /**
         * \brief return a mutated copy of a Genotype.
         *
         * \param[in,out] genotype the genotype mutated.
         * \param[in] genotypeGenerator genotype Generator of the genotype
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual std::unique_ptr<GraphBased::Genotype> mutateGenotype(const GraphBased::Genotype& genotype, const GraphBased::GenotypeGenerator& genotypeGenerator, RNG::RNG& rng) const = 0;
    };

    /**
     * \brief Initialize a set of individuals
     * 
     * \param[in] mutator Mutator method used to initialize the individuals
     * \param[in] representation The representation for which individuals are created.
     * \param[in] generator The generator from which individuals are created.
     * \param[in] nbIndividuals The number of individuals created.
     * \param[in] rng Random Number Generator used in the mutation process.
     */
    static std::set<std::shared_ptr<Individual>, SharedLess<Individual>> 
        initIndividuals(const Mutator& mutator, const Representations::Representation& representation, const GraphBased::GenotypeGenerator& generator, size_t nbIndividuals, RNG::RNG& rng)
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

    /**
     * \brief Mutate a set of individuals
     * 
     * \param[in] mutator Mutator method used to mutate the individuals
     * \param[in] individuals set of individuals mutated.
     * \param[in] rng Random Number Generator used in the mutation process.
     */
    static void mutateIndividuals(const Mutator& mutator, std::set<std::shared_ptr<Individual>, SharedLess<Individual>> individuals, const GraphBased::GenotypeGenerator& generator, RNG::RNG& rng)
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

    /**
     * \brief Same behavior as above method but in parallel.
     */
    static void mutateIndividualsInParallel(
        const Mutator& mutator,
        std::set<std::shared_ptr<Individual>, SharedLess<Individual>> individuals,
        const GraphBased::GenotypeGenerator& generator,
        RNG::RNG& rng,
        size_t maxNbThreads)
    {
        if (individuals.empty()) {
            return;
        }

        // Generate one independent seed per individual.
        std::queue<uint64_t> rngSeeds;
        for (size_t i = 0; i < individuals.size(); ++i) {
            rngSeeds.emplace(rng.getUnsignedInt64(0, UINT64_MAX));
        }

        // Create the job queue.
        std::queue<std::shared_ptr<Individual>> individualsQueue;
        for (const auto& indiv : individuals) {
            individualsQueue.push(indiv);
        }

        std::mutex queueMutex;
        std::mutex seedsMutex;

        auto worker = [&]() {
            while (true) {
                std::shared_ptr<Individual> indiv;
                uint64_t seed;

                {
                    std::lock_guard<std::mutex> lock(queueMutex);

                    if (individualsQueue.empty()) {
                        break;
                    }

                    indiv = individualsQueue.front();
                    individualsQueue.pop();
                }

                {
                    std::lock_guard<std::mutex> lock(seedsMutex);
                    seed = rngSeeds.front();
                    rngSeeds.pop();
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
}; // namespace Mutator

#endif // MUTATION_MUTATOR_H
