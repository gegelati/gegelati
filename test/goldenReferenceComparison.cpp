/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2021) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2021)
 * Thomas Bourgoin <tbourgoi@insa-rennes.fr> (2021)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

#include <gtest/gtest.h>

#include <algorithm>
#include <iostream>

#include "goldenReferenceComparison.h"

bool compare_files(const std::string& printedFilePath,
                   const std::string& goldenRefPath)
{
    std::ifstream printedFile(printedFilePath);
    if (!printedFile.is_open()) {
        std::cout << "Could not open file \"" << printedFilePath
                  << "\". Check project configuration." << std::endl;
    }

    std::ifstream goldenRef(goldenRefPath);
    if (!goldenRef.is_open()) {
        std::cout << "Could not open file \"" << goldenRefPath
                  << "\". Check project configuration." << std::endl;
    }

    // Check the file content line by line
    // print diffs in the console and count number of printed line.
    uint64_t nbDiffs = 0;
    uint64_t lineNumber = 0;
    while (!printedFile.eof() && !goldenRef.eof()) {
        std::string lineRef;
        std::getline(goldenRef, lineRef);

        std::string lineExport;
        std::getline(printedFile, lineExport);

        // If the lines are different
        // Ref lines starting with a '~' are ignored during the comparison.
        if (lineRef != lineExport && lineRef.at(0) != '~') {
            nbDiffs++;
            std::cout << "Diff at Line " << lineNumber << ":" << std::endl;
            std::cout << "\tref: " << lineRef << std::endl;
            std::cout << "\texp: " << lineExport << std::endl;
        }

        lineNumber++;
    }

    if (!printedFile.eof() || !goldenRef.eof()) {
        nbDiffs++;
        std::cout << "Files have different length." << std::endl;
    }

    return nbDiffs == 0;
}
