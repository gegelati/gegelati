
#ifndef MUTATOR_H
#define MUTATOR_H

#include <vector>
#include <string>
#include <array>

#include "evolution/individual.h"
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
         * \brief Sample a node value based on nodeValueTemplate.
         * 
         * If multiple configuration are proposed, a configuration is sampled uniformaly.
         * A nodeValue is then sampled based on the configuration.
         *
         * \param[in] nodeValueTemplate node value template of the sample value
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual Node::NodeValue sampleNodeValue(const Node::NodeValueTemplate& nodeValueTemplate, RNG::RNG& rng);

        /**
         * \brief Create a random GPNode.
         *
         * \param[in] nodeTemplate node template of the created node
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual std::unique_ptr<Node::GPNode> createRandomNode(const Node::NodeTemplate& nodeTemplate, RNG::RNG& rng);
        

        /**
         * \brief Initialize a random Individual.
         *
         * \param[in,out] indiv the individual initialized.
         * \param[in] genotypeTemplate genotype template of the individual
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void initRandomIndividual(Individual& indiv, const Node::GenotypeTemplate& genotypeTemplate, RNG::RNG& rng);


        /**
         * \brief mutate a GPNode.
         *
         * \param[in] node the node to mutate.
         * \param[in] nodeTemplate node template of the node
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void mutateNode(Node::GPNode& node, const Node::NodeTemplate& nodeTemplate, RNG::RNG& rng);

        /**
         * \brief Mutate an Individual.
         *
         * \param[in,out] indiv the individual mutated.
         * \param[in] genotypeTemplate genotype template of the individual
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void mutateIndividual(Individual& indiv, const Node::GenotypeTemplate& genotypeTemplate, RNG::RNG& rng);


    };
}; // namespace Mutation

#endif
