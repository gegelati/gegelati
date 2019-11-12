#include <algorithm>
#include "dataHandlers/dataHandler.h"

size_t DataHandlers::DataHandler::count = 0;

DataHandlers::DataHandler::DataHandler() : id{ count++ }, providedTypes(), cachedHash(), invalidCachedHash(true) {
};

size_t DataHandlers::DataHandler::getId() const
{
	return this->id;
}

size_t DataHandlers::DataHandler::getHash() const
{
	if (this->invalidCachedHash) {
		this->updateHash();
	}

	return this->cachedHash;
}

bool DataHandlers::DataHandler::canHandle(const std::type_info& type) const
{
	for (auto & t : this->providedTypes) {
		if (t.get() == type)
			return true;
	}

	return false;
}

const std::vector<std::reference_wrapper<const std::type_info>>& DataHandlers::DataHandler::getHandledTypes() const
{
	return this->providedTypes;
}

size_t DataHandlers::DataHandler::getLargestAddressSpace() const
{
	size_t maxSize = 0;

	// Scan data types
	for (auto type : this->getHandledTypes()) {
		size_t size = this->getAddressSpace(type);
		maxSize = (size > maxSize) ? size : maxSize;
	}

	return maxSize;
}

bool std::operator==(const std::reference_wrapper<const std::type_info>& r0, const std::reference_wrapper<const std::type_info>& r1)
{
	return r0.get() == r1.get();
}
