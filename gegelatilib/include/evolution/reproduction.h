
#ifndef REPRODUCTION_H
#define REPRODUCTION_H

#include <set>

#include "evolution/individual.h"


namespace Evolution {
    /**
     * \brief Class representing a Reproduction.
     */
    class Reproduction
    {
    protected:

    public:

        /// Default polymorphic destructor
        virtual ~Reproduction() = default;

        // Disable copying to avoid accidental copies (use references or pointers instead).
        Reproduction(const Reproduction&) = delete;
        Reproduction& operator=(const Reproduction&) = delete;
    
        /**
         * \brief Main Reproduction constructor.
         */
        Reproduction() {};

        /**
         * \brief Basic reproduction method replicating individuals
         * 
         * \param[in] parents the parents reproduced
         * \param[in] nbOffspring the number of offspring created.
         * \param[in] rng Random Number Generator.
         */
        std::set<std::shared_ptr<Evolution::Individual>, SharedLess<Evolution::Individual>> reproduce(
            std::vector<std::shared_ptr<const Evolution::Individual>> parents, size_t nbOffspring, RNG::RNG& rng) const;



    };
}; // namespace Reproduction

#endif
