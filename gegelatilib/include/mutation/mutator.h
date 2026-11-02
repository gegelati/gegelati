
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
     * \brief Initialize a list of individuals
     * 
     * \param[in] mutator Mutator method used to initialize the individuals
     * \param[in] representation The representation for which individuals are created.
     * \param[in] generator The generator from which individuals are created.
     * \param[in] nbIndividuals The number of individuals created.
     * \param[in] rng Random Number Generator used in the mutation process.
     * \return the created individuals, in creation order.
     */
    std::vector<std::shared_ptr<Individual>> initIndividuals(
        const Mutator& mutator, const Representations::Representation& representation, 
        const GraphBased::GenotypeGenerator& generator, size_t nbIndividuals, RNG::RNG& rng);

    /**
     * \brief Mutate a list of individuals
     * 
     * Mutation seeds are drawn from the rng in the order of the list, so the result only depends on the list order.
     * 
     * \param[in] mutator Mutator method used to mutate the individuals
     * \param[in] individuals individuals mutated.
     * \param[in] generator The generator used to mutate the genotypes.
     * \param[in] rng Random Number Generator used in the mutation process.
     */
    void mutateIndividuals(
        const Mutator& mutator, const std::vector<std::shared_ptr<Individual>>& individuals, 
        const GraphBased::GenotypeGenerator& generator, RNG::RNG& rng);

    /**
     * \brief Same behavior as above method but in parallel.
     * 
     * \param[in] maxNbThreads maximum number of threads used.
     */
    void mutateIndividualsInParallel(
        const Mutator& mutator, const std::vector<std::shared_ptr<Individual>>& individuals,
        const GraphBased::GenotypeGenerator& generator,  RNG::RNG& rng, size_t maxNbThreads);
}; // namespace Mutator

#endif // MUTATION_MUTATOR_H
