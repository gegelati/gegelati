#ifndef SUPPORTED_TYPE_H
#define SUPPORTED_TYPE_H

#include <typeinfo>

namespace Data {
	/**
	* \brief Base class to declare all types of Instruction operands, and provided by HandlerData.
	*/
	class SupportedType {
	public:
		/**
		* \brief Virtual destructor to force class polymorphism.
		*/
		virtual ~SupportedType() = default;
	};
}

#endif

