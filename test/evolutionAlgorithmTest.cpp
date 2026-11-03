/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
 * Pierre-Yves Le Rolland-Raumer <plerolla@insa-rennes.fr> (2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2023 - 2025)
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

#include <algorithm>
#include <fstream>
#include <gtest/gtest.h>
#include <numeric>
#include <sstream>
#include <string>

#include "instructions/set.h"
#include "instructions/lambdaInstruction.h"
#include "learn/stickGameWithOpponent.h"
#include "evaluation/reinforcementProblem.h"
#include "evaluation/evaluator.h"
#include "evaluation/parallelEvaluator.h"
#include "metrics/rewardsMetric.h"

#include "evolution/basicEvolution.h"
#include "learn/tpgGenerator.h"
#include "evaluation/dataset.h"
#include "evolution/originTPGEvolution.h"
#include "evolution/steps/exportStep.h"
#include "evolution/steps/lambdaStep.h"
#include "evolution/steps/logStep.h"

#include "dimensions/individualValueGenerator.h"

#include "representations/LGP.h"
#include "representations/TPG.h"

#include "mutation/pointMutator.h"
#include "reproduction/replicator.h"
#include "selection/randomSelector.h"
#include "selection/truncationSelector.h"
#include "fitnessAssignment/defaultAssigner.h"

#include "util/counterReset.h"

namespace {
    /// Register individuals by ID, to find the individuals selected by their ID.
    void addToIndex(std::unordered_map<uint64_t, std::shared_ptr<const Individual>>& index, const std::vector<std::shared_ptr<const Individual>>& individuals)
    {
        for(const std::shared_ptr<const Individual>& individual: individuals) {
            index.emplace(individual->getIndividualID(), individual);
        }
    }

    /// Return the individuals identified by their ID.
    std::vector<std::shared_ptr<const Individual>> resolve(const std::unordered_map<uint64_t, std::shared_ptr<const Individual>>& index, const std::vector<uint64_t>& ids)
    {
        std::vector<std::shared_ptr<const Individual>> individuals;
        for(uint64_t id: ids) {
            individuals.push_back(index.at(id));
        }
        return individuals;
    }
}

class EvolutionAlgorithmTest : public ::testing::Test
{
  protected:
    Instructions::Set set;
    std::vector<std::unique_ptr<Instructions::Instruction>> instructions;
    std::shared_ptr<Evaluation::ReinforcementEnvironment> le;


    virtual void SetUp()
    {
        CounterReset::counterReset();
        auto add = [](double a, double b) -> double { return a + b; };
        auto minus = [](int a, double b) -> double { return a - b; };
        auto times = [](double a, int b) -> double { return a * b; };
        auto div = [](int a, double b) -> double { return a / b; };
        
        instructions.push_back(std::make_unique<Instructions::LambdaInstruction<double, double, double>>(add));
        instructions.push_back(std::make_unique<Instructions::LambdaInstruction<double, int, double>>(minus));
        instructions.push_back(std::make_unique<Instructions::LambdaInstruction<double, double, int>>(times));
        instructions.push_back(std::make_unique<Instructions::LambdaInstruction<double, int, double>>(div));
        for(const auto& instruction: instructions) {
            set.add(*instruction);
        }
    
        le = std::make_shared<StickGameWithOpponent>();
    }
};


TEST_F(EvolutionAlgorithmTest, customEvolutionLGP) {
    RNG::RNG rng;
    rng.setSeed(4);

    // Create representation
    auto lgpRep = std::make_shared<Representations::LGP>(le->getInputDimensions(), Data::DataType::type<double>({8}), set, Data::DataType::type<double>({3}), 10, 20);
    lgpRep->addOutputFunction(std::make_unique<Dimensions::ActivationFunctions::ArgMax<double>>(lgpRep->getDimensionFlow().getOutputDimension()));


    // Create Mutator and breeder
    Reproduction::Replicator breeder;
    Mutation::PointMutator mutator(0.5, 0.1, 0.1);

    // Create selectors
    Selection::RandomSelector parentSelection(true);
    Selection::TruncationSelector survivingSelection;
    FitnessAssignment::DefaultAssigner assigner;
    
    // Create evaluationAgent
    Evaluation::ReinforcementProblem problem(le, 3);
    Evaluation::ParallelEvaluator evaluator(10);
    std::shared_ptr<Metrics::MetricMapTemplate> metrics 
        = std::make_shared<Metrics::MetricMapTemplate>(std::make_unique<Metrics::RewardsMetric>());


    size_t sizePopulation = 100;
    size_t nbOffspring = 100;

    // Initialize population
    std::unordered_map<uint64_t, std::shared_ptr<const Individual>> index;
    std::vector<std::shared_ptr<Individual>> individuals = Mutation::initIndividuals(mutator, lgpRep, GraphBased::GenotypeGenerator(lgpRep->getGenotypeConstraint()), sizePopulation, rng);
    std::vector<std::shared_ptr<const Individual>> population(individuals.begin(), individuals.end());
    addToIndex(index, population);

    // Initial evaluation
    std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>> evaluationResults = 
            evaluator.evaluateIndividuals(population, problem, metrics, 3, 0, Evaluation::Mode::TRAINING);
    // Fitness assignment 
    std::map<uint64_t, double> individualFitness = assigner.assignFitness(evaluationResults, Metrics::RewardsMetric::staticHash());

    size_t nbGen = 20;
    for (size_t idxGen = 0; idxGen < nbGen; idxGen++) {

        // Parent selection 
        std::vector<std::shared_ptr<const Individual>> parents = resolve(index, parentSelection.select(individualFitness, nbOffspring, rng));

        // Reproduce the parents
        std::vector<std::shared_ptr<Individual>> offspring = breeder.reproduce(parents, nbOffspring, rng);

        // Mutate the offspring
        Mutation::mutateIndividuals(mutator, offspring, GraphBased::GenotypeGenerator(lgpRep->getGenotypeConstraint()), rng);

        // Evaluate the population
        std::vector<std::shared_ptr<const Individual>> evaluatedIndividuals(population);
        evaluatedIndividuals.insert(evaluatedIndividuals.end(), offspring.begin(), offspring.end());
        addToIndex(index, std::vector<std::shared_ptr<const Individual>>(offspring.begin(), offspring.end()));
        evaluationResults = evaluator.evaluateIndividuals(evaluatedIndividuals, problem, metrics, 3, idxGen, Evaluation::Mode::TRAINING);

        // Do replacement
        individualFitness = assigner.assignFitness(evaluationResults, Metrics::RewardsMetric::staticHash());
        population = resolve(index, survivingSelection.select(individualFitness, sizePopulation, rng));


        // Print best individual
        std::cout<<"At gen: "<<idxGen<<" - ID: "<<individualFitness.begin()->first 
        <<" and score: " << individualFitness.begin()->second <<std::endl;
    }
    ASSERT_EQ(1704165046166301501U, rng.uniformSample<uint64_t>(0, UINT64_MAX)) << "bouh bouh bouh th determinism";
}

TEST_F(EvolutionAlgorithmTest, customEvolutionLGP2) {

    // Create representation
    auto lgpRep = std::make_shared<Representations::LGP>(le->getInputDimensions(), Data::DataType::type<double>({8}), set, Data::DataType::type<double>({3}), 10, 20);
    lgpRep->addOutputFunction(std::make_unique<Dimensions::ActivationFunctions::ArgMax<double>>(lgpRep->getDimensionFlow().getOutputDimension()));
    
    // Create evaluationAgent
    auto problem = std::make_shared<Evaluation::ReinforcementProblem>(le, 3);

    Evolution::BasicEvolution basic;
    basic.representation = lgpRep;
    basic.problem = problem;
    basic.sizePopulation = 100;
    basic.nbOffspring = 100;
    basic.nbTrainingIterations = 3;
    basic.nbThreads = 10;
    auto root = basic.build();
    Evolution::EvolutionData state(4);
    root->add(std::make_shared<Evolution::LogStep>(std::cout, Evolution::BasicEvolution::Keys().individuals,
        std::vector<std::pair<std::string, Evolution::Key<std::map<uint64_t, double>>>>{{"Fit", Evolution::BasicEvolution::Keys().trainingFitness}}));
    root->resume(state, 20);

    ASSERT_EQ(1704165046166301501U, state.rng.uniformSample<uint64_t>(0, UINT64_MAX)) << "bouh bouh bouh th determinism";
}

TEST_F(EvolutionAlgorithmTest, customEvolutionTPGPlusLGP) {
    RNG::RNG rng;
    rng.setSeed(4);

    // Create representations
    auto lgpRep = std::make_shared<Representations::LGP>(le->getInputDimensions(), Data::DataType::type<double>({8}), set, Data::DataType::type<double>({1}), 10);
    auto tpgRep = std::make_shared<Representations::TPG>(le->getInputDimensions(), 3, 2, 10);

    // Create Mutator and breeder
    Reproduction::Replicator breeder;
    Mutation::PointMutator mutator(0.5, 0.1, 0.1);

    // Create selectors
    Selection::RandomSelector parentSelection(true);
    Selection::TruncationSelector survivingSelection;
    FitnessAssignment::DefaultAssigner assigner;
    
    // Create evaluationAgent
    Evaluation::ReinforcementProblem problem(le, 3);
    Evaluation::ParallelEvaluator evaluator(20);
    std::shared_ptr<Metrics::MetricMapTemplate> metrics 
        = std::make_shared<Metrics::MetricMapTemplate>(std::make_unique<Metrics::RewardsMetric>());

    size_t sizePopulation = 100;
    size_t nbOffspring = 100;
    std::unordered_map<uint64_t, std::shared_ptr<const Individual>> index;

    // Initialize LGP population
    std::vector<std::shared_ptr<Individual>> individualsLGP = Mutation::initIndividuals(mutator, lgpRep, GraphBased::GenotypeGenerator(lgpRep->getGenotypeConstraint()), sizePopulation, rng);
    std::vector<std::shared_ptr<const Individual>> populationLGP(individualsLGP.begin(), individualsLGP.end());
    addToIndex(index, populationLGP);
    std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>> evaluationResultsLGP = 
            evaluator.evaluateIndividuals(populationLGP, problem, metrics, 3, 0, Evaluation::Mode::TRAINING);
    // Fitness assignment 
    std::map<uint64_t, double> individualFitnessLGP = assigner.assignFitness(evaluationResultsLGP, Metrics::RewardsMetric::staticHash());

    // Initialize TPG population
    std::vector<std::shared_ptr<const Individual>> members = resolve(index, parentSelection.select(individualFitnessLGP, nbOffspring, rng));
    std::vector<std::shared_ptr<Individual>> individualsTPG = Mutation::initIndividuals(mutator, tpgRep, *tpgGenerator(*tpgRep, members, {}), sizePopulation, rng);
    std::vector<std::shared_ptr<const Individual>> populationTPG(individualsTPG.begin(), individualsTPG.end());
    addToIndex(index, populationTPG);
    std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>> evaluationResultsTPG = 
            evaluator.evaluateIndividuals(populationTPG, problem, metrics, 3, 0, Evaluation::Mode::TRAINING);
    // Fitness assignment 
    std::map<uint64_t, double> individualFitnessTPG = assigner.assignFitness(evaluationResultsTPG, Metrics::RewardsMetric::staticHash());

    size_t nbGen = 20;
    for (size_t idxGen = 0; idxGen < nbGen; idxGen++) {

        // LGP Evolution : variation
        std::vector<std::shared_ptr<const Individual>> parentsLGP = resolve(index, parentSelection.select(individualFitnessLGP, nbOffspring, rng));
        std::vector<std::shared_ptr<Individual>> offspringLGP = breeder.reproduce(parentsLGP, nbOffspring, rng);
        Mutation::mutateIndividuals(mutator, offspringLGP, GraphBased::GenotypeGenerator(lgpRep->getGenotypeConstraint()), rng);
        // LGP Evolution : evaluation
        std::vector<std::shared_ptr<const Individual>> evaluatedIndividualsLGP(populationLGP);
        evaluatedIndividualsLGP.insert(evaluatedIndividualsLGP.end(), offspringLGP.begin(), offspringLGP.end());
        addToIndex(index, std::vector<std::shared_ptr<const Individual>>(offspringLGP.begin(), offspringLGP.end()));
        evaluationResultsLGP = evaluator.evaluateIndividuals(evaluatedIndividualsLGP, problem, metrics, 3, idxGen, Evaluation::Mode::TRAINING);
        // LGP Evolution : replacement
        individualFitnessLGP = assigner.assignFitness(evaluationResultsLGP, Metrics::RewardsMetric::staticHash());
        populationLGP = resolve(index, survivingSelection.select(individualFitnessLGP, sizePopulation, rng));


        // TPG Evolution : variation
        std::vector<std::shared_ptr<const Individual>> parentsTPG = resolve(index, parentSelection.select(individualFitnessTPG, nbOffspring, rng));
        std::vector<std::shared_ptr<Individual>> offspringTPG = breeder.reproduce(parentsTPG, nbOffspring, rng);

        
        std::vector<std::shared_ptr<const Individual>> members = resolve(index, parentSelection.select(individualFitnessLGP, nbOffspring, rng));
        std::vector<std::shared_ptr<const Individual>> tangleds = resolve(index, parentSelection.select(individualFitnessTPG, nbOffspring, rng));
        Mutation::mutateIndividuals(mutator, offspringTPG, *tpgGenerator(*tpgRep, members, tangleds), rng);
        // TPG Evolution : evaluation
        std::vector<std::shared_ptr<const Individual>> evaluatedIndividualsTPG(populationTPG);
        evaluatedIndividualsTPG.insert(evaluatedIndividualsTPG.end(), offspringTPG.begin(), offspringTPG.end());
        addToIndex(index, std::vector<std::shared_ptr<const Individual>>(offspringTPG.begin(), offspringTPG.end()));
        evaluationResultsTPG = evaluator.evaluateIndividuals(evaluatedIndividualsTPG, problem, metrics, 3, idxGen, Evaluation::Mode::TRAINING);
        // TPG Evolution : replacement
        individualFitnessTPG = assigner.assignFitness(evaluationResultsTPG, Metrics::RewardsMetric::staticHash());
        populationTPG = resolve(index, survivingSelection.select(individualFitnessTPG, sizePopulation, rng));


        // Print best individual
        std::cout<<"ID: "<<individualFitnessTPG.begin()->first 
        <<" and score: " << individualFitnessTPG.begin()->second <<std::endl;
    }

    ASSERT_EQ(2919827829777650766U, rng.uniformSample<uint64_t>(0, UINT64_MAX)) << "bouh bouh bouh th determinism";
}


TEST_F(EvolutionAlgorithmTest, customEvolutionTPGPlusLGP2) {
    RNG::RNG rng;
    rng.setSeed(4);

    // Create representations
    auto lgpRep = std::make_shared<Representations::LGP>(le->getInputDimensions(), Data::DataType::type<double>({8}), set, Data::DataType::type<double>({3}), 10, 20);
    auto tpgRep = std::make_shared<Representations::TPG>(le->getInputDimensions(), 3, 2, 10);

    Mutation::PointMutator mutator(0.5, 0.1, 0.1);

    /** CUSTOM GENERATOR **/
    GraphBased::NodeGenerator bidNodes;
    bidNodes.addGenerator(Dimensions::IndividualValueGenerator(lgpRep, mutator));
    bidNodes.addGenerator(Dimensions::NumericUniformGenerator<size_t>(0, 3 - 1));
    GraphBased::GenotypeGenerator customGenerator(bidNodes, std::make_pair(2U, 10U), std::make_pair(2U, 2U));
    /** CUSTOM GENERATOR **/

    // Create Mutator and breeder
    Reproduction::Replicator breeder;

    // Create selectors
    Selection::RandomSelector parentSelection(true);
    Selection::TruncationSelector survivingSelection;
    FitnessAssignment::DefaultAssigner assigner;
    
    // Create evaluationAgent
    Evaluation::ReinforcementProblem problem(le, 3);
    Evaluation::ParallelEvaluator evaluator(20);
    std::shared_ptr<Metrics::MetricMapTemplate> metrics 
        = std::make_shared<Metrics::MetricMapTemplate>(std::make_unique<Metrics::RewardsMetric>());


    size_t sizePopulation = 100;
    size_t nbOffspring = 100;
    std::unordered_map<uint64_t, std::shared_ptr<const Individual>> index;

    std::vector<std::shared_ptr<Individual>> individualsTPG = Mutation::initIndividuals(mutator, tpgRep, customGenerator, sizePopulation, rng);
    std::vector<std::shared_ptr<const Individual>> populationTPG(individualsTPG.begin(), individualsTPG.end());
    addToIndex(index, populationTPG);
    std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>> evaluationResultsTPG = 
            evaluator.evaluateIndividuals(populationTPG, problem, metrics, 3, UINT64_MAX, Evaluation::Mode::TRAINING);
    // Fitness assignment 
    std::map<uint64_t, double> individualFitnessTPG = assigner.assignFitness(evaluationResultsTPG, Metrics::RewardsMetric::staticHash());

    size_t nbGen = 20;
    for (size_t idxGen = 0; idxGen < nbGen; idxGen++) {

        // TPG Evolution : variation
        std::vector<std::shared_ptr<const Individual>> parentsTPG = resolve(index, parentSelection.select(individualFitnessTPG, nbOffspring, rng));
        std::vector<std::shared_ptr<Individual>> offspringTPG = breeder.reproduce(parentsTPG, nbOffspring, rng);

        Mutation::mutateIndividuals(mutator, offspringTPG, customGenerator, rng);
        // TPG Evolution : evaluation
        std::vector<std::shared_ptr<const Individual>> evaluatedIndividualsTPG(populationTPG);
        evaluatedIndividualsTPG.insert(evaluatedIndividualsTPG.end(), offspringTPG.begin(), offspringTPG.end());
        addToIndex(index, std::vector<std::shared_ptr<const Individual>>(offspringTPG.begin(), offspringTPG.end()));
        evaluationResultsTPG = evaluator.evaluateIndividuals(evaluatedIndividualsTPG, problem, metrics, 3, idxGen, Evaluation::Mode::TRAINING);
        // TPG Evolution : replacement
        individualFitnessTPG = assigner.assignFitness(evaluationResultsTPG, Metrics::RewardsMetric::staticHash());
        populationTPG = resolve(index, survivingSelection.select(individualFitnessTPG, sizePopulation, rng));

        // Print best individual
        std::cout<<"ID: "<<individualFitnessTPG.begin()->first << " and score: " << individualFitnessTPG.begin()->second<<std::endl;
    }
    ASSERT_EQ(Individual::getIndividualIDCounter(), 3585) << "boug boug ";
    
    ASSERT_EQ(8946111367163773343U, rng.uniformSample<uint64_t>(0, UINT64_MAX)) << "bouh bouh bouh th determinism";
}


namespace {
    // Settings of the TPG evolution of the tests, with the seed 4.
    Evolution::OriginTPGEvolution makeTPG(std::shared_ptr<const Representations::TPG> tpgRep, std::shared_ptr<const Representations::LGP> lgpRep,
                                          std::shared_ptr<Evaluation::Problem> problem, size_t sizePopulation, size_t nbOffspring, size_t nbTrainingIterations)
    {
        Evolution::OriginTPGEvolution tpg;
        tpg.representation = tpgRep;
        tpg.memberRepresentation = lgpRep;
        tpg.problem = problem;
        tpg.sizePopulation = sizePopulation;
        tpg.nbOffspring = nbOffspring;
        tpg.nbTrainingIterations = nbTrainingIterations;
        return tpg;
    }
}

TEST_F(EvolutionAlgorithmTest, originTPGPipeline) {
    // Create representations
    auto lgpRep = std::make_shared<Representations::LGP>(le->getInputDimensions(), Data::DataType::type<double>({8}), set, Data::DataType::type<double>({3}), 10, 20);
    auto tpgRep = std::make_shared<Representations::TPG>(le->getInputDimensions(), 3, 2, 10);

    // Create evaluationAgent
    auto problem = std::make_shared<Evaluation::ReinforcementProblem>(le, 3);

    auto root = makeTPG(tpgRep, lgpRep, problem, 100, 100, 3).build();
    Evolution::EvolutionData state(4);
    const Evolution::BasicEvolution::Keys population;
    root->add(std::make_shared<Evolution::LogStep>(std::cout, population.individuals,
        std::vector<std::pair<std::string, Evolution::Key<std::map<uint64_t, double>>>>{{"Fit", population.trainingFitness}}, Evolution::tpgLogColumns(population.individuals)));
    root->resume(state, 20);

    // Steps of the preset
    const std::string summary = root->summary();
    for(const std::string& stepName: {"CreateArchive", "ParentSelection", "Mutation", "SurvivorSelection", "ProtectAggregated", "UpdateArchive"}) {
        EXPECT_NE(summary.find(stepName), std::string::npos) << stepName;
    }
    EXPECT_EQ(summary.find("CacheMemberOutputs"), std::string::npos) << "Members are only cached for prediction problems";

    // The archive is a data of the population, filled with inputs of the population.
    const std::shared_ptr<const Evaluation::DataSet> archive = state.getIfAny(Evolution::OriginTPGEvolution::Keys().archive);
    ASSERT_NE(archive, nullptr);
    EXPECT_GT(archive->size(), 0U);
    EXPECT_LE(archive->size(), 50U);

    // Every vertex of the graphs is valid, with a number of nodes following the representation.
    const std::vector<std::shared_ptr<const Individual>> vertices = Evolution::getAllVertices(state.getIndividuals(population.individuals));
    ASSERT_FALSE(vertices.empty());
    for(const std::shared_ptr<const Individual>& vertex: vertices) {
        EXPECT_TRUE(vertex->isValid());
        EXPECT_LE(vertex->getGenotype().getNodeGroup(0).getSize(), 10U);
    }
    EXPECT_FALSE(Evolution::getMembersOf(vertices).empty());
    EXPECT_NE(state.bestIndividual(population.individuals, population.trainingFitness), nullptr);
}

TEST_F(EvolutionAlgorithmTest, originTPGPipelineIsDeterministicWithThreads) {
    auto lgpRep = std::make_shared<Representations::LGP>(le->getInputDimensions(), Data::DataType::type<double>({8}), set, Data::DataType::type<double>({3}), 10, 20);
    auto tpgRep = std::make_shared<Representations::TPG>(le->getInputDimensions(), 3, 2, 10);
    auto problem = std::make_shared<Evaluation::ReinforcementProblem>(le, 3);

    uint64_t lastRng[2];
    size_t nbIndividuals[2];
    size_t nbVertices[2];
    const size_t nbThreads[2] = {1, 10};
    for(size_t idx = 0; idx < 2; idx++) {
        CounterReset::counterReset();
        Evolution::OriginTPGEvolution tpg = makeTPG(tpgRep, lgpRep, problem, 100, 100, 3);
        tpg.nbThreads = nbThreads[idx];
        auto root = tpg.build();
        Evolution::EvolutionData state(4);
        root->resume(state, 20);
        lastRng[idx] = state.rng.uniformSample<uint64_t>(0, UINT64_MAX);
        nbIndividuals[idx] = Individual::getIndividualIDCounter();
        nbVertices[idx] = Evolution::getAllVertices(state.getIndividuals(Evolution::BasicEvolution::Keys().individuals)).size();
    }

    EXPECT_EQ(lastRng[0], lastRng[1]) << "Evolution should not depend on the number of threads";
    EXPECT_EQ(nbIndividuals[0], nbIndividuals[1]) << "Evolution should not depend on the number of threads";
    EXPECT_EQ(nbVertices[0], nbVertices[1]) << "Evolution should not depend on the number of threads";
}

TEST_F(EvolutionAlgorithmTest, originTPGPipelineProtectsAggregatedVertices) {
    auto lgpRep = std::make_shared<Representations::LGP>(le->getInputDimensions(), Data::DataType::type<double>({8}), set, Data::DataType::type<double>({3}), 10, 20);
    auto tpgRep = std::make_shared<Representations::TPG>(le->getInputDimensions(), 3, 2, 10);
    auto problem = std::make_shared<Evaluation::ReinforcementProblem>(le, 3);

    Evolution::OriginTPGEvolution tpg = makeTPG(tpgRep, lgpRep, problem, 50, 100, 3);
    tpg.archiveSize = 0;
    auto root = tpg.build();
    Evolution::EvolutionData state(4);
    EXPECT_TRUE(root->getInitialization()->has("ProtectAggregated"));
    EXPECT_TRUE(root->has("ProtectAggregated"));

    // After each generation, no root is referenced by another root.
    size_t nbChecks = 0;
    const auto individuals = Evolution::BasicEvolution::Keys().individuals;
    root->add(std::make_shared<Evolution::LambdaStep>("CheckRoots", [&nbChecks, individuals](Evolution::EvolutionData& state) {
        std::set<uint64_t> roots;
        for(const auto& root: *state.get(individuals)) {
            roots.insert(root->getIndividualID());
        }
        for(const auto& root: *state.get(individuals)) {
            for(const auto& subVertex: Evolution::getSubVertices(*root)) {
                EXPECT_EQ(roots.count(subVertex->getIndividualID()), 0U) << "A referenced vertex should not be a root";
            }
        }
        nbChecks++;
    }, std::set<std::string>{individuals.name()}));
    root->resume(state, 30);
    const size_t nbProtected = state.get(Evolution::OriginTPGEvolution::Keys().protectedVertices)->size();

    EXPECT_EQ(nbChecks, 30U);
    // Every protected vertex is still reachable, through the graphs of the population.
    const std::vector<std::shared_ptr<const Individual>> roots = state.getIndividuals(individuals);
    EXPECT_GE(Evolution::getAllVertices(roots).size(), roots.size() + nbProtected);
}

TEST_F(EvolutionAlgorithmTest, originTPGPipelineWithoutProtection) {
    auto lgpRep = std::make_shared<Representations::LGP>(le->getInputDimensions(), Data::DataType::type<double>({8}), set, Data::DataType::type<double>({3}), 10, 20);
    auto tpgRep = std::make_shared<Representations::TPG>(le->getInputDimensions(), 3, 2, 10);
    auto problem = std::make_shared<Evaluation::ReinforcementProblem>(le, 3);

    Evolution::OriginTPGEvolution tpg = makeTPG(tpgRep, lgpRep, problem, 50, 100, 3);
    tpg.archiveSize = 0;
    tpg.protectAggregated = false;
    auto root = tpg.build();
    Evolution::EvolutionData state(4);
    EXPECT_FALSE(root->has("ProtectAggregated"));
    EXPECT_FALSE(root->has("UpdateArchive"));
    EXPECT_FALSE(root->getInitialization()->has("CreateArchive"));
    root->resume(state, 5);
    EXPECT_EQ(state.getIndividuals(Evolution::BasicEvolution::Keys().individuals).size(), 50U);
}

TEST_F(EvolutionAlgorithmTest, originTPGSettingsAreChecked) {
    auto lgpRep = std::make_shared<Representations::LGP>(le->getInputDimensions(), Data::DataType::type<double>({8}), set, Data::DataType::type<double>({3}), 10, 20);
    auto tpgRep = std::make_shared<Representations::TPG>(le->getInputDimensions(), 3, 2, 10);
    auto problem = std::make_shared<Evaluation::ReinforcementProblem>(le, 3);

    // Order of the steps of the preset.
    Evolution::OriginTPGEvolution tpg = makeTPG(tpgRep, lgpRep, problem, 20, 20, 2);
    auto root = tpg.build();
    Evolution::EvolutionData state(4);
    auto stepNames = [](const Evolution::Module& container) {
        std::vector<std::string> names;
        for(const auto& step: container.getSteps()) {
            names.push_back(step->name());
        }
        return names;
    };
    EXPECT_EQ(stepNames(*root->getInitialization()),
        (std::vector<std::string>{"CreateArchive", "InitPopulation", "InitialEvaluation", "TrainingFitness", "ProtectAggregated", "UpdateArchive"}));
    EXPECT_EQ(stepNames(*root),
        (std::vector<std::string>{"ParentSelection", "Reproduction", "Mutation", "TrainingEvaluation", "TrainingFitness", "SurvivorSelection",
                                  "ProtectAggregated", "UpdateArchive"}));
    root->resume(state, 3);

    // The representation must be a TPG, and the members are required.
    auto policyRep = std::make_shared<Representations::LGP>(le->getInputDimensions(), Data::DataType::type<double>({8}), set, Data::DataType::type<double>({3}), 10, 20);
    policyRep->addOutputFunction(std::make_unique<Dimensions::ActivationFunctions::ArgMax<double>>(policyRep->getDimensionFlow().getOutputDimension()));
    Evolution::OriginTPGEvolution notTPG = tpg;
    notTPG.representation = policyRep;
    EXPECT_THROW(notTPG.build(), std::runtime_error) << "The population is not made of TPG";
    Evolution::OriginTPGEvolution noMembers = tpg;
    noMembers.memberRepresentation = nullptr;
    EXPECT_THROW(noMembers.build(), std::runtime_error) << "The members are required";
}
