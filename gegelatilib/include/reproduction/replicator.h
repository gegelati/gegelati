
#ifndef REPRODUCTION_REPLICATOR_H
#define REPRODUCTION_REPLICATOR_H

#include <set>

#include "reproduction/reproducer.h"


 namespace Reproduction {
    /**
     * \brief Reproducer creating the offspring as copies of random parents.
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
         *
         * \param[in] name name of the reproducer, only used to print it (e.g. in the summary of an evolution).
         */
        explicit Replicator(std::string name = "Replicator") : Reproducer(std::move(name)) {};

        /**
         * \brief Basic reproduction method replicating individuals
         * 
         * \param[in] parents the parents reproduced
         * \param[in] nbOffspring the number of offspring created.
         * \param[in] rng Random Number Generator.
         * \return the offspring, in creation order.
         */
        virtual std::vector<std::shared_ptr<Individual>> reproduce(
            std::vector<std::shared_ptr<const Individual>> parents, size_t nbOffspring, RNG::RNG& rng) const override;



    };
}; // namespace Reproduction

#endif // REPRODUCTION_REPLICATOR_H
