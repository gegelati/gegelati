
#ifndef MUTATOR_H
#define MUTATOR_H

#include <vector>
#include <string>
#include <array>
#include <set>

#include "evolution/individual.h"
#include "mutator/rng.h"


namespace Evolution {
    /**
     * \brief Class representing a Mutation.
     */
    class Mutation
    {
    protected:

    public:

        /// Default polymorphic destructor
        virtual ~Mutation() = default;

        // Disable copying to avoid accidental copies (use references or pointers instead).
        Mutation(const Mutation&) = delete;
        Mutation& operator=(const Mutation&) = delete;
    
        /**
         * \brief Main Mutation constructor.
         */
        Mutation() {};

        /**
         * \brief Create a random GPNode.
         *
         * \param[in] nodeTemplate node Template of the created node
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual std::unique_ptr<Node::GPNode> createRandomNode(Node::NodeTemplate& nodeTemplate, RNG::RNG& rng) const;
        

        /**
         * \brief Initialize a random Genotype.
         *
         * \param[in] genotypeTemplate genotype Template of the genotype
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual std::unique_ptr<Genotype> initRandomGenotype(std::unique_ptr<Node::GenotypeTemplate> genotypeTemplate, RNG::RNG& rng) const;

        /**
         * \brief Initialize an individual
         * 
         * \param[in] representation The representation from which individual is created.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual std::shared_ptr<Individual>
            initIndividual(const Representation& representation, RNG::RNG& rng) const;

        /**
         * \brief Initialize a set of individuals
         * 
         * \param[in] representation The representation from which individuals are created.
         * \param[in] nbIndividuals The number of individuals created.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual std::set<std::shared_ptr<Individual>, SharedLess<Individual>> 
            initIndividuals(const Representation& representation, size_t nbIndividuals, RNG::RNG& rng) const;

        /**
         * \brief return a mutated copy of a GPNode
         *
         * \param[in] node the node to mutate.
         * \param[in] nodeTemplate node Template of the node
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void mutateNode(Node::GPNode& node, Node::NodeTemplate& nodeTemplate, RNG::RNG& rng) const;

        /**
         * \brief return a mutated copy of a Genotype.
         *
         * \param[in,out] genotype the genotype mutated.
         * \param[in] genotypeTemplate genotype Template of the genotype
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual std::unique_ptr<Genotype> mutateGenotype(const Genotype& genotype, std::unique_ptr<Node::GenotypeTemplate> genotypeTemplate, RNG::RNG& rng) const;

        /**
         * \brief Mutate a set of individuals
         * 
         * \param[in] individuals set of individuals mutated.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void mutateIndividuals(std::set<std::shared_ptr<Individual>, SharedLess<Individual>> individuals, RNG::RNG& rng) const;

    };
}; // namespace Mutation

#endif
