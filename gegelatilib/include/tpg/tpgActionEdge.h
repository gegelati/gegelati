/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Mickaël Dardaillon <mdardail@insa-rennes.fr> (2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019)
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

#ifndef TPG_ACTION_EDGE_H
#define TPG_ACTION_EDGE_H

#include <memory>

#include "tpg/tpgEdge.h"

namespace TPG {
    // Declare class to make it usable as an attribute.
    class TPGVertex;

    /**
     * \brief Class representing ActionEdges of the Tangled Program Graphs.
     *
     * The source of an TPGActionEdge can not be a TPGTeam, only a TPGAction.
     * A TPGActionEdge does not have a destination.
     *
     * It also has an action class corresponding to the continuous actions class
     * it is assessing. This action class is only used for multi-action program
     * cases.
     */
    class TPGActionEdge : public TPGEdge
    {
      public:
        /**
         * \brief Main constructor of the TPGEdge class.
         *
         * This constructor does not register the created TPGEdge in the
         * list of incoming or outgoing edges of the given TPGVertex.
         *
         * \param[in] src pointer to the source TPGVertex of the edge.
         * \param[in] prog the shared pointer to the Program associated to the
         * \param[in] actClass actionClass of the TPGActionEdge
         *            edge.
         */
        TPGActionEdge(const TPGVertex* src,
                      const std::shared_ptr<Program::Program> prog,
                      uint64_t actClass)
            : TPGEdge{src, NULL, prog}, actionClass(actClass){};

        /**
         * \brief Override of the tpgEdge function because there should not be
         * destination to action edge
         */
        const TPGVertex* getDestination() const override;

        /**
         * \brief Override of the tpgEdge function because there should not be
         * destination to action edge
         */
        void setDestination(TPGVertex* newDestination) override;

        /**
         * \brief set a new action class
         *
         * \param[in] newActionClass new action
         */
        void setActionClass(uint64_t newActionClass);

        /**
         * \brief get the action class
         */
        uint64_t getActionClass() const;

        /// Delete the default constructor.
        TPGActionEdge() = delete;

      protected:
        /// @brief action class of the edge
        uint64_t actionClass;
    };
}; // namespace TPG

#endif
