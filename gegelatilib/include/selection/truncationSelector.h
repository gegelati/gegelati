#ifndef EVOLUTION_SELECTION_TRUNCATION_SELECTOR_H
#define EVOLUTION_SELECTION_TRUNCATION_SELECTOR_H

#include "selection/selector.h"

 namespace Selection {

    /**
     * \brief Class representing the truncation selection mechanism used for surviving selection, notably for mu+lambda replacements
     */
    class TruncationSelector : public Selector {

        protected:            
        
        public: 

            /// @brief Default constructor setting replacement value to false
            TruncationSelector() : Selector(false) {}

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
            virtual std::vector<std::shared_ptr<const Individual>> select(
                const std::set<std::shared_ptr<const Individual>, SharedLess<Individual>>& individuals,
                size_t nbSelected, RNG::RNG& rng) const override;
    };
};

#endif // EVOLUTION_SELECTION_TRUNCATION_SELECTOR_H