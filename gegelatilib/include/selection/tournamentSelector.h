#ifndef EVOLUTION_SELECTION_TOURNAMENT_SELECTOR_H
#define EVOLUTION_SELECTION_TOURNAMENT_SELECTOR_H

#include "selection/selector.h"

 namespace Selection {

    /**
     * \brief Selector keeping the best individual of random tournaments.
     */
    class TournamentSelector : public Selector {

        protected:            
        
            /// Number of candidates of each tournament.
            size_t sizeTournament;

        public: 

            /**
             * \brief Constructor.
             *
             * \param[in] sizeTournament number of candidates of each tournament.
             * \param[in] replacement indicates if the same individual can be selected several times.
             * \param[in] name name of the selector, only used to print it (e.g. in the summary of an evolution).
             */
            TournamentSelector(size_t sizeTournament, bool replacement = true, std::string name = "TournamentSelector")
                : Selector(replacement, std::move(name)), sizeTournament{sizeTournament} {}

            /// A name given in place of the replacement flag would silently convert to a bool: write TournamentSelector(size, true, "name").
            TournamentSelector(size_t sizeTournament, const char* name) = delete;

            /// Inherited from Named, adds the size of the tournaments and the replacement.
            virtual std::string getDetails() const override
            {
                return this->getName() + "(size " + std::to_string(this->sizeTournament) + (this->replacement ? ", with replacement)" : ", without replacement)");
            }


            /**
             * \brief method performing the tournament selection
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

#endif // EVOLUTION_SELECTION_TOURNAMENT_SELECTOR_H