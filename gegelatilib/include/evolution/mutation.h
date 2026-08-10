
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
         * \brief Initialize a random Individual.
         *
         * \param[in,out] indiv the individual initialized.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void initRandomIndividual(Individual& indiv, RNG::RNG& rng);

        /**
         * \brief Create a random GPNode, and place it at a random index in the individual.
         *
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual std::unique_ptr<Node::GPNode> createRandomNode(RNG::RNG& rng);
    };
}; // namespace Mutation

#endif
