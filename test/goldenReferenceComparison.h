#ifndef GOLDEN_REFERENCE_COMPARISON_H
#define GOLDEN_REFERENCE_COMPARISON_H

#include <fstream>

/**
 * Function that return true if printedFilePath and goldenRefPath have the same
 * content.
 *
 * @param printedFilePath const reference to a std::string holding the name of
 * the first file to compare.
 * @param goldenRefPath const reference to a std::string holding the name of the
 * second file to compare.
 * @return return true if the content of both files are identical, false in
 * other cases
 */
bool compare_files(const std::string& printedFilePath,
                   const std::string& goldenRefPath);

#endif // GOLDEN_REFERENCE_COMPARISON_H
