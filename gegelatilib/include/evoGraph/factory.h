/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2022 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2022)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2024 - 2025)
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
#include "evoGraph/action.h"
#include "evoGraph/actionEdge.h"
#include "evoGraph/edge.h"
#include "evoGraph/team.h"

namespace EvoGraph {

    // Declare the Graph class to be used as a parameter.
    class Graph;

    // Declare the ExecutionEngine class to be used as a parameter.
    class ExecutionEngine;

    /**
     * \brief Factory for creating all elements constituting a TPG.
     *
     * Using the factory design pattern, this class enables the creation of
     * all elements composing a Graph:
     * - Graph
     * - TPGTeam
     * - Action
     * - Vertex
     *
     * The factory also enables the creation of ExecutionEngine.
     *
     * This implementation returns the default type for each kind of element.
     */
    class GraphFactory
    {

      public:
        ///  Default virtual destructor.
        virtual ~GraphFactory() = default;

        /**
         * \brief Create a Graph with this GraphFactory.
         *
         * \param[in] env Environment used to build the Graph.
         */
        virtual std::shared_ptr<Graph> createGraph(
            const Environment& env) const;

        /**
         * \brief Create a TPGTeam for a Graph.
         *
         * This method allocates and returns a new TPGTeam.
         */
        virtual std::unique_ptr<TPGTeam> createTPGTeam() const;

        /**
         * \brief Create a Action for a Graph.
         *
         * This method allocates and returns a new Action.
         *
         * \param[in] id integer stored as the actionID of the Action.
         */
        virtual std::unique_ptr<Action> createAction(
            const uint64_t id) const;

        /**
         * \brief Create a Edge for a Graph.
         *
         * This method allocates and returns a new Edge.
         * The Edge is returned as a unique_ptr.
         *
         * \param[in] src pointer to the source Vertex of the edge.
         * \param[in] dest pointer to the destination Vertex of the edge.
         * \param[in] prog the shared pointer to the Program associated to the
         *            edge.
         */
        virtual std::unique_ptr<Edge> createEdge(
            const Vertex* src, const Vertex* dest,
            const std::shared_ptr<Program::Program> prog) const;

        /**
         * \brief Create a ActionEdge for a Graph.
         *
         * This method allocates and returns a new ActionEdge cat into a
         * Edge. The Edge is returned as a unique_ptr.
         *
         * \param[in] src pointer to the source Vertex of the edge. It must
         * be an action.
         * \param[in] prog the shared pointer to the Program associated to
         * the edge.
         * \param[in] actionClass of the actionEdge
         */
        virtual std::unique_ptr<Edge> createActionEdge(
            const Vertex* src, const std::shared_ptr<Program::Program> prog,
            uint64_t actionClass) const;

        /**
         * \brief Create a ExecutionEngine for a Graph produced by this
         * GraphFactory.
         *
         * \param[in] env Environment in which the Program of the Graph will
         * be executed.
         * \param[in] arch pointer to the Archive for storing recordings of the
         * Program Execution. By default, a NULL pointer is given, meaning that
         * no recording of the execution will be made.
         *
         * \return the returned ExecutionEngine returned as an unique_ptr.
         */
        virtual std::unique_ptr<EvoGraph::ExecutionEngine>
        createExecutionEngine(const Environment& env,
                                 Archive* arch = NULL) const;
    };

} // namespace EvoGraph

#endif // !TPG_GRAPH_ELEMENT_FACTORY_H
