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

#include "selector/selectorFactory.h"

std::shared_ptr<Selector::Selector> Selector::selectorFactory(
    const std::shared_ptr<TPG::TPGGraph> graph,
    const Learn::LearningEnvironment& le,
    const Learn::LearningParameters& params)
{

    // Create the corresponding selector
    if (dynamic_cast<const Learn::ClassificationLearningEnvironment*>(&le) !=
        nullptr) { // TODO better way for classifier
        return std::make_shared<ClassificationSelector>(graph, params,
                                                        le.getNbActions());
    }
    else if (params.selection._selectionMode == "truncation") {
        return std::make_shared<TruncationSelector>(graph, params);
    }
    else if (params.selection._selectionMode == "tournament") {
        return std::make_shared<TournamentSelector>(graph, params);
    }
    else if (params.selection._selectionMode == "mapElites") {
        if (params.mutation.tpg.ratioTeamsOverActions != 0.0 &&
            params.mutation.tpg.ratioTeamsOverActions != 1.0) {
            throw std::runtime_error(
                "MapElitesSelector currently does not support dual population");
        }
        return std::make_shared<MapElitesSelector>(graph, params);
    }
    else {
        throw std::runtime_error("Selection mode not found");
    }
}
