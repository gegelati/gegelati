#include <filesystem>
#include <gtest/gtest.h>
#include <sstream>

#include "evaluation/evaluate.h"
#include "evaluation/reinforcementProblem.h"
#include "evolution/evolutionData.h"
#include "evolution/originTPGEvolutionConfig.h"
#include "fitnessAssignment/defaultAssigner.h"
#include "metrics/rewardsMetric.h"
#include "file/populationDotExporter.h"
#include "file/populationDotImporter.h"
#include "instructions/lambdaInstruction.h"
#include "instructions/set.h"
#include "learn/stickGameWithOpponent.h"
#include "learn/tpgGenerator.h"
#include "mutation/pointMutator.h"
#include "representations/LGP.h"
#include "representations/TPG.h"
#include "util/counterReset.h"

class PopulationDotImporterTest : public ::testing::Test
{
  protected:
    Instructions::LambdaInstruction<double, double, double> add{[](double a, double b) -> double { return a + b; }};
    Instructions::LambdaInstruction<double, double, double> minus{[](double a, double b) -> double { return a - b; }};
    Instructions::Set set;
    std::shared_ptr<Representations::LGP> lgpRep;
    std::shared_ptr<Representations::TPG> tpgRep;
    std::vector<std::shared_ptr<const Representations::Representation>> representations;

    // Roots of the exported population: vertices pointing to members, some of them also to other vertices.
    std::vector<std::shared_ptr<const Individual>> roots;

    virtual void SetUp()
    {
        CounterReset::counterReset();
        set.add(add);
        set.add(minus);
        std::shared_ptr<Evaluation::ReinforcementEnvironment> le = std::make_shared<StickGameWithOpponent>();
        lgpRep = std::make_shared<Representations::LGP>(le->getInputDimensions(), Data::DataType::type<double>({8}), set, Data::DataType::type<double>({1}), 5, 20);
        tpgRep = std::make_shared<Representations::TPG>(le->getInputDimensions(), 3, 2, 5);
        representations = {lgpRep, tpgRep};

        RNG::RNG rng;
        rng.setSeed(0);
        Mutation::PointMutator mutator(0.5, 0.5, 0.5);
        const std::vector<std::shared_ptr<Individual>> members = Mutation::initIndividuals(mutator, lgpRep, GraphBased::GenotypeGenerator(lgpRep->getGenotypeConstraint()), 6, rng);
        const std::vector<std::shared_ptr<const Individual>> constMembers(members.begin(), members.end());
        const std::vector<std::shared_ptr<Individual>> vertices = Mutation::initIndividuals(mutator, tpgRep, *tpgGenerator(*tpgRep, constMembers, {}), 3, rng);
        const std::vector<std::shared_ptr<const Individual>> constVertices(vertices.begin(), vertices.end());
        const std::vector<std::shared_ptr<Individual>> tangled = Mutation::initIndividuals(mutator, tpgRep, *tpgGenerator(*tpgRep, constMembers, constVertices), 3, rng);
        roots.assign(vertices.begin(), vertices.end());
        roots.insert(roots.end(), tangled.begin(), tangled.end());
    }

    // Check that an imported individual is the same as the original one: same representation, same values, same introns,
    // and the individuals it references are the same too.
    static void expectSameIndividual(const Individual& original, const Individual& imported)
    {
        ASSERT_EQ(&original.getRepresentation(), &imported.getRepresentation());
        const GraphBased::Genotype& originalGenotype = original.getGenotype();
        const GraphBased::Genotype& importedGenotype = imported.getGenotype();
        ASSERT_EQ(originalGenotype.getSize(), importedGenotype.getSize());
        for(size_t idxGroup = 0; idxGroup < originalGenotype.getSize(); idxGroup++) {
            const GraphBased::NodeGroup& originalGroup = originalGenotype.getNodeGroup(idxGroup);
            const GraphBased::NodeGroup& importedGroup = importedGenotype.getNodeGroup(idxGroup);
            ASSERT_EQ(originalGroup.getSize(), importedGroup.getSize());
            for(size_t idxNode = 0; idxNode < originalGroup.getSize(); idxNode++) {
                const GraphBased::GPNode& originalNode = originalGroup.getNode(idxNode);
                const GraphBased::GPNode& importedNode = importedGroup.getNode(idxNode);
                EXPECT_EQ(originalNode.getIsIntron(), importedNode.getIsIntron());
                ASSERT_EQ(originalNode.getSize(), importedNode.getSize());
                for(size_t idxValue = 0; idxValue < originalNode.getSize(); idxValue++) {
                    const Data::DataValue& originalValue = originalNode.getValue(idxValue);
                    const Data::DataValue& importedValue = importedNode.getValue(idxValue);
                    if(originalValue.getElementType() == typeid(std::shared_ptr<const Individual>)) {
                        ASSERT_EQ(importedValue.getElementType(), typeid(std::shared_ptr<const Individual>));
                        expectSameIndividual(*originalValue.getScalar<std::shared_ptr<const Individual>>(), *importedValue.getScalar<std::shared_ptr<const Individual>>());
                    } else {
                        EXPECT_EQ(originalValue, importedValue);
                    }
                }
            }
        }
    }

    // A TPG evolution on the stick game, its members being the LGP of the test.
    std::shared_ptr<Evolution::Module> buildTPGEvolution(Evolution::OriginTPGEvolutionConfig& config) const
    {
        config.sizePopulation = 10;
        config.nbOffspring = 10;
        config.nbTrainingIterations = 1;
        config.nbThreads = 1;
        config.archiveSize = 10;
        // Without the protection of the aggregated teams, the graphs stay small and the test fast.
        config.protectAggregated = false;
        config.logOutput = nullptr;
        config.representation = tpgRep;
        config.memberRepresentation = lgpRep;
        config.problem = std::make_shared<Evaluation::ReinforcementProblem>(std::make_shared<StickGameWithOpponent>(), 4);
        return config.buildModule();
    }
};

TEST_F(PopulationDotImporterTest, roundTripWithIntrons)
{
    std::istringstream file(File::PopulationDotExporter::toString(roots, true));
    const std::vector<std::shared_ptr<Individual>> imported = File::PopulationDotImporter::read(file, representations);

    // The same roots, in the same order, with the same genotypes and the same graphs.
    ASSERT_EQ(imported.size(), roots.size());
    for(size_t idx = 0; idx < roots.size(); idx++) {
        expectSameIndividual(*roots.at(idx), *imported.at(idx));
    }

    // The new IDs keep the order of the IDs of the file.
    for(size_t idxA = 0; idxA < roots.size(); idxA++) {
        for(size_t idxB = 0; idxB < roots.size(); idxB++) {
            EXPECT_EQ(roots.at(idxA)->getIndividualID() < roots.at(idxB)->getIndividualID(),
                      imported.at(idxA)->getIndividualID() < imported.at(idxB)->getIndividualID());
        }
    }
}

TEST_F(PopulationDotImporterTest, roundTripWithoutIntrons)
{
    // By default the introns are not exported: the imported members only have the useful nodes, which are not introns.
    std::istringstream file(File::PopulationDotExporter::toString(roots));
    const std::vector<std::shared_ptr<Individual>> imported = File::PopulationDotImporter::read(file, representations);
    ASSERT_EQ(imported.size(), roots.size());

    const GraphBased::GPNode& originalNode = roots.at(0)->getGenotype().getNodeGroup(0).getNode(0);
    const GraphBased::GPNode& importedNode = imported.at(0)->getGenotype().getNodeGroup(0).getNode(0);
    const Individual& originalMember = *originalNode.getValue(0).getScalar<std::shared_ptr<const Individual>>();
    const Individual& importedMember = *importedNode.getValue(0).getScalar<std::shared_ptr<const Individual>>();
    size_t nbUseful = 0;
    const GraphBased::NodeGroup& originalLines = originalMember.getGenotype().getNodeGroup(0);
    for(size_t idx = 0; idx < originalLines.getSize(); idx++) {
        nbUseful += originalLines.getNode(idx).getIsIntron() ? 0 : 1;
    }
    const GraphBased::NodeGroup& importedLines = importedMember.getGenotype().getNodeGroup(0);
    EXPECT_EQ(importedLines.getSize(), nbUseful);
    for(size_t idx = 0; idx < importedLines.getSize(); idx++) {
        EXPECT_FALSE(importedLines.getNode(idx).getIsIntron());
    }
}

TEST_F(PopulationDotImporterTest, referencesToNewerIndividuals)
{
    // A vertex referencing members created after it (as the mutation of a vertex creates members) is valid once imported.
    std::shared_ptr<Individual> vertex = std::make_shared<Individual>(tpgRep);
    RNG::RNG rng;
    Mutation::PointMutator mutator(0.5, 0.5, 0.5);
    const std::vector<std::shared_ptr<Individual>> members = Mutation::initIndividuals(mutator, lgpRep, GraphBased::GenotypeGenerator(lgpRep->getGenotypeConstraint()), 2, rng);
    std::unique_ptr<GraphBased::NodeGroup> group = std::make_unique<GraphBased::NodeGroup>();
    for(size_t idx = 0; idx < members.size(); idx++) {
        std::vector<Data::DataValue> values;
        values.push_back(Data::DataValue::scalar<std::shared_ptr<const Individual>>(members.at(idx)));
        values.push_back(Data::DataValue::scalar<size_t>(idx));
        group->addNode(std::make_unique<GraphBased::GPNode>(values));
    }
    vertex->setGenotype(GraphBased::Genotype::singleGroupGenotype(std::move(group)));
    ASSERT_TRUE(vertex->isValid());
    ASSERT_LT(vertex->getIndividualID(), members.at(0)->getIndividualID());

    std::istringstream file(File::PopulationDotExporter::toString(std::vector<std::shared_ptr<const Individual>>{vertex}, true));
    const std::vector<std::shared_ptr<Individual>> imported = File::PopulationDotImporter::read(file, representations);
    ASSERT_EQ(imported.size(), 1U);
    EXPECT_TRUE(imported.at(0)->isValid());
    expectSameIndividual(*vertex, *imported.at(0));
}

TEST_F(PopulationDotImporterTest, importToData)
{
    const std::string path = (std::filesystem::temp_directory_path() / "gegelati_import_test.dot").string();
    File::PopulationDotExporter::exportToFile(path, roots, true);

    Evolution::EvolutionData data;
    File::PopulationDotImporter::importToData(path, representations, data, "Population");
    ASSERT_TRUE(data.has("Population"));
    EXPECT_EQ(data.get<std::vector<std::shared_ptr<Individual>>>("Population")->size(), roots.size());
    std::filesystem::remove(path);

    EXPECT_THROW(File::PopulationDotImporter::importFromFile(path, representations), std::runtime_error) << "The file does not exist anymore";
}

TEST_F(PopulationDotImporterTest, unknownRepresentation)
{
    std::istringstream file(File::PopulationDotExporter::toString(roots));
    EXPECT_THROW(File::PopulationDotImporter::read(file, {tpgRep}), std::runtime_error) << "The members have no representation";
}

TEST_F(PopulationDotImporterTest, importIsDeterministic)
{
    // The same file imported twice, from the same state of the counters, gives the same individuals: same IDs and same genotypes.
    const std::string text = File::PopulationDotExporter::toString(roots, true);
    std::vector<std::string> exported;
    for(size_t idx = 0; idx < 2; idx++) {
        CounterReset::counterReset();
        std::istringstream file(text);
        const std::vector<std::shared_ptr<Individual>> imported = File::PopulationDotImporter::read(file, representations);
        std::string again = File::PopulationDotExporter::toString(std::vector<std::shared_ptr<const Individual>>(imported.begin(), imported.end()), true);
        again.erase(0, again.find("digraph")); // The header has the date of the export.
        exported.push_back(again);
    }
    EXPECT_EQ(exported.at(0), exported.at(1));
}

TEST_F(PopulationDotImporterTest, importedIndividualsBehaveTheSame)
{
    // The original and the imported roots play the same games and get the same rewards (the introns are exported, so that the members
    // keep the number of lines their representation requires).
    std::istringstream file(File::PopulationDotExporter::toString(roots, true));
    const std::vector<std::shared_ptr<Individual>> imported = File::PopulationDotImporter::read(file, representations);
    ASSERT_EQ(imported.size(), roots.size());
    for(size_t idx = 0; idx < roots.size(); idx++) {
        ASSERT_TRUE(roots.at(idx)->isValid()) << "original " << idx;
        ASSERT_TRUE(imported.at(idx)->isValid()) << "imported " << idx;
    }

    Evaluation::ReinforcementProblem problem(std::make_shared<StickGameWithOpponent>(), 4);
    std::shared_ptr<Metrics::MetricMapTemplate> metrics = std::make_shared<Metrics::MetricMapTemplate>(std::make_unique<Metrics::RewardsMetric>());
    FitnessAssignment::DefaultAssigner assigner(Metrics::RewardsMetric::staticHash());
    const std::map<uint64_t, double> originalFitness = assigner.assignFitness(Evaluation::evaluateIndividuals(roots, problem, metrics, 5, 0, Evaluation::Mode::TRAINING));
    const std::map<uint64_t, double> importedFitness = assigner.assignFitness(
        Evaluation::evaluateIndividuals(std::vector<std::shared_ptr<const Individual>>(imported.begin(), imported.end()), problem, metrics, 5, 0, Evaluation::Mode::TRAINING));
    for(size_t idx = 0; idx < roots.size(); idx++) {
        EXPECT_EQ(originalFitness.at(roots.at(idx)->getIndividualID()), importedFitness.at(imported.at(idx)->getIndividualID())) << idx;
    }
}

TEST_F(PopulationDotImporterTest, unknownValueType)
{
    // A value with a type that is neither a scalar type nor a representation is refused as such.
    std::string text = File::PopulationDotExporter::toString(roots, true);
    text.replace(text.find("st:"), 3, "fd:");
    std::istringstream file(text);
    try {
        File::PopulationDotImporter::read(file, representations);
        FAIL() << "The value has an unknown type";
    } catch(const std::runtime_error& error) {
        EXPECT_NE(std::string(error.what()).find("unknown type"), std::string::npos) << error.what();
    }
}

TEST_F(PopulationDotImporterTest, evolutionContinuedAfterImport)
{
    // First evolution: 20 generations in a row.
    CounterReset::counterReset();
    Evolution::OriginTPGEvolutionConfig fullConfig;
    fullConfig.protectAggregated = true;
    std::shared_ptr<Evolution::Module> fullEvolution = buildTPGEvolution(fullConfig);
    Evolution::EvolutionData& fullData = fullEvolution->evolve(20, 7);

    // Second evolution: 10 generations, then the population is exported and imported back in place of the original one.
    CounterReset::counterReset();
    Evolution::OriginTPGEvolutionConfig cutConfig;
    cutConfig.protectAggregated = true;
    std::shared_ptr<Evolution::Module> cutEvolution = buildTPGEvolution(cutConfig);
    Evolution::EvolutionData& cutData = cutEvolution->evolve(10, 7);
    const std::vector<std::shared_ptr<Individual>>& exportedPopulation = *cutData.get<std::vector<std::shared_ptr<Individual>>>(cutConfig.names.population);
    std::istringstream file(File::PopulationDotExporter::toString(std::vector<std::shared_ptr<const Individual>>(exportedPopulation.begin(), exportedPopulation.end()), true));
    cutData.set(cutConfig.names.population, std::make_shared<std::vector<std::shared_ptr<Individual>>>(File::PopulationDotImporter::read(file, representations)));

    // The fitness of the population is by ID: the imported population is evaluated again, with the evaluations of the 10th generation.
    cutEvolution->find(cutConfig.names.trainingEvaluationStep)->apply(cutData);
    cutEvolution->find(cutConfig.names.trainingFitnessStep)->apply(cutData);

    // Then the 10 next generations.
    cutEvolution->runGenerations(10);

    // Both evolutions end with the same population, with the same fitness, and the same state of the random number generator.
    const std::vector<std::shared_ptr<Individual>>& fullPopulation = *fullData.get<std::vector<std::shared_ptr<Individual>>>(fullConfig.names.population);
    const std::vector<std::shared_ptr<Individual>>& cutPopulation = *cutData.get<std::vector<std::shared_ptr<Individual>>>(cutConfig.names.population);
    const std::map<uint64_t, double>& fullFitness = *fullData.get<std::map<uint64_t, double>>(fullConfig.names.trainingFitness);
    const std::map<uint64_t, double>& cutFitness = *cutData.get<std::map<uint64_t, double>>(cutConfig.names.trainingFitness);
    ASSERT_EQ(fullPopulation.size(), cutPopulation.size());
    for(size_t idx = 0; idx < fullPopulation.size(); idx++) {
        expectSameIndividual(*fullPopulation.at(idx), *cutPopulation.at(idx));
        EXPECT_EQ(fullFitness.at(fullPopulation.at(idx)->getIndividualID()), cutFitness.at(cutPopulation.at(idx)->getIndividualID())) << idx;
    }
    EXPECT_EQ(fullData.rng.uniformSample<uint64_t>(0, UINT64_MAX), cutData.rng.uniformSample<uint64_t>(0, UINT64_MAX));
}
