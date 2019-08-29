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
		const Program::Program& getProgram() const;

		/**
		* \brief Get a reference to the Program of the TPGEdge.
		*
		* \return a reference to the Program of the TPGEdge.
		*/
		Program::Program& getProgram();

		/**
		* \brief Set a new Program for the TPGEdge.
		*
		* \param[in] prog the new shared pointer to a Program.
		*/
		void setProgram(const std::shared_ptr<Program::Program> prog);

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
		std::shared_ptr<Program::Program> program;

		/// Delete the default constructor.
		TPGEdge() = delete;
	};
};

#endif