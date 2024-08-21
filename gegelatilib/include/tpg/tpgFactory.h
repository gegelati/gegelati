/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2022) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2022)
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

#ifndef TPG_FACTORY_H
#define TPG_FACTORY_H

#include <memory>

#include "archive.h"
#include "tpg/tpgAction.h"
#include "tpg/tpgEdge.h"
#include "tpg/tpgTeam.h"

namespace TPG {

    // Declare the TPGGraph class to be used as a parameter.
    class TPGGraph;

    // Declare the TPGExecutionEngine class to be used as a parameter.
    class TPGExecutionEngine;

    /**
     * \brief Factory for creating all elements constituting a TPG.
     *
     * Using the factory design pattern, this class enables the creation of
     * all elements composing a TPGGraph:
     * - TPGGraph
     * - TPGTeam
     * - TPGAction
     * - TPGVertex
     *
     * The factory also enables the creation of TPGExecutionEngine.
     *
     * This implementation returns the default type for each kind of element.
     */
    class TPGFactory
    {

      public:
        ///  Default virtual destructor.
        virtual ~TPGFactory() = default;

        /**
         * \brief Create a TPGGraph with this TPGFactory.
         *
         * \param[in] env Environment used to build the TPGGraph.
         */
        virtual std::shared_ptr<TPGGraph> createTPGGraph(
            const Environment& env) const;

        /**
         * \brief Create a TPGTeam for a TPGGraph.
         *
         * This method allocates and returns a new TPGTeam.
         */
        virtual TPGTeam* createTPGTeam() const;

        /**
         * \brief Create a TPGAction for a TPGGraph.
         *
         * This method allocates and returns a new TPGAction.
         *
         * \param[in] id integer stored as the actionID of the TPGAction.
         */
        virtual TPGAction* createTPGAction(const uint64_t id) const;

        /**
         * \brief Create a TPGEdge for a TPGGraph.
         *
         * This method allocates and returns a new TPGEdge.
         * The TPGEdge is returned as a unique_ptr.
         *
         * \param[in] src pointer to the source TPGVertex of the edge.
         * \param[in] dest pointer to the destination TPGVertex of the edge.
         * \param[in] prog the shared pointer to the Program associated to the
         *            edge.
         */
        virtual std::unique_ptr<TPGEdge> createTPGEdge(
            const TPGVertex* src, const TPGVertex* dest,
            const std::shared_ptr<Program::Program> prog) const;

        /**
         * \brief Create a TPGExecutionEngine for a TPGGraph produced by this
         * TPGFactory.
         *
         * \param[in] env Environment in which the Program of the TPGGraph will
         * be executed.
         * \param[in] arch pointer to the Archive for storing recordings of the
         * Program Execution. By default, a NULL pointer is given, meaning that
         * no recording of the execution will be made.
         *
         * \return the returned TPGExecutionEngine returned as an unique_ptr.
         */
        virtual std::unique_ptr<TPG::TPGExecutionEngine>
        createTPGExecutionEngine(const Environment& env,
                                 Archive* arch = NULL) const;
    };

} // namespace TPG

#endif // !TPG_GRAPH_ELEMENT_FACTORY_H
