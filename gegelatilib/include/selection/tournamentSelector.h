#ifndef EVOLUTION_SELECTION_TOURNAMENT_SELECTOR_H
#define EVOLUTION_SELECTION_TOURNAMENT_SELECTOR_H

#include "selection/selector.h"

 namespace Selection {

    /**
     * \brief Class representing the tournament selection mechanism used surviving selection
     */
    class TournamentSelector : public Selector {

        protected:            
        
            /// Tournament sizes
            size_t sizeTournament;

        public: 

            /// @brief Default constructor setting replacement value to false
            TournamentSelector(size_t sizeTournament, bool replacement=true) : Selector(replacement), sizeTournament{sizeTournament} {}

            /**
             * \brief method performing the truncation selection
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

#endif // EVOLUTION_SELECTION_TRUNCATION_SELECTOR_H