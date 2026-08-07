#ifndef INDIVIDUAL_H_TEMPABCDE
#define INDIVIDUAL_H_TEMPABCDE

#include <cstdint>
#include <string>
#include <memory>

#include "node/gpNode.h"

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
        std::vector<std::unique_ptr<const Node::GPNode>> genotype;

        /// \brief Intron property of the individual, by default, no node is intron.
        std::vector<bool> isIntronNode;

        /// Unique ID of the individual.
        uint64_t individualID;

        /**
         * \brief Incremente the individual ID counter and return the new value.
         */
        static uint64_t incrementeCounter();

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
         * 
         */
        Individual() : individualID(incrementeCounter()) {};

        /**
         * \brief return the ID of the individual.
         */
        static uint64_t getIndividualIDCounter();
    
        /**
         * \brief Get the unique identifier of the Individual.
         *
         * \return the integer ID of the Individual.
         */
        virtual uint64_t getIndividualID() const;

        /**
         * \brief Set a new unique identifier to the Individual.
         *
         * \param[in] newID the new integer ID to set to the Individual.
         */
        virtual void setIndividualID(uint64_t newID);

        // Disable copying to avoid accidental copies (use references or pointers instead).
        Individual(const Individual&) = delete;
        Individual& operator=(const Individual&) = delete;


        
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
}; // namespace Evolution

#endif
