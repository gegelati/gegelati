
#ifndef REPRODUCTION_REPLICATOR_H
#define REPRODUCTION_REPLICATOR_H

#include <set>

#include "reproduction/reproducer.h"


 namespace Reproduction {
    /**
     * \brief Class representing a Reproduction.
     */
    class Replicator : public Reproducer
    {
    protected:

    public:

        /// Default polymorphic destructor
        virtual ~Replicator() = default;

        // Disable copying to avoid accidental copies (use references or pointers instead).
        Replicator(const Replicator&) = delete;
        Replicator& operator=(const Replicator&) = delete;
    
        /**
         * \brief Main Reproducer constructor.
         */
        Replicator() {};

        /**
         * \brief Basic reproduction method replicating individuals
         * 
         * \param[in] parents the parents reproduced
         * \param[in] nbOffspring the number of offspring created.
         * \param[in] rng Random Number Generator.
         */
        virtual std::set<std::shared_ptr<Individual>, SharedLess<Individual>> reproduce(
            std::vector<std::shared_ptr<const Individual>> parents, size_t nbOffspring, RNG::RNG& rng) const override;



    };
}; // namespace Reproduction

#endif // REPRODUCTION_REPLICATOR_H
