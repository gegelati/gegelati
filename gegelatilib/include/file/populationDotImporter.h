#ifndef FILE_POPULATION_DOT_IMPORTER_H
#define FILE_POPULATION_DOT_IMPORTER_H

#include <istream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "data/dataValue.h"
#include "evolution/evolutionData.h"
#include "individual.h"
#include "representations/representation.h"

namespace File {

    /**
     * \brief Imports a population of Individual from a .dot file written by PopulationDotExporter.
     *
     * Every individual of the file is created again with its representation, found by its name (the node "LGP_12" is an individual of the
     * representation named "LGP"). All the individuals are first created in the order of the file, the order of their IDs, so their new IDs keep
     * this order. Their genotypes are then added, every referenced individual existing, and their validities are updated. The introns are
     * detected again by the representation.
     *
     * Only the population (the "{ rank= same ... }" line) is returned: the other individuals are only referenced by it (e.g. the members of a TPG).
     * To import individuals exactly, export them with their introns: without them, a genotype may have fewer nodes than its representation
     * requires, and the individual (and the ones referencing it) is then not valid: a warning gives the number of individuals not valid.
     */
    class PopulationDotImporter {
      public:
        /**
         * \brief Read the population of a dot description.
         *
         * \param[in] in stream giving the dot content.
         * \param[in] representations the representations of the individuals of the file, found by their names.
         * \return the population, in its order.
         * \throw std::runtime_error if an individual has no representation, if a value cannot be read, or if an individual of the population is unknown.
         */
        static std::vector<std::shared_ptr<Individual>> read(std::istream& in, const std::vector<std::shared_ptr<const Representations::Representation>>& representations);

        /**
         * \brief Read the population of a dot file.
         *
         * \param[in] filePath path of the file to read.
         * \param[in] representations the representations of the individuals of the file, found by their names.
         * \return the population, in its order.
         * \throw std::runtime_error if the file cannot be opened, or if it cannot be read (see read).
         */
        static std::vector<std::shared_ptr<Individual>> importFromFile(const std::string& filePath, const std::vector<std::shared_ptr<const Representations::Representation>>& representations);

        /**
         * \brief Read the population of a dot file and store it in the data of an evolution, as a list of individuals.
         *
         * \param[in] filePath path of the file to read.
         * \param[in] representations the representations of the individuals of the file, found by their names.
         * \param[in,out] data the data receiving the population.
         * \param[in] individuals name of the list of individuals in the data (e.g. the population of an evolution).
         * \throw std::runtime_error if the file cannot be read (see importFromFile).
         */
        static void importToData(const std::string& filePath, const std::vector<std::shared_ptr<const Representations::Representation>>& representations, Evolution::EvolutionData& data, const std::string& individuals);

      private:
        /**
         * \brief Read the population, the individuals of the "{ rank= same LGP_12 LGP_15 }" line.
         *
         * \param[in] line the line of the population.
         * \param[in] individualsById the individuals of the file, by their ID in the file.
         * \throw std::runtime_error if an individual of the population is not in the file.
         */
        static std::vector<std::shared_ptr<Individual>> readPopulation(const std::string& line, const std::map<uint64_t, std::shared_ptr<Individual>>& individualsById);

        /**
         * \brief Update the validity of individuals (see Individual::updateValidity).
         *
         * \param[in] individuals the individuals to update.
         * \return true if the validity of an individual changed.
         */
        static bool updateValidities(const std::vector<std::shared_ptr<Individual>>& individuals);

        /**
         * \brief Read a whole text as a value of a type.
         *
         * Used instead of std::stoi, std::stod... to detect a wrong value: these functions ignore what follows the number ("3abc" is read as 3)
         * and throw an error that does not give the value. Here, the whole text must be the value, and the error gives the text.
         *
         * \tparam T the type of the value.
         * \param[in] text the text of the value.
         * \throw std::runtime_error if the text is not a value of the type.
         */
        template <typename T> static T readAs(const std::string& text);

        /**
         * \brief Read a value written "type:value": a scalar of a supported type, or a reference to an individual ("LGP:12").
         *
         * \param[in] text the text of the value.
         * \param[in] individualsById the individuals of the file, by their ID in the file.
         * \throw std::runtime_error if the value cannot be read, or if the referenced individual is unknown.
         */
        static Data::DataValue readValue(const std::string& text, const std::map<uint64_t, std::shared_ptr<Individual>>& individualsById);

        /**
         * \brief Read a node, the text between its parentheses: values separated by commas.
         *
         * \param[in] text the text of the node.
         * \param[in] individualsById the individuals of the file, by their ID in the file.
         */
        static std::unique_ptr<GraphBased::GPNode> readNode(const std::string& text, const std::map<uint64_t, std::shared_ptr<Individual>>& individualsById);

        /**
         * \brief Read a node group, the text between its brackets: nodes between parentheses.
         *
         * \param[in] text the text of the node group.
         * \param[in] individualsById the individuals of the file, by their ID in the file.
         */
        static std::unique_ptr<GraphBased::NodeGroup> readNodeGroup(const std::string& text, const std::map<uint64_t, std::shared_ptr<Individual>>& individualsById);

        /**
         * \brief Read a genotype: node groups between brackets.
         *
         * \param[in] text the text of the genotype.
         * \param[in] individualsById the individuals of the file, by their ID in the file.
         */
        static std::unique_ptr<GraphBased::Genotype> readGenotype(const std::string& text, const std::map<uint64_t, std::shared_ptr<Individual>>& individualsById);
    };

} // namespace File

#endif // FILE_POPULATION_DOT_IMPORTER_H
