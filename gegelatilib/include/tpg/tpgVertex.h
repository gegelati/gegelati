#ifndef TPG_VERTEX_H
#define TPG_VERTEX_H

#include <list>

namespace TPG {
	// Declare class to make it usable as an attribute.
	class TPGEdge;

	/**
	* \brief Abstract class representing the vertices of a TPGGraph
	*/
	class TPGVertex {
	public:

		/// Default polymorphic destructor
		virtual ~TPGVertex() = default;

		/**
		* \brief Get a const reference to incoming edges of this TPGVertex.
		*/
		const std::list<TPGEdge*>& getIncomingEdges() const;

		/**
		* \brief Get a const reference to incoming edges of this TPGVertex.
		*/
		const std::list<TPGEdge*>& getOutgoingEdges() const;

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
		* \brief Removes the given incoming edge from the TPGVertex.
		*
		* If the given pointer is NULL or if the given edge is not in the
		* set of the TPGVertex, nothing happens. Otherwise, the edge is
		* removed.
		*
		* \param[in] edge the TPGEdge to remove.
		*/
		virtual void removeIncomingEdge(TPG::TPGEdge* edge);

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

		/**
		* \brief Removes the given outgoing edge from the TPGVertex.
		*
		* If the given pointer is NULL or if the given edge is not in the
		* set of the TPGVertex, nothing happens. Otherwise, the edge is
		* removed.
		*
		* \param[in] edge the TPGEdge to remove.
		*/
		virtual void removeOutgoingEdge(TPG::TPGEdge* edge);

	protected:
		/**
		* \brief Protected default constructor to forbid the instanciation of
		* object of this abstract class.
		*/
		TPGVertex() {};

		/**
		* \brief Set of incoming TPGEdge of the TPGVertex.
		*/
		std::list<TPG::TPGEdge*> incomingEdges;

		/**
		* \brief Set of outgoing TPGEdge of the TPGVertex.
		*/
		std::list<TPG::TPGEdge*> outgoingEdges;
	};
};

#endif