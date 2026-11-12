#ifndef SELECTION_SELECTOR_H
#define SELECTION_SELECTOR_H

#include <set>
#include <map>
#include <algorithm>

#include "individual.h"

 namespace Selection {

    /**
     * \brief Class representing the selection mechanism of the evolution algorithm, that can be used either for parents selection, surviving selection or anything else requiring selection.
     */
    class Selector {

        protected:          
            /// Boolean indicating wether this selection mechanism allows replacement.  
            bool replacement;

            /// Name of the selector.
            std::string selectorName;

        public: 

            /// Default polymorphic destructor
            virtual ~Selector() = default;

            /**
             * \brief Default constructor
             * 
             * \param[in] replacement Boolean indicating wether this selection mechanism allows replacement.
             * \param[in] selectorName name of the selector.
             */
            Selector(bool replacement, std::string selectorName = "Selector") : replacement{replacement}, selectorName{std::move(selectorName)} {};

            /// Return the name of the selector.
            const std::string& getSelectorName() const { return this->selectorName; }
            
            // Disable copying to avoid accidental copies (use references or pointers instead).
            Selector(const Selector&) = delete;
            Selector& operator=(const Selector&) = delete;

            /**
             * \brief method performing the selection
             * 
             * Selection only relies on the scores: individuals are identified by their ID.
             * 
             * \param[in] fitnessMap the score of each individual, indexed by individual ID.
             * \param[in] nbSelected the number of individuals to select.
             * \param[in] rng required for selecting
             * 
             * \return the IDs of the selected individuals. An ID can appear several times if the selector allows replacement.
             */
            virtual std::vector<uint64_t> select(
                const std::map<uint64_t, double>& fitnessMap,
                size_t nbSelected, RNG::RNG& rng) const = 0;

            /**
             * \brief Static method to obtain fitness in ascending ordered by default
             * 
             * When two individuals have the same fitness, the one with the lower ID (the older) is considered as higher, in both orders.
             * 
             * \param[in] fitnessMap map of the score of each individual, indexed by individual ID.
             * \param[in] ascending boolean indicating if the order is ascending or descending, default True for ascending
             * \return pairs of individual ID and score, ordered.
             */
            static std::vector<std::pair<uint64_t, double>> getOrderedFitness(const std::map<uint64_t, double>& fitnessMap, bool ascending = true);
    };
};

#endif // SELECTION_SELECTOR_H