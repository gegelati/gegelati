#include <algorithm>

#include "data/dataHandler.h"

size_t Data::DataHandler::count = 0;

Data::DataHandler::DataHandler() : id{ count++ }, cachedHash(), invalidCachedHash(true) {
};


size_t Data::DataHandler::getId() const
{
	return this->id;
}

size_t Data::DataHandler::getHash() const
{
	if (this->invalidCachedHash) {
		this->updateHash();
	}

	return this->cachedHash;
}

bool std::operator==(const std::reference_wrapper<const std::type_info>& r0, const std::reference_wrapper<const std::type_info>& r1)
{
	return r0.get() == r1.get();
}


