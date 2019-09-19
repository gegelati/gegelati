#include <math.h>

#include "archive.h"

Archive::~Archive()
{
	for (auto dHandlerAndHash : this->dataHandlers) {
		for (auto dHandler : dHandlerAndHash.second) {
			// Free memory of DataHandlers within the archive
			delete& dHandler.get();
		}
	}

}

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
	// get the combined hash
	size_t hash = getCombinedHash(dHandler);

	// Check is an identical recording (same hash, same program) already exists.
	// Program may be different
	if (this->isRecordingExisting(hash, program)) {
		return;
	}

	// Check if dataHandler copy is needed.
	if (this->dataHandlers.find(hash) == this->dataHandlers.end()) {
		// Store a copy of data handlers.
		std::vector<std::reference_wrapper<DataHandlers::DataHandler>> dHandlersCpy;
		for (std::reference_wrapper<DataHandlers::DataHandler> dh : dHandler) {
			DataHandlers::DataHandler* dhCopy = dh.get().clone();
			dHandlersCpy.push_back(*dhCopy);
		}
		// Create the map entry
		this->dataHandlers.emplace(hash, std::move(dHandlersCpy));
	}

	// Create and stores the recording
	ArchiveRecording recording{ program, hash, result };
	this->recordings.push_back(recording);

	// Update the recordings per Program
	auto iterNbRecordings = this->recordingsPerProgram.find(program);
	if (iterNbRecordings != this->recordingsPerProgram.end()) {
		iterNbRecordings->second.push_back(recording);
	}
	else {
		this->recordingsPerProgram.insert({ program, {recording} });
	}

	// Check if Archive max size was reached (or exceeded)
	while (this->recordings.size() > this->maxSize) {

		// Get the recording (copy)
		ArchiveRecording rec = this->recordings.front();
		// Remove the first recording
		this->recordings.pop_front();

		// Check if this DataHandler (hash) is still used in other recordings
		bool stillUsed = (std::find_if(this->recordings.begin(), this->recordings.end(),
			[&rec](ArchiveRecording r) {return r.dataHash == rec.dataHash; })) != this->recordings.end();

		// if not, remove it from the Archive also
		if (!stillUsed) {
			// Free memory of DataHandlers within the archive
			for (std::reference_wrapper<DataHandlers::DataHandler> toErase : this->dataHandlers.at(rec.dataHash)) {
				delete& toErase.get();
			}

			// Remove the entry from the map
			this->dataHandlers.erase(rec.dataHash);
		}

		// Update the recordingsPerProgram of the corresponding Program,
		// and remove it if it was the last.
		auto iter = this->recordingsPerProgram.find(rec.prog);
		iter->second.pop_front();
		if (iter->second.size() == 0) {
			this->recordingsPerProgram.erase(iter);
		}
	}
}

bool Archive::hasDataHandlers(const size_t& hash) const
{
	return this->dataHandlers.count(hash) != 0;
}

bool Archive::isRecordingExisting(
	size_t hash,
	const Program::Program* prog) const
{
	// If the hash does not exist, the result is unique since no recordings 
	// correspond to it.
	if (!hasDataHandlers(hash)) {
		return false;
	}

	// Else, check the recordings with this hash.
	auto equalityTester = [&hash, &prog](const ArchiveRecording& rec) {
		return hash == rec.dataHash && prog == rec.prog;
	};

	std::deque<ArchiveRecording>::const_iterator position = std::find_if(this->recordings.begin(), this->recordings.end(), equalityTester);
	return  position != this->recordings.end();
}

bool Archive::areProgramResultsUnique(std::map<size_t, double> hashesAndResults, double tau) const
{
	return false;
}

size_t Archive::getNbRecordings() const
{
	return this->recordings.size();
}

size_t Archive::getNbDataHandlers() const
{
	return this->dataHandlers.size();
}

const std::map<size_t, std::vector<std::reference_wrapper<DataHandlers::DataHandler>>>& Archive::getDataHandlers() const
{
	return this->dataHandlers;
}

void Archive::clear()
{
	for (auto dHandlerAndHash : this->dataHandlers) {
		for (auto dHandler : dHandlerAndHash.second) {
			// Free memory of DataHandlers within the archive
			delete& dHandler.get();
		}
	}

	this->dataHandlers.clear();
	this->recordings.clear();
	this->recordingsPerProgram.clear();
}
