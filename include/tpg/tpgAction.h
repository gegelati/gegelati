#ifndef TPG_ACTION_H
#define TPG_ACTION_H

#include "tpg/tpgVertex.h"

namespace TPG {
	/**
	* \brief Class representing an Action of a TPGGraph.
	*
	* An action is a leaf vertex of a TPG associated to an action of the
	* learning agent within its environment.
	*/
	class TPGAction : public TPGVertex {

		/**
		* \brief Integer number abstracting the selected action.
		*
		* It is up to the used of a TPGGraph to associate the code to each
		* actionID.
		*/
		const uint64_t actionID;

	public:

		/**
		* \brief Main constructor of a TPGAction.
		*
		* \param[in] id integer stored as the actionID of the TPGAction.
		*/
		TPGAction(const uint64_t id) : actionID{ id } {};

		/**
		* \brief Specialization throwing an std::runtime_exception.
		*
		* Since the TPGAction is intented to be a leaf TPGVertex, no outgoing
		* TPGEdge can be added to it.
		*/
		virtual void addOutgoingEdge(TPGEdge* edge);

		uint64_t getActionID() const { return this->actionID; };
	};
};

#endif