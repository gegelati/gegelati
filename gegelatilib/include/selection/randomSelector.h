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

            /**
             * \brief Constructor.
             *
             * \param[in] replacement indicates if the same individual can be selected several times.
             * \param[in] name name of the selector, only used to print it (e.g. in the summary of an evolution).
             */
            RandomSelector(bool replacement, std::string name = "RandomSelector") : Selector(replacement, std::move(name)) {}

            /// Inherited from Named, adds the replacement.
            virtual std::string getDetails() const override { return this->getName() + (this->replacement ? "(with replacement)" : "(without replacement)"); }

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