#ifndef EVOLUTION_SELECT_STEP_H
#define EVOLUTION_SELECT_STEP_H

#include <memory>
#include <set>
#include <string>
#include <vector>

#include "individual.h"
#include "evolution/module.h"
#include "selection/selector.h"

namespace Evolution {

    /**
     * \brief Step selecting individuals among candidates according to a fitness, and storing them in a list.
     *
     * Used to select the parents (from the population to a list of parents), or the survivors (from the population
     * and the offspring back to the population, each one at most once). Only the fitness of the candidates is used,
     * so the fitness may contain other individuals (e.g. individuals that did not survive).
     */
    class SelectStep : public Module {
        protected:
            /// Names of the lists of candidates.
            std::vector<std::string> candidates;

            /// Name of the fitness used by the selector.
            std::string fitness;

            /// Selector choosing the individuals.
            std::shared_ptr<const Selection::Selector> selector;

            /// Number of selections.
            size_t nbSelected;

            /// Name of the selected individuals, a list of individuals.
            std::string selected;

            /// Indicates if each individual is kept at most once, the selected ones being ordered by ID.
            bool distinct;

        public:
            /**
             * \brief Constructor.
             *
             * \param[in] candidates names of the lists of candidates (list of individuals or list of mutable individuals).
             * \param[in] fitness name of the fitness used by the selector, a fitness map.
             * \param[in] selector selector choosing the individuals.
             * \param[in] nbSelected number of selections.
             * \param[in] selected name of the selected individuals, a list of individuals (e.g. one of the candidates).
             * \param[in] distinct indicates if each individual is kept at most once, the selected ones being ordered by ID.
             * \param[in] name name of the step (e.g. "ParentSelection").
             * \throw std::runtime_error if there is no candidates or the selector is null.
             */
            SelectStep(std::vector<std::string> candidates, std::string fitness, std::shared_ptr<const Selection::Selector> selector, size_t nbSelected,
                       std::string selected, bool distinct = false, std::string name = "Select");

            /// Inherited from Module
            virtual std::string details() const override;
            /// Inherited from Module
            virtual void apply(EvolutionData& state) override;
    };

}; // namespace Evolution

#endif // EVOLUTION_SELECT_STEP_H
