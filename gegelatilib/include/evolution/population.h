
#ifndef POPULATION_H
#define POPULATION_H

#include <vector>
#include <memory>
#include <ranges>
#include <string>
#include <set>
#include <map>

#include "evolution/individual.h"
#include "util/genericComparator.h"

namespace Evolution {
    /**
     * \brief Class representing an Population used by an EvolutionAlgorithm.
     * 
     * The Population is in charge of storing, creating, copying or removing Individuals.
     * 
     * To unable aggregation of individuals from other individual with either member or tangled connections, the individuals are stored as shared_ptr.
     * The count of the shared_ptr indicate the number of aggregation.
     */
    class Population
    {
    protected:

        /// Current individuals used by the Population
        std::set<std::shared_ptr<Individual>, SharedLess<Individual>> individuals;
        
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
        virtual std::set<std::shared_ptr<Individual>>::iterator getIndividualFromCst(const Individual& individual);

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
         * \brief Get the current individuals.
         */
        virtual std::vector<std::reference_wrapper<const Individual>> getIndividuals() const;

        /**
         * \brief Get the pointers towards the current individuals.
         * 
         * The vector return references of shared_ptr to limit the increase of counts.
         */
        virtual std::vector<std::weak_ptr<const Individual>> getIndividualPtrs() const;

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
         * \brief Create a new empty Individual.
         * 
         * \return a shared pointer to the created Individual.
         */
        virtual const Individual& createIndividual();

        /**
         * \brief Copy an Individual.
         * 
         * \param[in] individual the Individual to copy.
         * 
         * \return a shared pointer to the created Individual.
         */
        virtual const Individual& copyIndividual(const Individual& individual);

        /**
         * \brief Delete an individual.
         * 
         * \param[in] individual the Individual to delete.
         */
        virtual bool deleteIndividual(const Individual& individual);

        /**
         * \brief Empty an Individual by clearing its gneotype.
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

}; // namespace Evolution

#endif // INDIVIDUAL_POPULATION_H
