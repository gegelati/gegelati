#ifndef SELECTION_H
#define SELECTION_H

#include <set>
#include <algorithm>

#include "evolution/individual.h"
#include "evaluation/evaluationResult.h"

#include "evaluation/scoreMetric.h"

namespace Evolution {

    /**
     * \brief Class representing the selection mechanism of the evolution algorithm, that can be used either for parents selection, surviving selection or anything else requiring selection.
     */
    class Selection {

        protected:          
            /// Boolean indicating wether this selection mechanism allows replacement.  
            bool replacement;

        public: 

            /// Default polymorphic destructor
            virtual ~Selection() = default;

            /**
             * \brief Default constructor
             * 
             * \param[in] replacement Boolean indicating wether this selection mechanism allows replacement.  
             */
            Selection(bool replacement) : replacement{replacement} {};
            
            // Disable copying to avoid accidental copies (use references or pointers instead).
            Selection(const Selection&) = delete;
            Selection& operator=(const Selection&) = delete;

            /**
             * \brief method returning the evaluation metrics required for the selection process.
             */
            virtual std::vector<std::unique_ptr<Evaluation::EvaluationMetric>> getSelectionMetrics();
            
            /**
             * \brief method ranking the individuals based on their average score on their evaluationRun
             * 
             * \param[in] individuals the individuals containing the scores.
             */
            virtual std::vector<std::pair<double, std::shared_ptr<const Individual>>> getRankedScores(
                const std::set<std::shared_ptr<const Individual>, SharedLess<Evolution::Individual>>& individuals) const;

            /**
             * \brief method performing the selection
             * 
             * \param[in] individuals the individuals containing the scores.
             * \param[in] nbSelected the number of individuals to select.
             * \param[in] rng required for selecting
             * 
             * \return a vector of the selected individuals.
             */
            virtual std::vector<std::shared_ptr<const Individual>> select(
                const std::set<std::shared_ptr<const Individual>, SharedLess<Evolution::Individual>>& individuals,
                size_t nbSelected, RNG::RNG& rng) const = 0;

            /**
             * \brief method returning the best individual from the given scores
             * 
             * \param[in] individuals the individuals containing the scores.
             * 
             * \return the best individual.
             */
            virtual const Evolution::Individual& getBest(const std::set<std::shared_ptr<const Individual>, SharedLess<Evolution::Individual>>& individuals) const;
    };
};

#endif // SELECTION_H