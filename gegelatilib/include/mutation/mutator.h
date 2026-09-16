
#ifndef MUTATION_MUTATOR_H
#define MUTATION_MUTATOR_H

#include <vector>
#include <string>
#include <array>
#include <set>

#include "individual.h"
#include "mutator/rng.h"


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
        virtual std::unique_ptr<GraphBased::GPNode> createRandomNode(GraphBased::NodeGenerator& nodeGenerator, RNG::RNG& rng) const;
        

        /**
         * \brief Initialize a random Genotype.
         *
         * \param[in] genotypeGenerator genotype Generator of the genotype
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual std::unique_ptr<GraphBased::Genotype> initRandomGenotype(std::unique_ptr<GraphBased::GenotypeGenerator> genotypeGenerator, RNG::RNG& rng) const;

        /**
         * \brief return a mutated copy of a GPNode
         *
         * \param[in] node the node to mutate.
         * \param[in] nodeGenerator node Generator of the node
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void mutateNode(GraphBased::GPNode& node, GraphBased::NodeGenerator& nodeGenerator, RNG::RNG& rng) const = 0;

        /**
         * \brief return a mutated copy of a Genotype.
         *
         * \param[in,out] genotype the genotype mutated.
         * \param[in] genotypeGenerator genotype Generator of the genotype
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual std::unique_ptr<GraphBased::Genotype> mutateGenotype(const GraphBased::Genotype& genotype, std::unique_ptr<GraphBased::GenotypeGenerator> genotypeGenerator, RNG::RNG& rng) const = 0;
    };

    /**
     * \brief Initialize a set of individuals
     * 
     * \param[in] mutator Mutator method used to initialize the individuals
     * \param[in] representation The representation from which individuals are created.
     * \param[in] nbIndividuals The number of individuals created.
     * \param[in] rng Random Number Generator used in the mutation process.
     */
    static std::set<std::shared_ptr<Individual>, SharedLess<Individual>> 
        initIndividuals(const Mutator& mutator, const Representations::Representation& representation, size_t nbIndividuals, RNG::RNG& rng)
    {
        std::set<std::shared_ptr<Individual>, SharedLess<Individual>> individuals;
        for(size_t idx = 0; idx < nbIndividuals; idx++) {
            individuals.insert(std::make_shared<Individual>(
                representation, 
                mutator.initRandomGenotype(representation.getGenotypeGenerator(), rng)
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
    static void mutateIndividuals(const Mutator& mutator, std::set<std::shared_ptr<Individual>, SharedLess<Individual>> individuals, RNG::RNG& rng)
    {
        for(const std::shared_ptr<Individual>& indiv: individuals) {
            indiv->setGenotype(std::move(mutator.mutateGenotype(indiv->getGenotype(), indiv->getRepresentation().getGenotypeGenerator(), rng)));
        }
    }
}; // namespace Mutator

#endif // MUTATION_MUTATOR_H
