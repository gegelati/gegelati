
#ifndef MUTATOR_H
#define MUTATOR_H

#include <vector>
#include <string>
#include <array>

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
         * \brief Sample a node value based on possible nodeValueTemplates.
         * 
         * If multiple nodeValueTemplates are proposed, a template is sampled uniformaly.
         * A nodeValue is then sampled based on the template.
         *
         * \param[in] valueTemplates node value templates of the sample value
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual Node::NodeValue sampleNodeValue(const std::vector<Node::NodeValueTemplate>& valueTemplates, RNG::RNG& rng);

        /**
         * \brief Create a random GPNode, and place it at a random index in the individual.
         *
         * \param[in] genotypeTemplate node template of the created individual
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual std::unique_ptr<Node::GPNode> createRandomNode(const std::vector<Node::NodeValueTemplate>& genotypeTemplate, RNG::RNG& rng);
        

        /**
         * \brief Initialize a random Individual.
         *
         * \param[in,out] indiv the individual initialized.
         * \param[in] genotypeTemplate genotype template of the individual
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void initRandomIndividual(Individual& indiv, const std::vector<Node::NodeValueTemplate>& genotypeTemplate, RNG::RNG& rng);
    };
}; // namespace Mutation

#endif
