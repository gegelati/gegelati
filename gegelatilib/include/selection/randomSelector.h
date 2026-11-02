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
             * \param[in] fitnessMap the score of each individual, indexed by individual ID.
             * \param[in] nbSelected the number of individuals to select.
             * \param[in] rng required for selecting
             * 
             * \return the IDs of the selected individuals.
             */
            virtual std::vector<uint64_t> select(
                const std::map<uint64_t, double>& fitnessMap,
                size_t nbSelected, RNG::RNG& rng) const override;
    };
};

#endif // SELECTION_RANDOM_SELECTOR_H