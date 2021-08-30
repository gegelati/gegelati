#ifndef GEGELATI_GOLDEN_REFERENCE_COMPARISON_H
#define GEGELATI_GOLDEN_REFERENCE_COMPARISON_H

#include <fstream>

/**
 * Function that return true if filename1 and filename2 have the same content.
 *
 * @param filename1 const reference to a std::string holding the name of the
 * first file to compare.
 * @param filename2 const reference to a std::string holding the name of the
 * second file to compare.
 * @return return true if the content of both files are identical, false in
 * other cases
 */
bool compare_files(const std::string& filename1, const std::string& filename2);

#endif // GEGELATI_GOLDEN_REFERENCE_COMPARISON_H
