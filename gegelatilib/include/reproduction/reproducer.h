
#ifndef REPRODUCTION_REPRODUCER_H
#define REPRODUCTION_REPRODUCER_H

#include <set>

#include "individual.h"


 namespace Reproduction {
    /**
     * \brief Class representing a Reproduction.
     */
    class Reproducer
    {
    protected:
        /// Name of the reproducer.
        std::string reproducerName;

    public:

        /// Default polymorphic destructor
        virtual ~Reproducer() = default;

        // Disable copying to avoid accidental copies (use references or pointers instead).
        Reproducer(const Reproducer&) = delete;
        Reproducer& operator=(const Reproducer&) = delete;
    
        /**
         * \brief Main Reproducer constructor.
         *
         * \param[in] reproducerName name of the reproducer.
         */
        Reproducer(std::string reproducerName = "Reproducer") : reproducerName{std::move(reproducerName)} {};

        /// Return the name of the reproducer.
        const std::string& getReproducerName() const { return this->reproducerName; }

        /**
         * \brief Basic reproduction method replicating individuals
         * 
         * \param[in] parents the parents reproduced
         * \param[in] nbOffspring the number of offspring created.
         * \param[in] rng Random Number Generator.
         * \return the offspring, in creation order.
         */
        virtual std::vector<std::shared_ptr<Individual>> reproduce(
            std::vector<std::shared_ptr<const Individual>> parents, size_t nbOffspring, RNG::RNG& rng) const = 0;



    };
}; // namespace Reproduction

#endif // REPRODUCTION_REPRODUCER_H
