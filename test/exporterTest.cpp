/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
 * Thomas Bourgoin <tbourgoi@insa-rennes.fr> (2021)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

#include <gtest/gtest.h>

#include <cinttypes>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "evoGraph/action.h"
#include "evoGraph/edge.h"
#include "evoGraph/graph.h"
#include "evoGraph/team.h"
#include "evoGraph/vertex.h"
#include "file/graphDotExporter.h"
#include "representation/population.h"
#include "representation/representation.h"
#include "util/counterReset.h"

namespace Representation{

class MockIndividual : public Individual
{
  public:
    explicit MockIndividual(uint64_t representationID)
        : Individual(representationID) {}

    void setVertex(const EvoGraph::Vertex& vertex)
    {
        this->vertex = std::cref(vertex);
    }

    bool hasVertex() const { return this->vertex.has_value(); }

    const EvoGraph::Vertex& getVertex() const { return this->vertex.value().get(); }

  private:
    std::optional<std::reference_wrapper<const EvoGraph::Vertex>> vertex;
};

class MockPopulation : public Population
{
  public:
    MockPopulation(const Output::OutputHandler& outputs, uint64_t representationID)
        : Population(outputs, representationID) {}

    const Individual& createIndividual(EvoGraph::Graph&) override
    {
        auto newIndividual = std::make_unique<MockIndividual>(this->getRepresentationID());
        const auto& ref = *newIndividual;
        this->individuals.insert(std::move(newIndividual));
        return ref;
    }

    const Individual& copyIndividual(const Individual& individual,
                                                     EvoGraph::Graph&) override
    {
        auto newIndividual = std::make_unique<MockIndividual>(individual.getRepresentationID());
        const auto& ref = *newIndividual;
        this->individuals.insert(std::move(newIndividual));
        return ref;
    }

    void emptyIndividual(const Individual&, EvoGraph::Graph&) override {}

    std::unique_ptr<ExecutionEngine> createExecutionEngine(
        std::vector<std::reference_wrapper<const Data::DataHandler>>,
        bool) const override
    {
        return nullptr;
    }
};

class MockRepresentation : public Representation
{
  public:
    MockRepresentation(std::string name = "MockRepresentation",
                       std::string color = "#123456")
        : Representation(std::make_unique<RepresentationParameters>(),
                                          std::move(name), std::move(color))
    {
    }

    void attachGraph(std::shared_ptr<EvoGraph::Graph> graph) { this->graph = std::move(graph); }

    void initializePopulation()
    {
        this->outputs = std::make_unique<Output::OutputHandler>(Output::OutputHandler(1));
        this->population = std::make_unique<MockPopulation>(*this->outputs, this->representationID);
        this->init = true;
    }

    const Individual& addTestIndividual(const EvoGraph::Vertex& vertex)
    {
        MockPopulation& pop = dynamic_cast<MockPopulation&>(*this->population);
        const Individual& created = pop.createIndividual(*this->graph);
        auto& mockIndividual = dynamic_cast<MockIndividual&>(const_cast<Individual&>(created));
        mockIndividual.setVertex(vertex);
        return created;
    }

    void initPopulation() override
    {
        this->outputs = std::make_unique<Output::OutputHandler>(Output::OutputHandler(1));
        this->population = std::make_unique<MockPopulation>(*this->outputs, this->representationID);
    }

    void initMutator() override {}

    std::unique_ptr<Representation> copy() const override
    {
        auto copy = std::make_unique<MockRepresentation>(this->representationName,
                                                         this->representationColor);
        copy->setRepresentationID(this->representationID);
        return copy;
    }

    void clearUnusedIndividualParts() override {}

    std::shared_ptr<PolicyStats> createPolicyStats() const override
    {
        return nullptr;
    }

    void printCodeGenIndividuals(std::ofstream&, std::ofstream&,
                                 const std::set<std::reference_wrapper<const Individual>>&, std::map<uint64_t, std::set<std::reference_wrapper<const Individual>>>&) const override
    {
    }

    void printIndividual(const Individual& individual, FILE* pFile,
                         std::string offset,
                         std::set<uint64_t>& printedIndividualID,
                         std::vector<std::reference_wrapper<const EvoGraph::Element>>& elementsToPrint) const override
    {
        if (printedIndividualID.find(individual.getIndividualID()) == printedIndividualID.end() &&
            this->containsIndividual(individual)) {
            printedIndividualID.insert(individual.getIndividualID());

            const auto& mockIndividual = dynamic_cast<const MockIndividual&>(individual);
            if (mockIndividual.hasVertex()) {
                elementsToPrint.push_back(mockIndividual.getVertex());
            }

            fprintf(pFile,
                    "%sP%" PRIu64 " [fillcolor=\"%s\" shape=diamond margin=0.03 width=0 height=0 label=\"%s.%" PRIu64 "\"]\n",
                    offset.c_str(), individual.getIndividualID(), this->representationColor.c_str(),
                    this->representationName.c_str(), this->representationID);
        }
    }

    const Individual& readIndividual(std::smatch&) override
    {
        throw std::runtime_error("MockRepresentation::readIndividual is not implemented");
    }
};

class ExporterTest : public ::testing::Test
{
  protected:
    void SetUp() override { CounterReset::counterReset(); }

    static std::string readFile(const std::filesystem::path& path)
    {
        std::ifstream input(path);
        return std::string(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    }
};

TEST_F(ExporterTest, exportRepresentationWritesADotFile)
{
    auto graph = std::make_shared<EvoGraph::Graph>();
    auto representation = std::make_unique<MockRepresentation>("Root", "#112233");
    representation->attachGraph(graph);
    representation->initializePopulation();

    auto subRepresentation = std::make_unique<MockRepresentation>("Sub", "#445566");
    subRepresentation->attachGraph(graph);
    subRepresentation->initializePopulation();
    representation->addSubRepresentation(*subRepresentation);

    const auto& team = graph->addNewTeam();
    const auto& action = graph->addNewAction(0);
    const auto& program = representation->addTestIndividual(team);
    graph->addNewEdge(team, action, program);

    const auto outputPath = std::filesystem::temp_directory_path() / "gegelati_export_representation.dot";
    std::filesystem::remove(outputPath);

    File::GraphDotExporter exporter;
    ASSERT_NO_THROW(exporter.exportRepresentation(outputPath.string().c_str(), *representation));

    ASSERT_TRUE(std::filesystem::exists(outputPath));
    const std::string content = readFile(outputPath);
    EXPECT_NE(content.find("digraph"), std::string::npos);
    EXPECT_NE(content.find("ALGO"), std::string::npos);
    EXPECT_NE(content.find("P"), std::string::npos);
    EXPECT_NE(content.find("T"), std::string::npos);
    EXPECT_NE(content.find("A"), std::string::npos);

    std::filesystem::remove(outputPath);
}

TEST_F(ExporterTest, exportIndividualWritesOnlyTheRequestedSubgraph)
{
    auto graph = std::make_shared<EvoGraph::Graph>();
    auto representation = std::make_unique<MockRepresentation>("Root", "#112233");
    representation->attachGraph(graph);
    representation->initializePopulation();

    const auto& team = graph->addNewTeam();
    const auto& action = graph->addNewAction(0);
    const auto& firstProgram = representation->addTestIndividual(team);
    graph->addNewEdge(team, action, firstProgram);

    const auto outputPath = std::filesystem::temp_directory_path() / "gegelati_export_individual.dot";
    std::filesystem::remove(outputPath);

    File::GraphDotExporter exporter;
    ASSERT_NO_THROW(exporter.exportIndividual(outputPath.string().c_str(), firstProgram, *representation));

    ASSERT_TRUE(std::filesystem::exists(outputPath));
    const std::string content = readFile(outputPath);
    EXPECT_NE(content.find("P"), std::string::npos);
    EXPECT_NE(content.find("T"), std::string::npos);
    EXPECT_NE(content.find("A"), std::string::npos);
    EXPECT_NE(content.find("Root."), std::string::npos);

    std::filesystem::remove(outputPath);
}

TEST_F(ExporterTest, exportRepresentationThrowsForInvalidPath)
{
    auto graph = std::make_shared<EvoGraph::Graph>();
    auto representation = std::make_unique<MockRepresentation>("Root", "#112233");
    representation->attachGraph(graph);
    representation->initializePopulation();

    const auto& team = graph->addNewTeam();
    const auto& action = graph->addNewAction(0);
    const auto& program = representation->addTestIndividual(team);
    graph->addNewEdge(team, action, program);

    File::GraphDotExporter exporter;
    EXPECT_THROW(exporter.exportRepresentation("XXX://INVALID_PATH", *representation), std::runtime_error);
}

TEST_F(ExporterTest, exportIndividualThrowsForUnknownIndividual)
{
    auto graph = std::make_shared<EvoGraph::Graph>();
    auto representation = std::make_unique<MockRepresentation>("Root", "#112233");
    representation->attachGraph(graph);
    representation->initializePopulation();

    const auto& team = graph->addNewTeam();
    const auto& action = graph->addNewAction(0);
    const auto& program = representation->addTestIndividual(team);
    graph->addNewEdge(team, action, program);

    auto unknownIndividual = std::make_unique<MockIndividual>(representation->getRepresentationID());

    File::GraphDotExporter exporter;
    EXPECT_THROW(exporter.exportIndividual("unused.dot", *unknownIndividual, *representation), std::runtime_error);
}

} // namespace