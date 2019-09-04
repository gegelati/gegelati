#ifndef ARCHIVE_H
#define ARCHIVE_H

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
	* \brief Storage for DataHandler copies used in recordings.
	*
	* This map associates a hash values with the corresonding copy of the set
	* of DataHandler that produced this value. The hash value is used in
	* recordings to associate each recording to the right copy of the
	* DataHandler.
	*/
	std::map<size_t, std::vector<std::reference_wrapper<DataHandlers::DataHandler>>> dataHandlers;

	/// Recordings of the Archive
	std::deque<ArchiveRecording> recordings;

public:
	/**
	* \brief Main constructor for Archive.
	*
	* \param[in] size maximum number of recordings kept in the Archive.
	*/
	Archive(size_t size = 50) : maxSize{ size }, recordings() {};

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
	* This method assumes that the cached hash value of all DataHandler is up
	* to date and simply accesses it using the DataHandler::getHash() method.
	*
	* \return the hash resulting from the combination.
	*/
	static size_t getCombinedHash(const std::vector<std::reference_wrapper<DataHandlers::DataHandler>>& dHandler);

	/**
	* \brief Add a new recording to the Archive.
	*
	* If the maximum number of recordings held in the archive is reached, the
	* oldest recording will be removed.
	* If this is the first time this set of DataHandler is stored in the
	* Archive according to its DataHandler::getHash() method, a copy of the
	* dataHandler will be created.
	* If an identical recording is already in the Archive (same hash, same
	* result, possibly different Program), the recording is not added.
	*
	* \param[in] program the Program associated to this recording.
	* \param[in] dHandler the set of dataHandler the Program worked on to
	*                     generate the associated result.
	* \param[in] result double value produced by the Program.
	*/
	void addRecording(const Program::Program* const program,
		const std::vector<std::reference_wrapper<DataHandlers::DataHandler>>& dHandler,
		double result);

	/**
	* \brief Check whether the given set of DataHandler is already in the archive.
	*
	* \param[in] dHandler the DataHandlers whose hash presence will be tested.
	* \return true if the hash of the given dataHandlers is already in the
	*         Archive, false otherwise.
	*/
	bool hasDataHandlers(const std::vector<std::reference_wrapper<DataHandlers::DataHandler>>& dHandler) const;

	/**
	* \brief Check if a recording exists for the given DataHandler and result.
	*
	* Check if there exists a recording in the archive that produces the same
	* result for the same DataHandler. The result is considered to be equal
	* if its absolute difference with one stored in the archive is below the
	* given tau value.
	*
	* \param[in] dHandler the DataHandlers associated to the checked result.
	* \param[in] result the checked result
	* \param[in] tau the double value for testing the approximate equality.
				(default is 10e-4)
	* \return whether the check was successful or not.
	*/
	bool isUnique(
		const std::vector<std::reference_wrapper<DataHandlers::DataHandler>>& dHandler,
		double result,
		double tau = 10e-4) const;

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

};

#endif