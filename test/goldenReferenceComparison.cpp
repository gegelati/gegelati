#include <gtest/gtest.h>

#include <algorithm>
#include <iostream>

#include "goldenReferenceComparison.h"

bool compare_files(const std::string& printedFilePath, const std::string& goldenRefPath)
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

        if (lineRef != lineExport) {
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