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
     * \brief Exports a population of Individual to a .dot file, read back by PopulationDotImporter.
     *
     * Each Individual is a single dot node, named by its representation and its ID separated by "_" (e.g. "LGP_12"). Its genotype is written
     * in the genotype attribute of the node: "{[(value,value),(value)],[...]}", one bracket per node group and one parenthesis per GPNode.
     * Each value is a scalar written with its type, "type:value": "st:" for a size_t, "i:" for an int, "d:" for a double, "f:" for a float
     * (doubles and floats with all their digits). A value referencing an individual is written with the representation of the individual as
     * type and its ID as value (e.g. "LGP:12"), and adds an edge towards the referenced individual.
     *
     * The individuals referenced by the population, directly or not (e.g. the members of the teams of a TPG), are written too. All the individuals
     * are written in the order of their IDs. The population is the list of the "{ rank= same ... }" line, in its order: an individual of the
     * population can also be referenced by another one (e.g. a team aggregated by another team).
     * The introns are not written by default: they do not change the behavior of an individual.
     */
    class PopulationDotExporter {
    public:
        /**
         * \brief Writes the dot description of the individuals and of every individual they reference.
         *
         * \param[in] out stream receiving the dot content.
         * \param[in] individuals the population to export.
         * \param[in] withIntrons indicates if the intron nodes are written.
         * \throw std::runtime_error if a value of a node is not a scalar, or if its type is not supported.
         */
        static void print(std::ostream& out, const std::vector<std::shared_ptr<const Individual>>& individuals, bool withIntrons = false);

        /**
         * \brief Returns the dot description as a string.
         *
         * \param[in] individuals the population to export.
         * \param[in] withIntrons indicates if the intron nodes are written.
         */
        static std::string toString(const std::vector<std::shared_ptr<const Individual>>& individuals, bool withIntrons = false);

        /**
         * \brief Writes the dot description into a file.
         *
         * \param[in] filePath path of the file to write.
         * \param[in] individuals the population to export.
         * \param[in] withIntrons indicates if the intron nodes are written.
         */
        static void exportToFile(const std::string& filePath, const std::vector<std::shared_ptr<const Individual>>& individuals, bool withIntrons = false);

    private:
        /// Text of a value of a node, "type:value". Throws if the value is not a scalar of a supported type.
        static std::string valueText(const Data::DataValue& value);

        /// Text of a node, "(value,value)".
        static std::string nodeText(const GraphBased::GPNode& node);

        /// Text of a node group, "[(...),(...)]", without the introns unless withIntrons.
        static std::string nodeGroupText(const GraphBased::NodeGroup& group, bool withIntrons);

        /// Text of a genotype, "{[...],[...]}", without the introns unless withIntrons.
        static std::string genotypeText(const GraphBased::Genotype& genotype, bool withIntrons);

        /**
         * \brief Write an individual: its node, with its genotype, and the edges towards the individuals it references.
         *
         * \param[in,out] out the stream receiving the graph.
         * \param[in] individual the individual written.
         * \param[in] withIntrons indicates if the intron nodes are written.
         */
        static void printIndividual(std::ostream& out, const Individual& individual, bool withIntrons);
    };
}

#endif // FILE_POPULATION_DOT_EXPORTER_H
