#ifndef TPG_VERTEX_H
#define TPG_VERTEX_H

#include <set>

namespace TPG {
	// Declare class to make it usable as an attribute.
	class TPGEdge;

	/**
	* \brief Abstract class representing the vertices of a TPGGraph
	*/
	class TPGVertex {
	public:
		/**
		* \brief Get a const reference to incoming edges of this TPGVertex.
		*/
		const std::set<TPGEdge*>& getIncomingEdges() const;

		/**
		* \brief Get a const reference to incoming edges of this TPGVertex.
		*/
		const std::set<TPGEdge*>& getOutgoingEdges() const;

		/**
		* \brief Method to add an incoming TPGEdge to the TPGVertex.
		*
		* Since the incomingEdges set is a std::set, the same TPGEdge pointer
		* can not be added twice to the set.
		* This method does not register the TPGVertex as the destination of the
		* TPGEdge.
		*
		* \param[in] edge the TPGEdge pointer to be added to the incomingEdges
		*                 Set.
		*/
		virtual void addIncomingEdge(TPG::TPGEdge* edge);

		/**
		* \brief Method to add an outgoing TPGEdge to the TPGVertex.
		*
		* Since the outgoingEdges set is a std::set, the same TPGEdge pointer
		* can not be added twice to the set.
		* This method does not register the TPGVertex as the spource of the
		* TPGEdge.
		*
		* \param[in] edge the TPGEdge pointer to be added to the outgoingEdges
		*                 Set.
		*/
		virtual void addOutgoingEdge(TPG::TPGEdge* edge);

	protected:
		/**
		* \brief Protected default constructor to forbid the instanciation of
		* object of this abstract class.
		*/
		TPGVertex() {};

		/**
		* \brief Set of incoming TPGEdge of the TPGVertex.
		*/
		std::set<TPG::TPGEdge*> incomingEdges;

		/**
		* \brief Set of outgoing TPGEdge of the TPGVertex.
		*/
		std::set<TPG::TPGEdge*> outgoingEdges;
	};
};

#endif