#include <algorithm>
#include "dataHandlers/dataHandler.h"

bool DataHandlers::DataHandler::canHandle(const std::type_info& type) const
{
	for (auto t : this->providedTypes) {
		if (t.get() == type)
			return true;
	}

	return false;
}

const std::vector<std::reference_wrapper<const std::type_info>>& DataHandlers::DataHandler::getHandledTypes() const
{
	return this->providedTypes;
}

bool std::operator==(const std::reference_wrapper<const std::type_info>& r0, const std::reference_wrapper<const std::type_info>& r1)
{
	return r0.get() == r1.get();
}
