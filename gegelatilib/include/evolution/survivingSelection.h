#ifndef SELECTION_H
#define SELECTION_H

#include <set>
#include <algorithm>

#include "evolution/individual.h"
#include "evaluation/evaluationResult.h"

#include "evaluation/scoreMetric.h"

namespace Evolution {

    /**
     * \brief Class representing the surviving selection process of the evolution algorithm.
     */
    class SurvivingSelection {

        protected:            
        
        public: 

            /// Default polymorphic destructor
            virtual ~SurvivingSelection() = default;

            /// @brief Default constructor
            SurvivingSelection() {};
            
            // Disable copying to avoid accidental copies (use references or pointers instead).
            SurvivingSelection(const SurvivingSelection&) = delete;
            SurvivingSelection& operator=(const SurvivingSelection&) = delete;

            /**
             * \brief method returning the evaluation metrics required for the selection process.
             */
            std::vector<std::unique_ptr<Evaluation::EvaluationMetric>> getSelectionMetrics();
            
            /**
             * \brief method ranking the individuals based on their average score on their evaluationRun
             * 
             * \param[in] individuals the individuals containing the scores.
             */
            std::vector<std::pair<double, std::reference_wrapper<const Individual>>> getRankedScores(const std::set<std::reference_wrapper<const Individual>>& individuals) const;

            /**
             * \brief method performing the surviving selection
             * 
             * \param[in] individuals the individuals containing the scores.
             * 
             * \return a map with the individuals as keys. The values indicate if the individual survived (true) or not (false)
             */
            std::map<std::reference_wrapper<const Individual>, bool> select(const std::set<std::reference_wrapper<const Individual>>& individuals) const;

            /**
             * \brief method returning the best individual from the given scores
             * 
             * \param[in] individuals the individuals containing the scores.
             * 
             * \return the best individual.
             */
            const Evolution::Individual& getBest(const std::set<std::reference_wrapper<const Individual>>& individuals) const;
    };
};

#endif // SELECTION_H