/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019)
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

#ifndef TPG_ACTION_H
#define TPG_ACTION_H

#include <cstdint>

#include "tpg/tpgVertex.h"

namespace TPG {
    /**
     * \brief Class representing an Action of a TPGGraph.
     *
     * An action is a leaf vertex of a TPG associated to an action of the
     * learning agent within its environment.
     */
    class TPGAction : public TPGVertex
    {

        /**
         * \brief Integer number abstracting the ID of selected action. (depending on it class)
         *
         * It is up to the used of a TPGGraph to associate the code to each
         * actionID.
         */
        const uint64_t actionID;

        /**
         * \brief Integer number abstracting the class of selected action.
         *
         * It is up to the used of a TPGGraph to associate the code to each
         * actionClass.
         */
        const uint64_t actionClass;

      public:
        /**
         * \brief Main constructor of a TPGAction.
         *
         * \param[in] actID integer stored as the actionID of the TPGAction.
         * \param[in] actClass integer stored as the actionClass of the TPGAction.
         */
        TPGAction(const uint64_t actID, const uint64_t actClass) : actionID{actID}, actionClass{actClass} {};

        /**
         * \brief Specialization throwing an std::runtime_exception.
         *
         * Since the TPGAction is intented to be a leaf TPGVertex, no outgoing
         * TPGEdge can be added to it.
         */
        virtual void addOutgoingEdge(TPGEdge* edge);

        /**
         * \brief Get the action ID associated to the TPGAction.
         *
         * \return the integer ID of the TPGAction.
         */
        uint64_t getActionID() const
        {
            return this->actionID;
        };

        /**
         * \brief Get the action Class associated to the TPGAction.
         *
         * \return the integer Class of the TPGAction.
         */
        uint64_t getActionClass() const
        {
            return this->actionClass;
        };
    };
}; // namespace TPG

#endif
