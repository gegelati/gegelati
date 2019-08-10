#include "environment.h"

size_t Environment::computeLargestAddressSpace(const std::vector<std::reference_wrapper<DataHandlers::DataHandler>>& dHandlers)
{
	size_t res{ 0 };
	for (auto dHandler : dHandlers) {
		size_t addressSpace = dHandler.get().getLargestAddressSpace();
		res = (addressSpace > res) ? addressSpace : res;
	}
	return res;
}
