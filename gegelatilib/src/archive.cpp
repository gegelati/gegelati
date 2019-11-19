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

const ArchiveRecording& Archive::at(uint64_t n) const
{
	return this->recordings.at(n);
}

void Archive::setRandomSeed(size_t newSeed)
{
	this->randomEngine.seed(newSeed);
}

void Archive::addRecording(const Program::Program* const program, const std::vector<std::reference_wrapper<DataHandlers::DataHandler>>& dHandler, double result, bool forced)
{
	// Archive according to probability
	if (forced || this->archivingProbability == 1.0 || this->archivingProbability <= std::uniform_real_distribution<double>(0.0, 1.0)(this->randomEngine)) {
		// get the combined hash
		size_t hash = getCombinedHash(dHandler);

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
}

bool Archive::hasDataHandlers(const size_t& hash) const
{
	return this->dataHandlers.count(hash) != 0;
}

bool Archive::areProgramResultsUnique(std::map<size_t, double> hashesAndResults, double tau) const
{
	// Check programs until one is equivalent or until all have been checked.
	for (auto programRecordings : this->recordingsPerProgram) {
		// check all recordings "presence" within the hashesAndResults map.
		bool isIdentical = false;
		for (const auto& recording : programRecordings.second) {
			// For each recording there are three possibilities
			// 1- there is no result for this hash in the Map
			//    > Nothing to do for this recording
			// 2- there is a different result in the Map
			//    > Put isIdentical to false and stop browsing the recordings for this program.
			// 3- there is an "identical" (within tau margin) result in the Map
			//    > Put the isIdentical to true. If at the end of all recordings the isIdentical is true
			//    > The program bid behavior is marked as equivalent.
			auto iter = hashesAndResults.find(recording.dataHash);
			if (iter != hashesAndResults.end()) {
				// Cases 2 & 3
				if (std::abs(iter->second - recording.result) <= tau) {
					// results are equivalent
					isIdentical = true;
				}
				else {
					isIdentical = false;
					break; // break for recordings loop
				}
			}
			else {
				// Case 1 > do nothing
			}
		}

		// If isIdentical is 1 => Programs have equivalent bidding behaviour
		if (isIdentical) {
			return false;
		} // else, go to the next Program comparison
	}

	return true;
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