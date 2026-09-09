
#ifndef MUTATOR_H
#define MUTATOR_H

#include <vector>
#include <string>
#include <array>

#include "evolution/genotype.h"
#include "node/genotypeTemplate.h"
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
        virtual std::unique_ptr<Node::GPNode> createRandomNode(Node::NodeTemplate& nodeTemplate, RNG::RNG& rng);
        

        /**
         * \brief Initialize a random Genotype.
         *
         * \param[in,out] genotype the genotype initialized.
         * \param[in] genotypeTemplate genotype Template of the genotype
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void initRandomGenotype(Genotype& genotype, std::unique_ptr<Node::GenotypeTemplate> genotypeTemplate, RNG::RNG& rng);


        /**
         * \brief mutate a GPNode.
         *
         * \param[in] node the node to mutate.
         * \param[in] nodeTemplate node Template of the node
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void mutateNode(Node::GPNode& node, Node::NodeTemplate& nodeTemplate, RNG::RNG& rng);

        /**
         * \brief Mutate a Genotype.
         *
         * \param[in,out] genotype the genotype mutated.
         * \param[in] genotypeTemplate genotype Template of the genotype
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void mutateGenotype(Genotype& genotype, std::unique_ptr<Node::GenotypeTemplate> genotypeTemplate, RNG::RNG& rng);


    };
}; // namespace Mutation

#endif
