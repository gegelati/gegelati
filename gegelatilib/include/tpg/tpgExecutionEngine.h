/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2022) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
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

#ifndef TPG_EXECUTION_ENGINE_H
#define TPG_EXECUTION_ENGINE_H

#include <set>
#include <vector>

#include "archive.h"
#include "program/programExecutionEngine.h"

#include "tpg/tpgGraph.h"

namespace TPG {
    /**
     * Class in charge of executing a TPGGraph.
     *
     * This first implementation is purely sequential and does not parallelize
     * Program execution, nor executions of the TPG starting from several roots.
     */
    class TPGExecutionEngine
    {
      protected:
        /**
         *  \brief Archive for recording Program results.
         */
        Archive* archive;

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
         * \param[in] env Environment in which the Program of the TPGGraph will
         *                be executed.
         * \param[in] arch pointer to the Archive for storing recordings of
         *                 the Program Execution. By default, a NULL pointer is
         *                 given, meaning that no recording of the execution
         *                 will be made.
         */
        TPGExecutionEngine(const Environment& env, Archive* arch = NULL)
            : progExecutionEngine(env), archive{arch} {};

        ///  Default virtual destructor
        virtual ~TPGExecutionEngine() = default;

        /**
         * \brief Set a new Archive for storing Program results.
         *
         * \param[in] newArchive A pointer (possibly NULL) to an Archive.
         */
        void setArchive(Archive* newArchive);

        /**
         * \brief Execute the Program associated to an Edge and returns the
         * obtained double.
         *
         * If an Archive is associated to the TPGExecutionEngine, the Program
         * result is recorded in it.
         *
         * If the value returned by the Program is NaN, then it is replaced with
         * a -inf value.
         *
         * \param[in] edge the const ref to the TPGEdge whose Program will be
         * evaluated.
         * \return the double value returned by the Program of the TPGEdge.
         */
        virtual double evaluateEdge(const TPGEdge& edge);

        /**
         * \brief Execute all the Program of the outgoing TPGEdge of the
         * TPGTeam.
         * 
         * This method evaluates the Programs of all outgoing TPGEdge of the
         * TPGTeam, and activates the nbEdgesActivated of TPGEdge providing the
         * largest evaluation.
         * 
         * Only one team can be activated while an unlimited number of actions
         * can be activated.
         * 
         * The action value of an activated action is saved if the action ID is activated for the first time.
         * If a team is activated, it is execute recursively.
         *
         *
         * \param[in] currentTeam the TPGTeam whose outgoing TPGEdge are evaluated.
         * \param[in] visitedTeams vector of the different visited TPGTeam seen during this root execution
         * \param[in] actionsTaken vector of int64_t of actions taken.
         * \param[in] nbEdgesActivated number of TPGEdge that can be activated.
         *
         */
          virtual void executeTeam(
                const TPG::TPGVertex* currentTeam, std::vector<const TPG::TPGVertex*>& visitedTeams,
                std::vector<std::int64_t>* actionsTaken, uint64_t nbEdgesActivated);

        /**
         * \brief Execute the TPGGraph starting from the given TPGVertex.
         *
         * This method browse the graph by successively evaluating Teams and
         * following the TPGEdge proposing the best bids.
         *
         * \param[in] root the TPGVertex from which the execution will start.
         * \param[in] nbActionsID the number of action ID that can be choosen by the root.
         * \param[in] nbEdgesActivated the number of edges that can be activated by team. A team can only activate a single other team.
         * \return a vector containing all the TPGVertex traversed during the
         *         evaluation of the TPGGraph. The TPGAction resulting from the
         *         TPGGraph execution is at the end of the returned vector.
         */
        virtual const std::vector<int64_t> executeFromRoot(
            const TPGVertex& root, uint64_t nbActionsID, uint64_t nbEdgesActivated);
    };
}; // namespace TPG

#endif
