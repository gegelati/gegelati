#ifndef TRUNCATION_H
#define TRUNCATION_H

#include "evolution/selection.h"

namespace Selectors {

    /**
     * \brief Class representing the truncation selection mechanism used for surviving selection, notably for mu+lambda replacements
     */
    class Truncation : public Evolution::Selection {

        protected:            
        
        public: 

            /// @brief Default constructor setting replacement value to false
            Truncation() : Evolution::Selection(false) {}

            /**
             * \brief method performing the truncation selection
             * 
             * \param[in] individuals the individuals containing the scores.
             * \param[in] nbSelected the number of individuals to select.
             * \param[in] rng required for selecting (unused for truncation)
             * 
             * If NbSelected is higher than the number of individuals, it will return a vector with each individual.
             * 
             * \return a vector of the selected individuals.
             */
            virtual std::vector<std::shared_ptr<const Evolution::Individual>> select(
                const std::set<std::shared_ptr<const Evolution::Individual>, SharedLess<Evolution::Individual>>& individuals,
                size_t nbSelected, RNG::RNG& rng) const override;
    };
};

#endif // MU_PLUS_LAMBDA_H