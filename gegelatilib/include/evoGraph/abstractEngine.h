/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2021) :
 *
 * Thomas Bourgoin <tbourgoi@insa-rennes.fr> (2021)
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

#ifndef TPG_ABSTRACT_ENGINE_H
#define TPG_ABSTRACT_ENGINE_H

#include "program/program.h"
#include "evoGraph/graph.h"

namespace EvoGraph {
    /**
     * \brief Abstract Class in charge of managing maps to give a unique ID
     * for vertex and a program of a Graph.
     *
     */
    class AbstractEngine
    {

      protected:
        /**
         * \brief Reference to the Graph whose content will be used to fill
         * the maps.
         */
        const EvoGraph::Graph& tpg;

        /**
         * \brief Set of all program ID.
         */
        std::set<uint64_t> programID;

        /**
         * \brief Integer number used during export to associate a unique
         * integer identifier to each Action.
         *
         * Identifier associated to Action are NOT preserved during multiple
         * printing of a Graph.
         */
        uint64_t nbActions;

        /**
         * \brief Constructor for the abstract engine.
         *
         * \param[in] tpg const reference to the graph whose content will be
         * used to fill the maps of IDs  (vertex and program).
         */

        AbstractEngine(const EvoGraph::Graph& tpg) : tpg{tpg}, nbActions{0} {};

      public:
        /**
         * \brief Method to find if the given Program is already in the
         * programID set.
         *
         * \return A boolean value indicating whether the returned ID is a new
         * one (true), or one found in the programID map (false).
         */

        bool programIDIsNew(const uint64_t& progID);
    };
} // namespace EvoGraph
#endif // TPG_ABSTRACT_ENGINE_H
