#include <stdexcept>

#include "learn/learningEnvironment.h"

Learn::LearningEnvironment* Learn::LearningEnvironment::clone() const
{
	return NULL;
}

bool Learn::LearningEnvironment::isCopyable() const
{
	return false;
}

void Learn::LearningEnvironment::doAction(uint64_t actionID)
{
	if (actionID >= this->nbActions) {
		throw std::runtime_error("Given action ID exceeds the number of actions for this learning environment.");
	}
}
