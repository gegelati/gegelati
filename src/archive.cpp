#include "archive.h"

size_t Archive::getCombinedHash(const std::vector<std::reference_wrapper<DataHandlers::DataHandler>>& dHandlers)
{
	size_t hash = 0;
	for (const std::reference_wrapper<DataHandlers::DataHandler> dHandler : dHandlers) {
		hash ^= dHandler.get().getHash();
	}
	return hash;
}

void Archive::addRecording(const Program::Program* const program, const std::vector<std::reference_wrapper<DataHandlers::DataHandler>>& dHandler, double result)
{
	size_t hash = 0;
	this->recordings.push_back({ program, hash, result });
}
