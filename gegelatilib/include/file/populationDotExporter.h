#ifndef FILE_POPULATION_DOT_EXPORTER_H
#define FILE_POPULATION_DOT_EXPORTER_H

#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "individual.h"
#include "util/timestamp.h"

namespace File {

    /**
     * \brief Exports a population of Individual to a .dot file.
     *
     * Each Individual is a single dot node. Its genotype is printed inside the
     * node label: one line per GPNode, each value being printed with
     * Data::DataValue::toValueString.
     *
     * A value holding a std::shared_ptr<const Individual> is printed as
     * "{ID, RepresentationName}" and adds an edge towards the referenced
     * Individual. Only these individual aggregations appear as edges; the
     * genotype of each Individual stays "hidden" inside its node.
     */
    class PopulationDotExporter {

    public:

        /**
         * \brief Writes the dot description of the individuals and of every
         * individual reachable from them.
         *
         * \param[in] out stream receiving the dot content.
         * \param[in] individuals roots of the population to export.
         */
        static void print(std::ostream& out, const std::vector<std::shared_ptr<const Individual>>& individuals);

        /**
         * \brief Returns the dot description as a string.
         *
         * \param[in] individuals roots of the population to export.
         */
        static std::string toString(const std::vector<std::shared_ptr<const Individual>>& individuals);

        /**
         * \brief Writes the dot description into a file.
         *
         * \param[in] filePath path of the file to write.
         * \param[in] individuals roots of the population to export.
         */
        static void exportToFile(const std::string& filePath, const std::vector<std::shared_ptr<const Individual>>& individuals);
    };
}

#endif // FILE_POPULATION_DOT_EXPORTER_H
