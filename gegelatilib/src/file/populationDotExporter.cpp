#include "file/populationDotExporter.h"

#include <fstream>
#include <iomanip>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>

#include "util/individualUtils.h"

std::string File::PopulationDotExporter::valueText(const Data::DataValue& value) {
    if (value.getRank() != 0) {
        throw std::runtime_error("PopulationDotExporter: the values of the nodes must be scalars.");
    }
    std::ostringstream text;
    if (value.getElementType() == typeid(std::shared_ptr<const Individual>)) {
        const Individual& referenced = *value.getScalar<std::shared_ptr<const Individual>>();
        text << referenced.getRepresentation().getName() << ":" << referenced.getIndividualID();
    } else if (value.getElementType() == typeid(size_t)) {
        text << "st:" << value.getScalar<size_t>();
    } else if (value.getElementType() == typeid(int)) {
        text << "i:" << value.getScalar<int>();
    } else if (value.getElementType() == typeid(double)) {
        text << "d:" << std::setprecision(std::numeric_limits<double>::max_digits10) << value.getScalar<double>();
    } else if (value.getElementType() == typeid(float)) {
        text << "f:" << std::setprecision(std::numeric_limits<float>::max_digits10) << value.getScalar<float>();
    } else {
        throw std::runtime_error("PopulationDotExporter: the type of a value of a node is not supported (size_t, int, double, float or an individual).");
    }
    return text.str();
}

std::string File::PopulationDotExporter::nodeText(const GraphBased::GPNode& node) {
    std::string text = "(";
    for (size_t idxValue = 0; idxValue < node.getSize(); idxValue++) {
        text += (idxValue > 0 ? "," : "") + valueText(node.getValue(idxValue));
    }
    return text + ")";
}

std::string File::PopulationDotExporter::nodeGroupText(const GraphBased::NodeGroup& group, bool withIntrons) {
    std::string text = "[";
    bool firstNode = true;
    for (size_t idxNode = 0; idxNode < group.getSize(); idxNode++) {
        const GraphBased::GPNode& node = group.getNode(idxNode);
        if (node.getIsIntron() && !withIntrons) {
            continue;
        }
        text += (firstNode ? "" : ",") + nodeText(node);
        firstNode = false;
    }
    return text + "]";
}

std::string File::PopulationDotExporter::genotypeText(const GraphBased::Genotype& genotype, bool withIntrons) {
    std::string text = "{";
    for (size_t idxGroup = 0; idxGroup < genotype.getSize(); idxGroup++) {
        text += (idxGroup > 0 ? "," : "") + nodeGroupText(genotype.getNodeGroup(idxGroup), withIntrons);
    }
    return text + "}";
}

void File::PopulationDotExporter::printIndividual(std::ostream& out, const Individual& individual, bool withIntrons) {
    // The node of an individual is named by its representation and its ID, e.g. "LGP_12".
    const std::string name = individual.getRepresentation().getName() + "_" + std::to_string(individual.getIndividualID());
    out << "\t\t" << name << " [fillcolor=\"" << individual.getRepresentation().getRepresentationColor() << "\", shape=box margin=0.03 width=0 height=0"
        << ", label=\"" << name << "\", genotype=\"" << genotypeText(individual.getGenotype(), withIntrons) << "\"];\n";

    const std::vector<std::shared_ptr<const Individual>> members = Util::getMemberIndividuals(individual);
    for (const std::shared_ptr<const Individual>& member : members) {
        out << "\t\t" << name << " -> " << member->getRepresentation().getName() << "_" << member->getIndividualID() << "\n";
    }
}

void File::PopulationDotExporter::print(std::ostream& out, const std::vector<std::shared_ptr<const Individual>>& individuals, bool withIntrons) {
    
    out << "// File exported with GEGELATI v" GEGELATI_VERSION "\n";
    out << "// On the " << Util::getCurrentDate().c_str() << "\n";
    out << "// With the " << DEMANGLE_TYPEID_NAME(typeid(PopulationDotExporter).name()) << "\n";
    out << "digraph{\n";
    out << "\tgraph[pad = \"0.212, 0.055\" bgcolor = lightgray]\n";
    out << "\tnode[shape=circle style = filled label = \"\"]\n";

    // All the individuals, the population and their members, are written in the order of their IDs.
    std::vector<std::shared_ptr<const Individual>> allIndividuals = Util::getAllMemberIndividuals(individuals);
    allIndividuals.insert(allIndividuals.end(), individuals.begin(), individuals.end());
    const std::map<uint64_t, std::shared_ptr<const Individual>> orderedIndividuals = Util::makeOrderedIndex(allIndividuals);
    for (const std::pair<const uint64_t, std::shared_ptr<const Individual>>& individual : orderedIndividuals) {
        printIndividual(out, *individual.second, withIntrons);
    }

    // The population, in its order.
    out << "\t\t{ rank= same ";
    for (const std::shared_ptr<const Individual>& individual : individuals) {
        out << individual->getRepresentation().getName() << "_" << individual->getIndividualID() << " ";
    }
    out << "}\n}\n";
}

std::string File::PopulationDotExporter::toString(const std::vector<std::shared_ptr<const Individual>>& individuals, bool withIntrons) {
    std::ostringstream oss;
    print(oss, individuals, withIntrons);
    return oss.str();
}

void File::PopulationDotExporter::exportToFile(const std::string& filePath, const std::vector<std::shared_ptr<const Individual>>& individuals, bool withIntrons) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("PopulationDotExporter: cannot open file " + filePath);
    }
    print(file, individuals, withIntrons);
}
