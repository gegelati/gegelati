#ifndef INDIVIDUAL_H_TEMPABCDE
#define INDIVIDUAL_H_TEMPABCDE

#include <cstdint>
#include <string>
#include <memory>

#include "evolution/genotype.h"

struct CounterReset;
namespace Evolution {
    /**
     * \brief Class representing an Individual used by a population.
     * 
     * An individual is composed of GPNode, which are used to represent the genotype of the individual.
     */
    class Individual
    {
    protected:

        /// \brief Genotype of the individual.
        std::unique_ptr<Genotype> genotype;

        /// Unique ID of the individual.
        size_t individualID;

        /**
         * \brief Incremente the individual ID counter and return the new value.
         */
        static size_t incrementeCounter();

        /**
         * \brief Reset the individual ID counter.
         *
         * This method set the ID counter to a new value.
         * It can quickly lead to segmentation fault if not used carefully.
         */
        static void resetIndividualIDCounter();
        friend struct ::CounterReset;

    public:

        /// Default polymorphic destructor
        virtual ~Individual() = default;

        /**
         * \brief Constructor for the Individual.
         */
        Individual() : genotype(std::make_unique<Genotype>()), individualID(incrementeCounter()) {};

        /**
         * \brief return the ID of the individual.
         */
        static size_t getIndividualIDCounter();
    
        /**
         * \brief Get the unique identifier of the Individual.
         *
         * \return the integer ID of the Individual.
         */
        virtual size_t getIndividualID() const;

        /**
         * \brief Set a new unique identifier to the Individual.
         *
         * \param[in] newID the new integer ID to set to the Individual.
         */
        virtual void setIndividualID(size_t newID);

        // Disable copying to avoid accidental copies (use references or pointers instead).
        Individual(const Individual&) = delete;
        Individual& operator=(const Individual&) = delete;

        /**
         * \brief Get the size of the Genotype.
         * 
         * \return the size of the Genotype.
         */
        virtual size_t getSize() const;

        /**
         * \brief Get the genotype of the Individual.
         */
        virtual const Genotype& getGenotype() const;

        /**
         * \brief Get the mutable genotype of the Individual.
         */
        virtual Genotype& getMutableGenotype();

        /**
         * \brief Compare two individuals' genotype.
         * 
         * \param[in] other the individual compared.
         * \param[in] effectiveGenotype true to compare only the effective genotype.
         */
        //virtual bool hasSameGenotypeAs(const Individual& other, bool effectiveGenotype = false) const;
    };
    
    /**
     * \brief Comparison function to enable sorting of Individual with
     * STL.
     */
    bool operator<(const Individual& a, const Individual& b);

    /**
     * \brief Comparison function to enable sorting of Individual with
     * STL.
     */
    bool operator==(const Individual& a, const Individual& b);

    /**
     * \brief Comparison function to enable sorting of Individual with
     * STL.
     */
    bool operator!=(const Individual& a, const Individual& b);

    /**
     * \brief Comparison function to enable sorting of Individual with
     * STL.
     */
    bool operator>(const Individual& a, const Individual& b);

    /**
     * \brief Comparison function to enable sorting of Individual with
     * STL.
     */
    bool operator<=(const Individual& a, const Individual& b);

    /**
     * \brief Comparison function to enable sorting of Individual with
     * STL.
     */
    bool operator>=(const Individual& a, const Individual& b);

}; // namespace Evolution

#endif
