
#ifndef POPULATION_H
#define POPULATION_H

#include <vector>
#include <memory>
#include <ranges>
#include <string>
#include <set>

#include "evolution/individual.h"
#include "util/genericComparator.h"

namespace Evolution {
    /**
     * \brief Class representing an Population used by an Representation.
     * 
     * The Population is in charge of storing, creating, copying or removing Individuals.
     */
    class Population
    {
    protected:

        /// Current individuals used by the representation
        std::set<std::unique_ptr<Individual>, UniqueLess<Individual>> individuals;

        
        /// Unique ID of the Population.
        size_t populationID;

        /**
         * \brief Incremente the Population ID counter and return the new value.
         */
        static size_t incrementeCounter();

        /**
         * \brief Reset the Population ID counter.
         *
         * This method set the ID counter to a new value.
         * It can quickly lead to segmentation fault if not used carefully.
         */
        static void resetPopulationIDCounter();
        friend struct ::CounterReset;

        /**
         * \brief Get the Individual iterator from a const Individual pointer.
         * 
         * \param[in] individual the Individual to get.
         */
        virtual std::set<std::unique_ptr<Individual>>::iterator getIndividualFromCst(const Individual& individual);

    public:

    
        /// Default polymorphic destructor
        virtual ~Population() = default;

        // Disable copying to avoid accidental copies (use references or pointers instead).
        Population(const Population&) = delete;
        Population& operator=(const Population&) = delete;

        /**
         * Constructor for individual population
         */
        Population() : populationID(incrementeCounter()) {};

        
        /**
         * \brief return the ID of the Population.
         */
        static size_t getPopulationIDCounter();
        
        /**
         * \brief Get the unique identifier of the Population.
         *
         * \return the integer ID of the Population.
         */
        virtual size_t getPopulationID() const;

        /**
         * \brief Set a new unique identifier to the Population.
         *
         * \param[in] newID the new integer ID to set to the Population.
         */
        virtual void setPopulationID(size_t newID);

        /**
         * \brief Get the current individuals used by the representation.
         */
        virtual const std::vector<std::reference_wrapper<const Individual>> getIndividuals() const;

        /**
         * \brief method that indicate if the population contains a specific individual.
         * 
         * \param[in] individual searched individual.
         */
        virtual bool containsIndividual(const Individual& individual) const;

        /**
         * \brief Get a mutable Individual from a const Individual pointer.
         * 
         * \param[in] individual the Individual to get.
         */
        virtual Individual& getMutableIndividual(const Individual& individual);

        /**
         * \brief Create a new Individual of the type used by the current representation.
         * 
         * \return a shared pointer to the created Individual.
         */
        virtual const Individual& createIndividual();

        /**
         * \brief Copy a new Individual of the type used by the current representation.
         * 
         * \param[in] individual the Individual to copy.
         * 
         * \return a shared pointer to the created Individual.
         */
        virtual const Individual& copyIndividual(const Individual& individual);

        /**
         * \brief Create a new Individual of the type used by the current representation.
         * 
         * \param[in] individual the Individual to delete.
         */
        virtual void deleteIndividual(const Individual& individual);

        /**
         * \brief Empty an Individual of the type used by the current representation.
         * 
         * \param[in] individual the Individual to empty.
         */
        virtual void emptyIndividual(const Individual& individual);

        /**
         * \brief Clear all individuals from the population.
         */
        virtual void clearIndividuals();

        /**
         * \brief Get the number of Individuals in the Population.
         * 
         * \return the number of Individuals in the Population.
         */
        virtual size_t size() const;
    };


    /**
     * \brief Comparison function to enable sorting of Population with
     * STL.
     */
    bool operator<(const Population& a, const Population& b);

    /**
     * \brief Comparison function to enable sorting of Population with
     * STL.
     */
    bool operator==(const Population& a, const Population& b);

    /**
     * \brief Comparison function to enable sorting of Population with
     * STL.
     */
    bool operator!=(const Population& a, const Population& b);

}; // namespace Representation

#endif // INDIVIDUAL_POPULATION_H
