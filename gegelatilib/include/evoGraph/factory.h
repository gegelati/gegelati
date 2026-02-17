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
#include "evoGraph/edge.h"
#include "evoGraph/team.h"

namespace EvoGraph {

    // Declare the Graph class to be used as a parameter.
    class Graph;

    /**
     * \brief Factory for creating all elements constituting a TPG.
     *
     * Using the factory design pattern, this class enables the creation of
     * all elements composing a Graph:
     * - Graph
     * - Team
     * - Action
     * - Vertex
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
         */
        virtual std::shared_ptr<Graph> createGraph() const;

        /**
         * \brief Create a Team for a Graph.
         *
         * This method allocates and returns a new Team.
         * \param[in] programAgent weak pointer to the Agent Program associated to the newly
         *                 created Team.
         */
        virtual std::shared_ptr<Team> createTeam(std::optional<std::reference_wrapper<const Algorithm::Agent>> programAgent = std::nullopt) const;

        /**
         * \brief Create a Action for a Graph.
         *
         * This method allocates and returns a new Action.
         *
         * \param[in] id integer stored as the actionID of the Action.
         * \param[in] programAgent weak pointer to the Agent Program associated to the newly
         *                 created Action.
         */
        virtual std::shared_ptr<Action> createAction(
            const uint64_t id, std::optional<std::reference_wrapper<const Algorithm::Agent>> programAgent = std::nullopt) const;

        /**
         * \brief Create a Edge for a Graph.
         *
         * This method allocates and returns a new Edge.
         * The Edge is returned as a shared_ptr.
         *
         * \param[in] src pointer to the source Vertex of the edge.
         * \param[in] dest pointer to the destination Vertex of the edge.
         * \param[in] agentProgram the weak pointer to the agentProgram associated to the
         *            edge.
         */
        virtual std::shared_ptr<Edge> createEdge(
            std::shared_ptr<const Vertex> src, std::shared_ptr<const Vertex> dest,
            std::optional<std::reference_wrapper<const Algorithm::Agent>> agentProgram = std::nullopt) const;


    };

} // namespace EvoGraph

#endif // !TPG_GRAPH_ELEMENT_FACTORY_H
