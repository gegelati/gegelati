#ifndef SELECTION_SELECTOR_H
#define SELECTION_SELECTOR_H

#include <set>
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

        public: 

            /// Default polymorphic destructor
            virtual ~Selector() = default;

            /**
             * \brief Default constructor
             * 
             * \param[in] replacement Boolean indicating wether this selection mechanism allows replacement.  
             */
            Selector(bool replacement) : replacement{replacement} {};
            
            // Disable copying to avoid accidental copies (use references or pointers instead).
            Selector(const Selector&) = delete;
            Selector& operator=(const Selector&) = delete;
            

            /**
             * \brief method performing the selection
             * 
             * \param[in] individualFitnesses the individuals containing the scores.
             * \param[in] nbSelected the number of individuals to select.
             * \param[in] rng required for selecting
             * 
             * \return a vector of the selected individuals.
             */
            virtual std::vector<std::shared_ptr<const Individual>> select(
                const std::vector<std::pair<double, std::shared_ptr<const Individual>>>& individualFitnesses,
                size_t nbSelected, RNG::RNG& rng) const = 0;
    };
};

#endif // SELECTION_SELECTOR_H