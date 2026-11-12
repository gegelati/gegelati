#ifndef FILE_POPULATION_DOT_EXPORTER_H
#define FILE_POPULATION_DOT_EXPORTER_H

#include <memory>
#include <ostream>
#include <string>
#include <unordered_set>
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

    private:
        /// Indicates if a value of a node is a reference to an individual (a destination or a member).
        static bool isIndividual(const Data::DataValue& value);

        /// Name of the node of an individual in the graph: the name of its representation followed by its ID.
        static std::string nodeName(const Individual* individual);

        /// Text with the quotes and the backslashes escaped, to write it in a string of the dot file.
        static std::string escape(const std::string& text);

        /**
         * \brief Write an individual, then the individuals it references, each one once.
         *
         * \param[in,out] out the stream receiving the graph.
         * \param[in] individual the individual written.
         * \param[in,out] visited the individuals already written.
         */
        static void printIndividual(std::ostream& out, const Individual& individual, std::unordered_set<const Individual*>& visited);
    };
}

#endif // FILE_POPULATION_DOT_EXPORTER_H
