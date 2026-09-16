#ifndef SELECTION_RANDOM_SELECTOR_H
#define SELECTION_RANDOM_SELECTOR_H

#include "selection/selector.h"

 namespace Selection {

    /**
     * \brief Class representing the mu plus lambda selection mechanism used for surviving selection
     */
    class RandomSelector : public Selector {

        protected:            
        
        public: 

            /// @brief Default constructor setting replacement value to false
            RandomSelector(bool replacement) : Selector(replacement) {}

            /**
             * \brief method performing the random selection
             * 
             * \param[in] individuals the individuals containing the scores.
             * \param[in] nbSelected the number of individuals to select.
             * \param[in] rng required for selecting
             * 
             * \return a vector of the selected individuals.
             */
            virtual std::vector<std::shared_ptr<const Individual>> select(
                const std::set<std::shared_ptr<const Individual>, SharedLess<Individual>>& individuals,
                size_t nbSelected, RNG::RNG& rng) const override;
    };
};

#endif // SELECTION_RANDOM_SELECTOR_H