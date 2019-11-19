#ifndef ARCHIVE_H
#define ARCHIVE_H

#include <random>
#include <map>
#include <deque>

#include "dataHandlers/dataHandler.h"
#include "program/program.h"

/**
* \brief Class used to store one recording of an Archive.
*
* A recording in the archive is a tuple consisting of:
* - A Program pointer (that may not exist anymore)
* - A set of DataHandler copies with all their data.
* - A double resulting from the execution of the Program on the DataHandler.
*/
typedef struct ArchiveRecording {
	/// Pointer to the Program. This pointer may point to a freed program.
	const Program::Program* const prog;

	/// Hash of the set of DataHandler for this recording
	const size_t dataHash;

	/// Value returned by the Program for the DataHandler with the specified 
	/// hash.
	const double result;
} ArchiveRecording;

/**
* Class use to manage the Archive associating input DataHandler and Program to
* the results they produced during execution.
*
* This Archive is used when mutating a Program to perform the neutrality test
* which requires a Mutated program to produce an original result compared to
* any Program still in the Archive.
*
*/
class Archive {
protected:
	/// Maximum number of recordings held in the Archive.
	const size_t maxSize;

	/**
	* \brief Randomness engine for archiving.
	*
	* This randomness engine is used to ensure determinism of the archiving
	* process even in parallel execution context.
	* The randomness engine should be reset with a new seed before entering a
	* parallelizable part of the computations (even if these computations are
	* done sequentially). As a more concrete example, if each policy starting
	* from a root TPGVertex in a TPGGraph is evaluated in parallel, the
	* randomEngine should be reset before each root.
	*/
	std::mt19937_64 randomEngine;

	/**
	* \brief Storage for DataHandler copies used in recordings.
	*
	* This map associates a hash values with the corresonding copy of the set
	* of DataHandler that produced this value. The hash value is used in
	* recordings to associate each recording to the right copy of the
	* DataHandler.
	*/
	std::map<size_t, std::vector<std::reference_wrapper<DataHandlers::DataHandler>>> dataHandlers;

	/**
	* \brief Map storing the Program pointers referenced in recordings the
	* associated recording.
	*
	* The Map is filled in the addRecording method, and elements are removed
	* whenever the las ArchiveRecording referencing a Program is removed from
	* the Archive.
	*
	* The Map is used to speed the unicity tests.
	*/
	std::map<const Program::Program*, std::deque<ArchiveRecording>> recordingsPerProgram;

	/// Recordings of the Archive
	std::deque<ArchiveRecording> recordings;

	/**
	* \brief Probability of adding any program execution to the archive.
	*/
	const double archivingProbability;

public:
	/**
	* \brief Main constructor for Archive.
	*
	* \param[in] archivingProbability probability for each call to
	* addRecording to actually lead to a new recodring in the Archive.
	* \param[in] size maximum number of recordings kept in the Archive.
	* \param[in] initialSeed Seed value for the randomEngine.
	*/
	Archive(size_t size = 50, double archivingProbability = 1.0, size_t initialSeed = 0) : archivingProbability{ archivingProbability }, maxSize{ size }, recordings(), randomEngine(initialSeed) {};

	/**
	* \brief Destructor of the class.
	*
	* In addition to default behavior, free all the memory associated to the
	* referenced DataHandler in the dataHandlers attribute.
	*/
	~Archive();

	/**
	* \brief Combien the hash of a set of dataHandlers into a single one.
	*
	* Hashes of each DataHandler is accessed with the
	* DataHandler::getHash() method.
	*
	* \return the hash resulting from the combination.
	*/
	static size_t getCombinedHash(const std::vector<std::reference_wrapper<DataHandlers::DataHandler>>& dHandler);

	/**
	* \brief Set a new seed for the randomEngine.
	*
	* \param[in] newSeed Set a new seed for the random engine.
	*/
	void setRandomSeed(size_t newSeed);

	/**
	* \brief Add a new recording to the Archive.
	*
	* A call to this function adds an ArchiveRecording to the archive with the
	* probability specified by the archivingProbability attribute.
	* If the maximum number of recordings held in the archive is reached, the
	* oldest recording will be removed.
	* If this is the first time this set of DataHandler is stored in the
	* Archive according to its DataHandler::getHash() method, a copy of the
	* dataHandler will be created.
	* If an identical recording is already in the Archive (same hash, same
	* Program), the recording is not added.
	*
	* \param[in] program the Program associated to this recording.
	* \param[in] dHandler the set of dataHandler the Program worked on to
	*                     generate the associated result.
	* \param[in] result double value produced by the Program.
	*/
	virtual void addRecording(const Program::Program* const program,
		const std::vector<std::reference_wrapper<DataHandlers::DataHandler>>& dHandler,
		double result);

	/**
	* \brief Check whether the given hash is already in the archive.
	*
	* \param[in] hash the DataHandler hash whose presence will be tested.
	* \return true if the given hash is already in the
	*         Archive, false otherwise.
	*/
	bool hasDataHandlers(const size_t& hash) const;

	/**
	* \brief Check if a recording exists for the given Program and DataHandler.
	*
	* Check if there exists a recording in the Archive for the pair of DataHandler hash
	* and the given Program.
	*
	* \param[in] hash The hash of the set of DataHandler for the searched recording.
	* \param[in] prog the Program of the searched Recording.
	* \return whether the check was successful or not.
	*/
	virtual bool isRecordingExisting(
		size_t hash,
		const Program::Program* prog) const;

	/**
	* Check if the given hash-results pairs are unique compared to Program in
	* the Archive.
	*
	* This method will return false is there exist any Program in the Archive
	* for which all recordings with hashes contained in the given map, are
	* associated to results equal to those of the given map (within tau
	* margin).
	*/
	virtual bool areProgramResultsUnique(
		std::map<size_t, double> hashesAndResults,
		double tau = 1e-4
	) const;

	/**
	* \brief Get the number of recordings currently held in the Archive.
	*
	* \return the size of the recordings attribute.
	*/
	size_t getNbRecordings() const;

	/**
	* \brief Get the number of different vector of DataHandler associated to
	* recordings.
	*
	* \return the size of the dataHandlers attribute.
	*/
	size_t getNbDataHandlers() const;

	/**
	* \brief Const accessor to the dataHandlers attribute.
	*
	* In order to test the unicity of a Program value, this Program must be
	* executed on all DataHandlers contained in an Archive to assess the
	* uniqueness of the results it produces.
	*
	* \return a const reference to the dataHandlers attribute.
	*/
	const std::map < size_t, std::vector<std::reference_wrapper<DataHandlers::DataHandler>>>& getDataHandlers() const;

	/**
	* \brief Clear all content from the Archive.
	*/
	void clear();

};

#endif