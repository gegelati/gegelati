#ifndef SELECTION_H
#define SELECTION_H

#include <map>
#include <algorithm>

#include "evolution/individual.h"
#include "evaluation/evaluationResult.h"

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
             * \brief method performing the surviving selection
             * 
             * \param[in] scores the scores of the individuals' evaluation.
             * 
             * \return a map with the individuals as keys. The values indicate if the individual survived (true) or not (false)
             */
            std::map<std::reference_wrapper<const Individual>, bool> select(const std::map<std::reference_wrapper<const Individual>, std::shared_ptr<Evaluation::EvaluationResult>>& scores) const;

            /**
             * \brief method returning the best individual from the given scores
             * 
             * \param[in] scores the scores of the individuals' evaluation.
             * 
             * \return the best individual.
             */
            const Evolution::Individual& getBest(const std::map<std::reference_wrapper<const Individual>, std::shared_ptr<Evaluation::EvaluationResult>>& scores) const;
    };
};

#endif // SELECTION_H