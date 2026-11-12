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

            /**
             * \brief Constructor, the selection is without replacement.
             *
             * \param[in] name name of the selector, only used to print it (e.g. in the summary of an evolution).
             */
            explicit TruncationSelector(std::string name = "TruncationSelector") : Selector(false, std::move(name)) {}

            /**
             * \brief method performing the truncation selection
             * 
             * \param[in] fitnessMap the score of each individual, indexed by individual ID.
             * \param[in] nbSelected the number of individuals to select.
             * \param[in] rng required for selecting (unused for truncation)
             * 
             * If NbSelected is higher than the number of individuals, it will return a vector with each individual.
             * 
             * \return the IDs of the selected individuals, best first.
             */
            virtual std::vector<uint64_t> select(
                const std::map<uint64_t, double>& fitnessMap,
                size_t nbSelected, RNG::RNG& rng) const override;
    };
};

#endif // EVOLUTION_SELECTION_TRUNCATION_SELECTOR_H