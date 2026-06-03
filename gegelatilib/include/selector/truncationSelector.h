/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2025) :
 *
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */



#ifndef TRUNCATION_SELECTOR_H
#define TRUNCATION_SELECTOR_H

#include "selector/selector.h"

namespace Selector {

    /**
     * \brief Selection class that will do a selection with a truncation. This
     * is the classic selection methods used in the earliest works of TPGs
     */
    class TruncationSelector : public Selector
    {
      public:
        /**
         * \brief Constructor for Selector.
         *
         * \param[in] graph shared pointer of the graph on which the selection
         * is done.
         * \param[in] params parameters used by the Selector.
         */
        TruncationSelector(std::shared_ptr<TPG::TPGGraph> graph,
                           const Learn::LearningParameters& params)
            : Selector{graph, params}
        {
        }

        /**
         * \brief override of doSelection method
         *
         * Removed the worst agents from the population with a truncation
         * process where the worst proportion set in the parameters is deleted.
         *
         * \param[in,out] results a multimap containing root TPGVertex
         * associated to their score during an evaluation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void doSelection(
            std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                          const TPG::TPGVertex*>& results,
            Mutator::RNG& rng) override;
    };
}; // namespace Selector

#endif // TRUNCATION_SELECTOR_H
