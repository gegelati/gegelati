#include "file/populationDotExporter.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

bool File::PopulationDotExporter::isIndividual(const Data::DataValue& value) {
    return value.getRank() == 0 && value.getElementType() == typeid(std::shared_ptr<const Individual>);
}

std::string File::PopulationDotExporter::nodeName(const Individual* individual) {
    return individual->getRepresentation().getName() + std::to_string(individual->getIndividualID());
}

std::string File::PopulationDotExporter::escape(const std::string& text) {
    std::string result;
    for (char c : text) {
        if (c == '"' || c == '\\') {
            result += '\\';
        }
        result += c;
    }
    return result;
}

void File::PopulationDotExporter::printIndividual(std::ostream& out, const Individual& individual, std::unordered_set<const Individual*>& visited) {
    if (!visited.insert(&individual).second) {
        return;
    }

    std::vector<const Individual*> referenced;
    std::ostringstream genotypeStr;
    std::ostringstream edges;
    // Name-only label used when the graph is rendered in reduced mode.
    const std::string label = escape(nodeName(&individual));

    const GraphBased::Genotype& genotype = individual.getGenotype();
    genotypeStr << "{";
    for (size_t g = 0; g < genotype.getSize(); ++g) {
        const GraphBased::NodeGroup& group = genotype.getNodeGroup(g);
        genotypeStr <<"[";

        for (size_t n = 0; n < group.getSize(); ++n) {
            const GraphBased::GPNode& node = group.getNode(n);
            if(node.getIsIntron()) {
                continue;   
            }

            genotypeStr << "(";
            for (size_t v = 0; v < node.getSize(); ++v) {
                const Data::DataValue& value = node.getValue(v);
                if (v > 0) {
                    genotypeStr << ",";
                }
                if (isIndividual(value)) {
                    const Individual* target = value.getScalar<std::shared_ptr<const Individual>>().get();
                    genotypeStr << escape(nodeName(target));
                    edges << "\t\t" << nodeName(&individual) << " -> " << nodeName(target) << "\n";
                    referenced.push_back(target);
                } else {
                    genotypeStr << escape(value.toValueString());
                }
            }
            genotypeStr <<")";
            if(node.getIsIntron()) {
                genotypeStr << "-intr";
            }
            if(n < group.getSize() - 1) {
                genotypeStr << ",";
            }
        }
        genotypeStr <<"]";
        if(g < genotype.getSize() - 1) {
            genotypeStr << ",";
        }
    }
    genotypeStr << "}";

    out << "\t\t" << nodeName(&individual) << " [fillcolor=\""
        << individual.getRepresentation().getRepresentationColor()
        << "\", shape=box margin=0.03 width=0 height=0"
        << ", label=\"" << label
        << "\", genotype=\"" << genotypeStr.str() << "\"];\n";
        
    for (const Individual* target : referenced) {
        printIndividual(out, *target, visited);
    }
        out << edges.str();
}

void File::PopulationDotExporter::print(std::ostream& out, const std::vector<std::shared_ptr<const Individual>>& individuals) {
    /*
    // File exported with GEGELATI vX.Y.Z
    // On the YYYY-MM-DD HH:MM:SS
    // With the <Printer>
    graph{
    graph[pad = "0.212,0.055" bgcolor = lightgray]
    node[style = filled label = ""]
    */
    std::string offset = "";

    out << "// File exported with GEGELATI v" GEGELATI_VERSION "\n";
    out << "// On the " << Util::getCurrentDate().c_str() << "\n";
    out << "// With the " << DEMANGLE_TYPEID_NAME(typeid(PopulationDotExporter).name()) << "\n";
    out << "digraph{\n";
    out << "\tgraph[pad = \"0.212, 0.055\" bgcolor = lightgray]\n";
    out << "\tnode[shape=circle style = filled label = \"\"]\n";

    std::unordered_set<const Individual*> visited;
    for (const std::shared_ptr<const Individual>& individual : individuals) {
        printIndividual(out, *individual, visited);
    }

    
    // Rank all the individuals
    out << "\t\t{ rank= same ";
    // Team root ids
    for (const std::shared_ptr<const Individual>& individual : individuals) {
        out << nodeName(individual.get()) << " ";
    }
    out << "}\n}\n";
}

std::string File::PopulationDotExporter::toString(const std::vector<std::shared_ptr<const Individual>>& individuals) {
    std::ostringstream oss;
    print(oss, individuals);
    return oss.str();
}

void File::PopulationDotExporter::exportToFile(const std::string& filePath, const std::vector<std::shared_ptr<const Individual>>& individuals) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("PopulationDotExporter: cannot open file " + filePath);
    }
    print(file, individuals);
}
