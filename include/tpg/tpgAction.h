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
	class TPGAction: public TPGVertex {
		
	public:
		/**
		* \brief Specialization throwing an std::runtime_exception.
		*
		* Since the TPGAction is intented to be a leaf TPGVertex, no outgoing
		* TPGEdge can be added to it.
		*/
		virtual void addOutgoingEdge(TPGEdge* edge);
	};
};

#endif