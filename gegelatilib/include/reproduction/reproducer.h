
#ifndef REPRODUCTION_REPRODUCER_H
#define REPRODUCTION_REPRODUCER_H

#include <set>

#include "individual.h"
#include "util/named.h"


 namespace Reproduction {
    /**
     * \brief Base class of the methods creating offspring from parents.
     */
    class Reproducer : public Util::Named
    {
    protected:

    public:

        /// Default polymorphic destructor
        virtual ~Reproducer() = default;

        // Disable copying to avoid accidental copies (use references or pointers instead).
        Reproducer(const Reproducer&) = delete;
        Reproducer& operator=(const Reproducer&) = delete;
    
        /**
         * \brief Main Reproducer constructor.
         *
         * \param[in] name name of the reproducer, only used to print it (see Util::Named).
         */
        Reproducer(std::string name = "Reproducer") : Util::Named(std::move(name)) {};

        /**
         * \brief Create offspring from parents.
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
