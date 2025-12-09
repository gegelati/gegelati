/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
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

#ifndef OLD_TPG_EXECUTION_ENGINE_H
#define OLD_TPG_EXECUTION_ENGINE_H

#include <set>
#include <vector>
	

#include "archive.h"
#include "program/programExecutionEngine.h"

#include "evoGraph/graph.h"

namespace EvoGraph {
    /**
     * Class in charge of executing a Graph.
     *
     * This first implementation is purely sequential and does not parallelize
     * Program execution, nor executions of the TPG starting from several roots.
     */
    class OldExecutionEngine
    {
      protected:
        /**
         *  \brief Archive for recording Program results.
         */
        Archive* archive;

        /**
         * \brief Environment used
         */
        const Environment& env;

        /**
         * \brief ProgramExecutionEngine for executing Programs of edges.
         *
         * Keeping this ProgramExecutionEngine as an attribute avoids wasting
         * time rebuilding a new one for each edge.
         */
        Program::ProgramExecutionEngine progExecutionEngine;

      public:
        /**
         * \brief Main constructor of the class.
         *
         * \param[in] env Environment in which the Program of the Graph will
         *                be executed.
         * \param[in] arch pointer to the Archive for storing recordings of
         *                 the Program Execution. By default, a NULL pointer is
         *                 given, meaning that no recording of the execution
         *                 will be made.
         */
        OldExecutionEngine(const Environment& env, Archive* arch = NULL)
            : progExecutionEngine(env), env{env}, archive{arch} {};

        ///  Default virtual destructor
        virtual ~OldExecutionEngine() = default;

        /**
         * \brief Set a new Archive for storing Program results.
         *
         * \param[in] newArchive A pointer (possibly NULL) to an Archive.
         */
        void setArchive(Archive* newArchive);

        /**
         * \brief Apply a sigmoid function on all the actions
         *
         * \param[in] actionsTaken a reference to the action taken
         */
        void applyActivationFunctionOnActions(
            std::vector<double>& actionsTaken);

        /**
         * \brief Execute the Program associated to an Edge and returns the
         * obtained double.
         *
         * If an Archive is associated to the OldExecutionEngine, the Program
         * result is recorded in it.
         *
         * If the value returned by the Program is NaN, then it is replaced with
         * a -inf value.
         *
         * \param[in] edge the const ref to the Edge whose Program will be
         * evaluated.
         * \return the double value returned by the Program of the Edge.
         */
        virtual double evaluateEdge(const Edge& edge);

        /**
         * \brief Evaluate all the Program of the outgoing Edge of the
         *        Team.
         *
         * This method evaluates the Programs of all outgoing Edge of the
         * Team, and returns the reference to the Edge providing the
         * largest evaluation.
         *
         * \param[in] team the Team whose outgoing Edge are evaluated.
         * \return the reference to the Edge evaluated with the the highest
         *         double value (and not excluded).
         *
         * \throw std::runtime_error in case the Team has no outgoing edge.
         * This should not happen in a correctly constructed Graph.
         */
        virtual const EvoGraph::Edge& evaluateTeam(const Team& team);

        /**
         * \brief Execute the Graph starting from the given Vertex.
         *
         * This method browse the graph by successively evaluating Teams and
         * following the Edge proposing the best bids.
         *
         * \param[in] root the Vertex from which the execution will start.
         * \param[in] initActions the vector of initial action that can be
         * chosen by default by the root.
         * \return a vector containing all the Vertex traversed during the
         *         evaluation of the Graph. The Action resulting from the
         *         Graph execution is at the end of the returned vector.
         */
        virtual const std::pair<std::vector<const EvoGraph::Vertex*>,
                                std::vector<double>>
        executeFromRoot(const Vertex& root,
                        const std::vector<uint64_t>& initActions = {0});
    };
}; // namespace EvoGraph

#endif
