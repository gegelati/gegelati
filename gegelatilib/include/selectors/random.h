#ifndef RANDOM_SELECTOR_H
#define RANDOM_SELECTOR_H

#include "evolution/selection.h"

namespace Selectors {

    /**
     * \brief Class representing the mu plus lambda selection mechanism used for surviving selection
     */
    class Random : public Evolution::Selection {

        protected:            
        
        public: 

            /// @brief Default constructor setting replacement value to false
            Random(bool replacement) : Evolution::Selection(replacement) {}

            /**
             * \brief method performing the random selection
             * 
             * \param[in] individuals the individuals containing the scores.
             * \param[in] nbSelected the number of individuals to select.
             * \param[in] rng required for selecting
             * 
             * \return a vector of the selected individuals.
             */
            virtual std::vector<std::shared_ptr<const Evolution::Individual>> select(
                const std::set<std::shared_ptr<const Evolution::Individual>, SharedLess<Evolution::Individual>>& individuals,
                size_t nbSelected, RNG::RNG& rng) const override;
    };
};

#endif // MU_PLUS_LAMBDA_H