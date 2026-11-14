#include "file/populationDotImporter.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

std::vector<std::shared_ptr<Individual>> File::PopulationDotImporter::readPopulation(const std::string& line, const std::map<uint64_t, std::shared_ptr<Individual>>& individualsById)
{
    // The line lists the node names of the population, "{ rank= same LGP_12 LGP_15 }".
    const std::string key = "{ rank= same ";
    std::istringstream nodeNames(line.substr(line.find(key) + key.size()));
    std::vector<std::shared_ptr<Individual>> population;
    std::string nodeName;
    while(nodeNames >> nodeName && nodeName != "}") {
        const size_t separator = nodeName.rfind('_');
        const std::map<uint64_t, std::shared_ptr<Individual>>::const_iterator individual = individualsById.find(std::stoull(nodeName.substr(separator + 1)));
        if(individual == individualsById.end() || individual->second->getRepresentation().getName() != nodeName.substr(0, separator)) {
            throw std::runtime_error("PopulationDotImporter: the individual " + nodeName + " of the population is not in the file");
        }
        population.push_back(individual->second);
    }
    return population;
}

template <typename T> T File::PopulationDotImporter::readAs(const std::string& text)
{
    // The whole text must be read as a T, nothing must remain (unlike std::stoi, std::stod... which ignore what follows the number).
    std::istringstream stream(text);
    T value;
    stream >> value;
    if(stream.fail() || !stream.eof()) {
        throw std::runtime_error("PopulationDotImporter: cannot read the value \"" + text + "\"");
    }
    return value;
}

Data::DataValue File::PopulationDotImporter::readValue(const std::string& text, const std::map<uint64_t, std::shared_ptr<Individual>>& individualsById)
{
    // A value is written "type:value".
    const size_t separator = text.find(':');
    if(separator == std::string::npos) {
        throw std::runtime_error("PopulationDotImporter: the value \"" + text + "\" has no type");
    }
    const std::string type = text.substr(0, separator);
    const std::string value = text.substr(separator + 1);

    // A scalar of a supported type.
    if(type == "st") {
        return Data::DataValue::scalar<size_t>(readAs<size_t>(value));
    }
    if(type == "i") {
        return Data::DataValue::scalar<int>(readAs<int>(value));
    }
    if(type == "d") {
        return Data::DataValue::scalar<double>(readAs<double>(value));
    }
    if(type == "f") {
        return Data::DataValue::scalar<float>(readAs<float>(value));
    }

    // Otherwise a reference to an individual of the file: the type is the name of its representation and the value its ID ("LGP:12").
    const std::map<uint64_t, std::shared_ptr<Individual>>::const_iterator referenced = individualsById.find(std::stoull(value));
    if(referenced == individualsById.end() || referenced->second->getRepresentation().getName() != type) {
        throw std::runtime_error("PopulationDotImporter: the value \"" + text + "\" has an unknown type (st, i, d, f) or does not reference an individual of the file (representation:ID)");
    }
    return Data::DataValue::scalar<std::shared_ptr<const Individual>>(referenced->second);
}

std::unique_ptr<GraphBased::GPNode> File::PopulationDotImporter::readNode(const std::string& text, const std::map<uint64_t, std::shared_ptr<Individual>>& individualsById)
{
    // The values of the node are separated by commas, "st:1,st:4,LGP:12".
    std::vector<Data::DataValue> values;
    size_t start = 0;
    while(start < text.size()) {
        size_t end = text.find(',', start);
        if(end == std::string::npos) {
            end = text.size();
        }
        values.push_back(readValue(text.substr(start, end - start), individualsById));
        start = end + 1;
    }
    return std::make_unique<GraphBased::GPNode>(values);
}

std::unique_ptr<GraphBased::NodeGroup> File::PopulationDotImporter::readNodeGroup(const std::string& text, const std::map<uint64_t, std::shared_ptr<Individual>>& individualsById)
{
    // Each node of the group is between parentheses, "(...),(...)".
    std::unique_ptr<GraphBased::NodeGroup> group = std::make_unique<GraphBased::NodeGroup>();
    size_t start = text.find('(');
    while(start != std::string::npos) {
        const size_t end = text.find(')', start);
        group->addNode(readNode(text.substr(start + 1, end - start - 1), individualsById));
        start = text.find('(', end);
    }
    return group;
}

std::unique_ptr<GraphBased::Genotype> File::PopulationDotImporter::readGenotype(const std::string& text, const std::map<uint64_t, std::shared_ptr<Individual>>& individualsById)
{
    // Each node group of the genotype is between brackets, "{[...],[...]}".
    std::unique_ptr<GraphBased::Genotype> genotype = std::make_unique<GraphBased::Genotype>();
    size_t start = text.find('[');
    while(start != std::string::npos) {
        const size_t end = text.find(']', start);
        genotype->addNodeGroup(readNodeGroup(text.substr(start + 1, end - start - 1), individualsById));
        start = text.find('[', end);
    }
    return genotype;
}

bool File::PopulationDotImporter::updateValidities(const std::vector<std::shared_ptr<Individual>>& individuals)
{
    // Only the individuals not valid yet can become valid, now that the individuals they reference may be.
    bool changed = false;
    for(const std::shared_ptr<Individual>& individual: individuals) {
        if(individual->isValid()) {
            continue;
        }
        individual->updateValidity();
        changed = changed || individual->isValid();
    }
    return changed;
}

std::vector<std::shared_ptr<Individual>> File::PopulationDotImporter::read(std::istream& in, const std::vector<std::shared_ptr<const Representations::Representation>>& representations)
{
    std::map<std::string, std::shared_ptr<const Representations::Representation>> representationsByName;
    for(const std::shared_ptr<const Representations::Representation>& representation: representations) {
        representationsByName.emplace(representation->getName(), representation);
    }

    // Step 1: create the individual of each node ("LGP_12 [..., genotype="..."];") with its representation. The file is in the order of the IDs,
    // so the new IDs keep this order. The genotypes are read once all the individuals exist, since they reference each other.
    std::vector<std::shared_ptr<Individual>> individuals;
    std::vector<std::string> genotypeTexts;
    std::map<uint64_t, std::shared_ptr<Individual>> individualsById;
    std::string populationLine;
    std::string line;
    while(std::getline(in, line)) {
        const size_t genotypeStart = line.find("genotype=\"");
        if(genotypeStart != std::string::npos) {
            // The node name is the first word of the line, the representation name followed by "_" and the ID in the file.
            std::string nodeName;
            std::istringstream(line) >> nodeName;
            const size_t separator = nodeName.rfind('_');
            const std::string representationName = nodeName.substr(0, separator);
            const uint64_t id = std::stoull(nodeName.substr(separator + 1));
            if(representationsByName.count(representationName) == 0) {
                throw std::runtime_error("PopulationDotImporter: no representation named " + representationName);
            }

            // The individual is created now, its genotype (the text between the quotes of the genotype attribute) is kept for step 2.
            const std::shared_ptr<Individual> individual = std::make_shared<Individual>(representationsByName.at(representationName));
            if(!individualsById.emplace(id, individual).second) {
                throw std::runtime_error("PopulationDotImporter: two individuals have the ID " + std::to_string(id));
            }
            const size_t textStart = genotypeStart + std::string("genotype=\"").size();
            individuals.push_back(individual);
            genotypeTexts.push_back(line.substr(textStart, line.find('"', textStart) - textStart));
        } else if(line.find("{ rank= same ") != std::string::npos) {
            populationLine = line;
        }
    }

    // Step 2: add the genotypes, every referenced individual existing.
    for(size_t idx = 0; idx < individuals.size(); idx++) {
        individuals.at(idx)->setGenotype(readGenotype(genotypeTexts.at(idx), individualsById));
    }

    // Step 3: the validity of an individual depends on the validity of the ones it references, which may have received their genotype after it
    // (e.g. a team referencing a member created by its mutation): the validities are updated until none changes.
    bool changed = true;
    while(changed) {
        changed = updateValidities(individuals);
    }

    // The individuals still not valid are not valid for their representation (e.g. exported without their introns, see the class): it is only signaled.
    size_t nbInvalid = 0;
    for(const std::shared_ptr<Individual>& individual: individuals) {
        nbInvalid += individual->isValid() ? 0 : 1;
    }
    if(nbInvalid > 0) {
        std::cerr << "Warning: PopulationDotImporter: " << nbInvalid << " imported individual(s) are not valid for their representation." << std::endl;
    }

    // Step 4: the population ("{ rank= same LGP_12 LGP_15 }").
    return readPopulation(populationLine, individualsById);
}

std::vector<std::shared_ptr<Individual>> File::PopulationDotImporter::importFromFile(const std::string& filePath, const std::vector<std::shared_ptr<const Representations::Representation>>& representations)
{
    std::ifstream file(filePath);
    if(!file.is_open()) {
        throw std::runtime_error("PopulationDotImporter: cannot open file " + filePath);
    }
    return read(file, representations);
}

void File::PopulationDotImporter::importToData(const std::string& filePath, const std::vector<std::shared_ptr<const Representations::Representation>>& representations, Evolution::EvolutionData& data, const std::string& individuals)
{
    // The population replaces the list of individuals of the data, if any.
    data.set(individuals, std::make_shared<std::vector<std::shared_ptr<Individual>>>(importFromFile(filePath, representations)));
}
