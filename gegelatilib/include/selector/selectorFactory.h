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



#ifndef SELECTOR_FACTORY_H
#define SELECTOR_FACTORY_H

#include "learn/classificationLearningEnvironment.h"
#include "learn/learningEnvironment.h"
#include "tpg/tpgGraph.h"

#include "selector/classificationSelector.h"
#include "selector/mapElites/mapElitesSelector.h"
#include "selector/selector.h"
#include "selector/tournamentSelector.h"
#include "selector/truncationSelector.h"

namespace Selector {

    /**
     * \brief Factory method to create the appropriate Selector
     *
     * \param[in] graph shared pointer of the TPGGraph on which the selection is
     * done.
     * \param[in] le LearningEnvironment used by the LearningAgent.
     * \param[in] params LearningParameters used
     */
    std::shared_ptr<Selector> selectorFactory(
        const std::shared_ptr<TPG::TPGGraph> graph,
        const Learn::LearningEnvironment& le,
        const Learn::LearningParameters& params);

}; // namespace Selector

#endif // SELECTOR_FACTORY_H
