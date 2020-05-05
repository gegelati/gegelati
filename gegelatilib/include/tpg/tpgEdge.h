/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019)
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

#ifndef TPG_EDGE_H
#define TPG_EDGE_H

#include <memory>

#include "program/program.h"

namespace TPG {
	// Declare class to make it usable as an attribute.
	class TPGVertex;

	/**
	* \brief Class representing edges of the Tangled Program Graphs.
	*/
	class TPGEdge {
	public:
		/**
		* \brief Main constructor of the TPGEdge class.
		*
		* This constructor does not register the created TPGEdge in the
		* list of incoming or outgoing edges of the given TPGVertex.
		*
		* \param[in] src pointer to the source TPGVertex of the edge.
		* \param[in] dest pointer to the destination TPGVertex of the edge.
		* \param[in] prog the shared pointer to the Program associated to the
		*            edge.
		*/
		TPGEdge(const TPGVertex* src, const TPGVertex* dest,
			const std::shared_ptr<Program::Program> prog) :
			source{ src }, destination{ dest }, program{ prog } {};

		/**
		* \brief Get a const reference to the Program of the TPGEdge.
		*
		* \return a const reference to the Program of the TPGEdge.
		*/
		Program::Program& getProgram() const;

		/**
		* \brief Set a new Program for the TPGEdge.
		*
		* \param[in] prog the new shared pointer to a Program.
		*/
		void setProgram(const std::shared_ptr<Program::Program> prog) const;

		/**
		* \brief Get the shared_pointer to the Program.
		*
		* This method is voluntarily non-const to make sure that only the 
		* TPGGraph containing the edge can use it.
		*
		* \return a copy of the program attribute.
		*/
		std::shared_ptr<Program::Program> getProgramSharedPointer();

		/**
		* \brief Get the source TPGVertex of the TPGEdge.
		*
		* \return a const pointer to the source TPGVertex.
		*/
		const TPGVertex* getSource() const;

		/**
		* \brief Set a new source TPGVertex to the TPGEdge.
		*
		* \param[in] newSource the new TPGVertex  to register as the source.
		*/
		void setSource(TPGVertex* newSource);

		/**
		* \brief Get the destination TPGVertex of the TPGEdge.
		*
		* \return a const pointer to the destination TPGVertex.
		*/
		const TPGVertex* getDestination() const;

		/**
		* \brief Set a new destination TPGVertex to the TPGEdge.
		*
		* \param[in] newDestination the new TPGVertex to register as the destination.
		*/
		void setDestination(TPGVertex* newDestination);

	protected:
		/// Pointer to the source TPGVertex of this TPGEdge
		const TPGVertex* source;

		/// Pointer to the destination TPGVertex of this TPGEdge
		const TPGVertex* destination;

		/// Shared pointer to the Program to execute when evaluating the bid 
		/// of this TPGEdge.
		mutable std::shared_ptr<Program::Program> program;

		/// Delete the default constructor.
		TPGEdge() = delete;
	};
};

#endif
