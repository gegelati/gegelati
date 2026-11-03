#include <gtest/gtest.h>
#include <algorithm>
#include <filesystem>
#include <memory>
#include <sstream>
#include <vector>

#include "instructions/lambdaInstruction.h"
#include "instructions/set.h"
#include "learn/stickGameWithOpponent.h"

#include "evaluation/reinforcementProblem.h"
#include "evolution/basicEvolution.h"
#include "learn/populationNames.h"
#include "evolution/steps/exportStep.h"
#include "evolution/steps/logStep.h"
#include "evolution/steps/assignFitnessStep.h"
#include "evolution/steps/createGeneratorStep.h"
#include "evolution/steps/evaluateStep.h"
#include "evolution/steps/initPopulationStep.h"
#include "evolution/steps/lambdaStep.h"
#include "evolution/steps/mutateStep.h"
#include "evolution/steps/reproduceStep.h"
#include "evolution/steps/selectStep.h"
#include "fitnessAssignment/defaultAssigner.h"
#include "metrics/rewardsMetric.h"

#include "mutation/pointMutator.h"
#include "representations/LGP.h"
#include "reproduction/replicator.h"
#include "selection/randomSelector.h"
#include "selection/truncationSelector.h"
#include "util/counterReset.h"

class ModuleTest : public ::testing::Test
{
  protected:
    Instructions::Set set;
    std::vector<std::unique_ptr<Instructions::Instruction>> instructions;
    std::shared_ptr<Evaluation::ReinforcementEnvironment> le;
    std::shared_ptr<Representations::LGP> lgpRep;
    std::shared_ptr<Evaluation::ReinforcementProblem> problem;

    // Default names of the data of the basic evolution.
    const Evolution::BasicEvolution defaults;

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
        lgpRep = std::make_shared<Representations::LGP>(le->getInputDimensions(), Data::DataType::type<double>({8}), set, Data::DataType::type<double>({3}), 10, 20);
        lgpRep->addOutputFunction(std::make_unique<Dimensions::ActivationFunctions::ArgMax<double>>(lgpRep->getDimensionFlow().getOutputDimension()));
        problem = std::make_shared<Evaluation::ReinforcementProblem>(le, 3);
    }

    // Settings of the basic evolution, on the representation and the problem of the fixture.
    Evolution::BasicEvolution makeEvolution(size_t sizePopulation, size_t nbOffspring, size_t nbTrainingIterations,
                                            size_t nbValidationIterations = 0, size_t nbThreads = 1, size_t validationPeriod = 1)
    {
        Evolution::BasicEvolution evolution;
        evolution.representation = lgpRep;
        evolution.problem = problem;
        evolution.sizePopulation = sizePopulation;
        evolution.nbOffspring = nbOffspring;
        evolution.nbTrainingIterations = nbTrainingIterations;
        evolution.nbValidationIterations = nbValidationIterations;
        evolution.validationPeriod = validationPeriod;
        evolution.nbThreads = nbThreads;
        evolution.logOutput = nullptr;
        return evolution;
    }

    // Steps of the basic evolution, see makeEvolution.
    std::shared_ptr<Evolution::Module> makeSteps(size_t sizePopulation, size_t nbOffspring, size_t nbTrainingIterations, size_t nbValidationIterations = 0,
                                               size_t nbThreads = 1, size_t validationPeriod = 1)
    {
        return makeEvolution(sizePopulation, nbOffspring, nbTrainingIterations, nbValidationIterations, nbThreads, validationPeriod).build();
    }

    // Key of the generator of the genotypes in the tests of the steps.
    const Evolution::Key<GraphBased::GenotypeGenerator> generatorKey{"Generator"};

    // Store the generator of the representation in a state, as the first step of a preset does.
    void setGenerator(Evolution::EvolutionData& state) const
    {
        state.set(generatorKey, std::make_shared<GraphBased::GenotypeGenerator>(lgpRep->getGenotypeConstraint()));
    }

    // Fill a state with the random individuals of a population and its offspring, ready for the first generation.
    void initPopulation(Evolution::EvolutionData& state, const Evolution::BasicEvolution::Keys& population, size_t nbOffspring) const
    {
        state.reset(0);
        setGenerator(state);
        Evolution::InitPopulationStep(population.individuals, lgpRep, 10, std::make_shared<Mutation::PointMutator>(0.2, 0.5, 0.5), generatorKey).apply(state);
        state.generation = 0;
        Evolution::ReproduceStep(population.individuals, std::make_shared<Reproduction::Replicator>(), nbOffspring, population.offspring).apply(state);
    }

    // Evaluate the individuals of a population and compute their fitness, see initPopulation.
    void evaluatePopulation(Evolution::EvolutionData& state, const Evolution::BasicEvolution::Keys& population) const
    {
        auto metricTemplate = std::make_shared<Metrics::MetricMapTemplate>(std::make_unique<Metrics::RewardsMetric>());
        Evolution::EvaluateStep({population.individuals}, population.trainingMetrics, problem, std::make_shared<Evaluation::Evaluator>(), metricTemplate, 2)
            .apply(state);
        Evolution::AssignFitnessStep(population.trainingMetrics, population.trainingFitness, std::make_shared<FitnessAssignment::DefaultAssigner>(),
                                     Metrics::RewardsMetric::staticHash())
            .apply(state);
    }
};

TEST_F(ModuleTest, classicFlow)
{
    auto root = makeSteps(100, 100, 3, 0, 10);
    Evolution::EvolutionData state(4);
    root->resume(state, 20);

    const Evolution::BasicEvolution::Keys population;
    EXPECT_EQ(state.nbGenerationsDone, 20U);
    const std::vector<std::shared_ptr<const Individual>> individuals = state.getIndividuals(population.individuals);
    EXPECT_EQ(individuals.size(), 100U);
    EXPECT_EQ(state.get(population.parents)->size(), 100U);
    EXPECT_EQ(state.get(population.offspring)->size(), 100U);

    // The fitness and metrics are the ones of the last evaluation, of the population and of the offspring.
    const std::map<uint64_t, double>& fitness = *state.get(population.trainingFitness);
    EXPECT_EQ(fitness.size(), 200U);
    EXPECT_EQ((state.get(population.trainingMetrics)->size()), 200U);

    // The best individual is in the population, with the highest fitness of the population.
    std::shared_ptr<const Individual> best = state.bestIndividual(population.individuals, population.trainingFitness);
    ASSERT_NE(best, nullptr);
    EXPECT_NE(std::find(individuals.begin(), individuals.end(), best), individuals.end());
    for(const std::shared_ptr<const Individual>& individual: individuals) {
        EXPECT_LE(fitness.at(individual->getIndividualID()), fitness.at(best->getIndividualID()));
    }

    ASSERT_EQ(1704165046166301501U, state.rng.uniformSample<uint64_t>(0, UINT64_MAX)) << "determinism";
}

TEST_F(ModuleTest, summary)
{
    auto root = makeSteps(10, 10, 3, 2);
    Evolution::EvolutionData state;
    const std::string summary = root->summary();
    for(const std::string& text: {"InitPopulation", "InitialEvaluation", "ParentSelection (10 from Population to Parents)",
                                  "Reproduction", "Mutation", "TrainingEvaluation (training, 3 iterations)",
                                  "SurvivorSelection (10 distinct from Population, Offspring to Population)",
                                  "ValidationEvaluation (validation, 2 iterations)", "ValidationFitness (validationMetrics to validationFitness)"}) {
        EXPECT_NE(summary.find(text), std::string::npos) << text << "\n" << summary;
    }
}

TEST_F(ModuleTest, settingsAreChecked)
{
    Evolution::BasicEvolution evolution;
    EXPECT_THROW(evolution.build(), std::runtime_error) << "No representation nor problem";
    evolution.representation = lgpRep;
    evolution.problem = problem;
    EXPECT_THROW(evolution.build(), std::runtime_error) << "No population size";
    evolution.sizePopulation = 10;
    evolution.nbOffspring = 10;
    EXPECT_NO_THROW(evolution.build());
    evolution.nbValidationIterations = 2;
    evolution.validationPeriod = 0;
    EXPECT_THROW(evolution.build(), std::runtime_error) << "The validation period cannot be zero";
}

TEST_F(ModuleTest, insertSecondMutation)
{
    auto root = makeSteps(20, 20, 2);
    Evolution::EvolutionData state(4);
    const Evolution::BasicEvolution::Keys population;

    // A second mutation after the first one, then a custom step counting the generations.
    size_t nbCalls = 0;
    auto secondMutation = std::make_shared<Evolution::MutateStep>(population.offspring, std::make_shared<Mutation::PointMutator>(0.5, 0.5, 0.5),
                                                                  population.generator, 1, "SecondMutation");
    (*root)
        .insertAfter(root->find("Mutation"), secondMutation)
        .insertAfter(secondMutation, std::make_shared<Evolution::LambdaStep>("Count", [&nbCalls](Evolution::EvolutionData&) { nbCalls++; }));
    ASSERT_EQ(root->size(), 8) << "The preset is one step holding the steps of a generation";

    root->resume(state, 5);
    EXPECT_EQ(nbCalls, 5U);
    for(const auto& individual: state.getIndividuals(population.individuals)) {
        EXPECT_TRUE(individual->isValid());
    }

    // Each step knows how long it took, and a step holding steps lasts at least as long as them.
    EXPECT_GT(secondMutation->getLastDuration(), 0.0);
    EXPECT_GT(root->find("Mutation")->getLastDuration(), 0.0);
    EXPECT_EQ(Evolution::LambdaStep("Unused", [](Evolution::EvolutionData&) {}).getLastDuration(), 0.0) << "Zero until the step ran";
}

TEST_F(ModuleTest, editPipeline)
{
    auto root = makeSteps(10, 10, 2);
    Evolution::EvolutionData state;
    Evolution::Module& steps = *root;
    auto unknown = std::make_shared<Evolution::LambdaStep>("Unknown", [](Evolution::EvolutionData&) {});
    const std::shared_ptr<Evolution::MutateStep> mutation = steps.find<Evolution::MutateStep>("Mutation");

    EXPECT_TRUE(steps.has("Mutation"));
    EXPECT_FALSE(steps.has("Unknown"));
    EXPECT_FALSE(steps.has("InitPopulation")) << "Only in the initialization";
    EXPECT_THROW(steps.find("Unknown"), std::runtime_error);
    EXPECT_THROW(steps.find<Evolution::SelectStep>("Mutation"), std::runtime_error) << "Not a selection";
    EXPECT_THROW(steps.insertAfter(unknown, std::make_shared<Evolution::LambdaStep>("Step", [](Evolution::EvolutionData&) {})), std::runtime_error);
    EXPECT_THROW(steps.insertBefore(mutation, nullptr), std::runtime_error);

    // Steps are found by address, so names can be shared.
    auto before = std::make_shared<Evolution::LambdaStep>("Mutation", [](Evolution::EvolutionData&) {});
    steps.insertBefore(mutation, before);
    EXPECT_EQ(steps.getSteps().at(2), before);
    EXPECT_EQ(steps.getSteps().at(3), mutation);
    auto replaced = std::make_shared<Evolution::LambdaStep>("Replaced", [](Evolution::EvolutionData&) {});
    steps.replace(before, replaced);
    EXPECT_EQ(steps.getSteps().at(2), replaced);
    steps.remove(replaced);
    EXPECT_EQ(steps.getSteps().at(2), mutation);
    EXPECT_THROW(steps.remove(replaced), std::runtime_error);
}

TEST_F(ModuleTest, misorderedPipelineIsRejected)
{
    auto root = makeSteps(10, 10, 2);
    Evolution::EvolutionData state;
    root->remove(root->find("ParentSelection"));

    // The reproduction now has no parents.
    EXPECT_THROW(root->resume(state, 1), std::runtime_error);
}

TEST_F(ModuleTest, evolveContinues)
{
    // Evolving again continues the same evolution, as long as the state is not reset.
    auto reference = makeSteps(20, 20, 2);
    Evolution::EvolutionData referenceState(4);
    reference->resume(referenceState, 5);

    auto continued = makeSteps(20, 20, 2);
    Evolution::EvolutionData state(4);
    continued->resume(state, 3);
    continued->resume(state, 2);

    EXPECT_EQ(state.nbGenerationsDone, 5U);
    EXPECT_EQ(referenceState.rng.uniformSample<uint64_t>(0, UINT64_MAX), state.rng.uniformSample<uint64_t>(0, UINT64_MAX));

    // A reset starts again from the initialization.
    state.reset(4);
    continued->resume(state, 5);
    EXPECT_EQ(state.nbGenerationsDone, 5U);
    EXPECT_EQ(state.getIndividuals(Evolution::BasicEvolution::Keys().individuals).size(), 20U);
}

TEST_F(ModuleTest, customStopIsALoop)
{
    // Stopping is the business of the caller: it evolves one generation at a time.
    auto root = makeSteps(10, 10, 2);
    Evolution::EvolutionData state;
    while(state.nbGenerationsDone == 0 || state.generation < 2) {
        root->resume(state, 1);
    }
    EXPECT_EQ(state.nbGenerationsDone, 3U);
}

TEST_F(ModuleTest, validationAndLogger)
{
    Evolution::BasicEvolution evolution = makeEvolution(10, 10, 2, 2, 1, 2);
    auto root = evolution.build();
    Evolution::EvolutionData state(4);
    const Evolution::BasicEvolution::Keys population;
    const auto validationFitness = root->find<Evolution::AssignFitnessStep>("ValidationFitness")->getFitness();
    EXPECT_EQ(validationFitness, (population.validationFitness));

    std::ostringstream output;
    root->add(std::make_shared<Evolution::LogStep>(output, evolution.logColumns()));
    root->resume(state, 4);

    const std::string log = output.str();
    EXPECT_NE(log.find("Val_Avg"), std::string::npos);

    // Validation ran on generations 0 and 2 only.
    EXPECT_FALSE((state.get(validationFitness)->empty()));
    EXPECT_FALSE(state.isUpToDate(validationFitness));
    EXPECT_TRUE(state.isUpToDate(population.trainingFitness));

    // Rows of generations 1 and 3 show "-" instead of the older validation values.
    std::istringstream lines(log);
    size_t nbRowsWithoutValidation = 0;
    for(std::string line; std::getline(lines, line);) {
        nbRowsWithoutValidation += (line.find(" -") != std::string::npos) ? 1 : 0;
    }
    EXPECT_EQ(nbRowsWithoutValidation, 2U);
    EXPECT_NE(root->summary().find("(validation, 2 iterations, every 2 generations)"), std::string::npos) << root->summary();
    EXPECT_THROW(root->find("ValidationEvaluation")->setPeriod(0), std::runtime_error);
}

TEST_F(ModuleTest, validationWithItsOwnProblemAndAssigner)
{
    auto validationProblem = std::make_shared<Evaluation::ReinforcementProblem>(le, 5);
    Evolution::BasicEvolution evolution = makeEvolution(10, 10, 2, 3);
    evolution.validationProblem = validationProblem;
    evolution.validationAssigner = std::make_shared<FitnessAssignment::DefaultAssigner>();
    auto root = evolution.build();
    Evolution::EvolutionData state;
    const Evolution::BasicEvolution::Keys population;

    const std::shared_ptr<Evolution::EvaluateStep> validation = root->find<Evolution::EvaluateStep>("ValidationEvaluation");
    EXPECT_EQ(validation->getProblem(), validationProblem);
    EXPECT_EQ(validation->getMode(), Evaluation::Mode::VALIDATION);
    EXPECT_EQ(root->find<Evolution::EvaluateStep>("TrainingEvaluation")->getProblem().get(), problem.get());
    root->resume(state, 2);

    EXPECT_EQ((state.get(population.validationFitness)->size()), 10U);
    EXPECT_NE(state.bestIndividual(population.individuals, population.validationFitness), nullptr);
}

TEST_F(ModuleTest, parentsWithTheirOwnFitness)
{
    Evolution::BasicEvolution evolution = makeEvolution(10, 10, 2);
    evolution.parentAssigner = std::make_shared<FitnessAssignment::DefaultAssigner>();
    auto root = evolution.build();
    Evolution::EvolutionData state;
    const Evolution::BasicEvolution::Keys population;

    EXPECT_EQ(root->find("ParentSelection")->requiredData().count(population.parentFitness.name()), 1U);
    EXPECT_TRUE(root->getInitialization()->has("ParentFitness")) << "Parents of the first generation need it";
    EXPECT_NE(root->summary().find("ParentFitness (trainingMetrics to parentFitness)"), std::string::npos);

    root->resume(state, 3);
    EXPECT_TRUE(state.isUpToDate(population.parentFitness));

    // Without the parent fitness, the parents cannot be selected.
    root->getInitialization()->remove(root->getInitialization()->find("ParentFitness"));
    root->remove(root->find("ParentFitness"));
    state.reset(0);
    EXPECT_THROW(root->resume(state, 1), std::runtime_error);
}

TEST_F(ModuleTest, selectionIsGeneric)
{
    Evolution::EvolutionData state;
    auto a = std::make_shared<Individual>(lgpRep);
    auto b = std::make_shared<Individual>(lgpRep);
    auto c = std::make_shared<Individual>(lgpRep);
    auto other = std::make_shared<Individual>(lgpRep);
    const Evolution::Key<std::vector<std::shared_ptr<Individual>>> listA("A");
    const Evolution::Key<std::vector<std::shared_ptr<Individual>>> listB("B");
    const Evolution::Key<std::map<uint64_t, double>> fitness("Fitness");
    const Evolution::Key<std::vector<std::shared_ptr<Individual>>> selected("Selected");
    state.set(listA, std::make_shared<std::vector<std::shared_ptr<Individual>>>(std::vector<std::shared_ptr<Individual>>{c, a}));
    state.set(listB, std::make_shared<std::vector<std::shared_ptr<Individual>>>(std::vector<std::shared_ptr<Individual>>{b}));
    state.set(fitness, std::make_shared<std::map<uint64_t, double>>(std::map<uint64_t, double>{
        {a->getIndividualID(), 3.0}, {b->getIndividualID(), 2.0}, {c->getIndividualID(), 1.0}, {other->getIndividualID(), 10.0}}));

    // The best candidates are kept once each, ordered by ID: the fitness of non candidates is ignored.
    Evolution::SelectStep distinct({listA, listB}, fitness, std::make_shared<Selection::TruncationSelector>(), 2, selected, true, "Best");
    EXPECT_EQ(distinct.name(), "Best");
    EXPECT_EQ(distinct.requiredData(), (std::set<std::string>{listA.name(), listB.name(), fitness.name()}));
    distinct.apply(state);
    EXPECT_EQ(state.getIndividuals(selected), (std::vector<std::shared_ptr<const Individual>>{a, b}));

    // A selection can replace one of its candidate lists, and keep duplicates.
    Evolution::SelectStep withDuplicates({listA}, fitness, std::make_shared<Selection::RandomSelector>(true), 5, listA);
    EXPECT_EQ(withDuplicates.name(), "Select");
    withDuplicates.apply(state);
    EXPECT_EQ(state.get(listA)->size(), 5U);
    EXPECT_THROW(Evolution::SelectStep({}, fitness, std::make_shared<Selection::TruncationSelector>(), 2, selected), std::runtime_error);
}

TEST_F(ModuleTest, loggerColumns)
{
    Evolution::BasicEvolution evolution = makeEvolution(10, 10, 2);
    auto root = evolution.build();
    Evolution::EvolutionData state;
    const Evolution::BasicEvolution::Keys population;
    const auto offspring = population.offspring;

    // The columns of the preset, and a custom one.
    std::vector<Evolution::LogStep::Column> columns = evolution.logColumns();
    columns.push_back({"Nb_Offspring", [offspring](const Evolution::EvolutionData& data) {
        const auto children = data.getIfAny(offspring);
        return (children == nullptr) ? 0.0 : static_cast<double>(children->size());
    }, 0});
    std::ostringstream output;
    root->add(std::make_shared<Evolution::LogStep>(output, columns));
    root->resume(state, 2);
    state.reset(0);
    root->resume(state, 1);

    // The training fitness without the validation, the custom column, and one header per evolution.
    const std::string log = output.str();
    EXPECT_NE(log.find("Fit_Avg"), std::string::npos);
    EXPECT_NE(log.find("Pop_Size"), std::string::npos);
    EXPECT_EQ(log.find("Val_Avg"), std::string::npos);
    size_t nbHeaders = 0;
    for(size_t pos = log.find("Nb_Offspring"); pos != std::string::npos; pos = log.find("Nb_Offspring", pos + 1)) {
        nbHeaders++;
    }
    EXPECT_EQ(nbHeaders, 2U);
}

TEST_F(ModuleTest, loggerSeparator)
{
    Evolution::BasicEvolution evolution = makeEvolution(10, 10, 2);
    auto root = evolution.build();

    // One log aligned for the console, and one written for another tool: the columns of the preset and the time of each step, separated by ";".
    std::vector<Evolution::LogStep::Column> csvColumns = evolution.logColumns();
    const std::vector<Evolution::LogStep::Column> times = Evolution::stepTimeColumns(*root);
    csvColumns.insert(csvColumns.end(), times.begin(), times.end());
    std::ostringstream table;
    std::ostringstream csv;
    auto tableLog = std::make_shared<Evolution::LogStep>(table, evolution.logColumns());
    tableLog->setColumnWidth(10);
    root->add(tableLog);
    root->add(std::make_shared<Evolution::LogStep>(csv, csvColumns, ";", "CsvLog"));
    root->evolve(3);

    // The table is aligned in columns of 10 characters.
    std::istringstream tableLines(table.str());
    std::string line;
    std::getline(tableLines, line);
    EXPECT_EQ(line.find(';'), std::string::npos);
    EXPECT_EQ(line.size() % 10, 0U) << line;

    // The csv has no padding, the same number of fields on each line, and the steps heading.
    std::istringstream csvLines(csv.str());
    std::getline(csvLines, line);
    EXPECT_EQ(line.substr(0, 12), "Gen;Pop_Size") << line;
    EXPECT_NE(line.find(";Mutation;"), std::string::npos) << line;
    EXPECT_EQ(line.substr(line.size() - 14), ";T_Gen;T_Total") << line;
    EXPECT_EQ(line.find("  "), std::string::npos);
    const auto nbSeparators = std::count(line.begin(), line.end(), ';');
    size_t nbRows = 0;
    while(std::getline(csvLines, line)) {
        EXPECT_EQ(std::count(line.begin(), line.end(), ';'), nbSeparators) << line;
        nbRows++;
    }
    EXPECT_EQ(nbRows, 3U);
}

TEST_F(ModuleTest, stepsOfThePreset)
{
    auto root = makeSteps(10, 10, 3);
    Evolution::EvolutionData state;
    const Evolution::BasicEvolution::Keys population;
    const auto evaluation = root->find<Evolution::EvaluateStep>("TrainingEvaluation");
    EXPECT_EQ(evaluation->getNbIterations(), 3U);
    EXPECT_EQ(evaluation->getEvaluated(), (std::vector<Evolution::Key<std::vector<std::shared_ptr<Individual>>>>{population.individuals, population.offspring}));
    EXPECT_EQ(root->getInitialization()->find<Evolution::EvaluateStep>("InitialEvaluation")->getEvaluated(), (std::vector<Evolution::Key<std::vector<std::shared_ptr<Individual>>>>{population.individuals}));
    EXPECT_EQ(evaluation->getMetrics(), (population.trainingMetrics));
    EXPECT_TRUE(root->getInitialization()->has("TrainingFitness"));
    EXPECT_TRUE(root->has("TrainingFitness"));
    EXPECT_FALSE(root->has("ValidationEvaluation"));
}

TEST_F(ModuleTest, parentsComeFromPopulation)
{
    auto root = makeSteps(20, 20, 2);
    Evolution::EvolutionData state(4);
    const auto individuals = Evolution::BasicEvolution::Keys().individuals;
    const auto parents = Evolution::BasicEvolution::Keys().parents;

    // Individuals discarded by the survival must not be selected as parents anymore.
    size_t nbParentsChecked = 0;
    root->insertAfter(root->find("ParentSelection"), std::make_shared<Evolution::LambdaStep>("CheckParents",
        [&nbParentsChecked, individuals, parents](Evolution::EvolutionData& state) {
            const std::vector<std::shared_ptr<const Individual>> current = state.getIndividuals(individuals);
            for(const auto& parent: *state.get(parents)) {
                EXPECT_NE(std::find(current.begin(), current.end(), parent), current.end());
                nbParentsChecked++;
            }
        }));
    root->resume(state, 10);
    EXPECT_EQ(nbParentsChecked, 200U);
}

namespace {
    // Custom name of the tests.
    const Evolution::Key<int> Counter{"Counter"};
}

TEST_F(ModuleTest, stateEntries)
{
    Evolution::EvolutionData state;
    EXPECT_FALSE(state.has(Counter));
    EXPECT_THROW(state.get(Counter), std::runtime_error);

    state.set(Counter, std::make_shared<int>(3));
    ASSERT_TRUE(state.has(Counter));
    EXPECT_EQ(*state.get(Counter), 3);
    EXPECT_EQ(*state.get(Evolution::Key<int>("Counter")), 3) << "Keys are identified by their name";
    EXPECT_THROW(state.get(Evolution::Key<double>("Counter")), std::runtime_error) << "The entry is not a double";
    EXPECT_EQ(state.getIfAny(Evolution::Key<int>("Other")), nullptr);

    const Evolution::EvolutionData& constState = state;
    EXPECT_EQ(*constState.get(Counter), 3);

    // Entries record the generation in which they are stored, not when they are modified.
    state.generation = 4;
    EXPECT_FALSE(state.isUpToDate(Counter));
    (*state.get(Counter))++;
    EXPECT_FALSE(state.isUpToDate(Counter));
    state.set(Counter, std::make_shared<int>(5));
    EXPECT_TRUE(state.isUpToDate(Counter));

    state.erase(Counter);
    EXPECT_FALSE(state.has(Counter));
    EXPECT_FALSE(state.isUpToDate(Counter));

    // Individuals are read from lists, an absent list is empty, and several lists are concatenated.
    auto individual = std::make_shared<Individual>(lgpRep);
    auto other = std::make_shared<Individual>(lgpRep);
    const Evolution::Key<std::vector<std::shared_ptr<Individual>>> first("First");
    const Evolution::Key<std::vector<std::shared_ptr<Individual>>> second("Second");
    state.set(first, std::make_shared<std::vector<std::shared_ptr<Individual>>>(std::vector<std::shared_ptr<Individual>>{individual}));
    state.set(second, std::make_shared<std::vector<std::shared_ptr<Individual>>>(std::vector<std::shared_ptr<Individual>>{other}));
    EXPECT_EQ(state.getIndividuals(first), (std::vector<std::shared_ptr<const Individual>>{individual}));
    EXPECT_EQ(state.getIndividuals({first, second}), (std::vector<std::shared_ptr<const Individual>>{individual, other}));
    EXPECT_TRUE(state.getIndividuals(Evolution::Key<std::vector<std::shared_ptr<Individual>>>("Absent")).empty());
    state.set(Counter, std::make_shared<int>(4));
    EXPECT_THROW(state.getIndividuals(Evolution::Key<std::vector<std::shared_ptr<Individual>>>(Counter.name())), std::runtime_error) << "The counter is not a list of individuals";
    EXPECT_THROW(state.get(Evolution::Key<double>(Counter.name())), std::runtime_error) << "A key of the wrong type is refused at runtime too, when built from a name";
    EXPECT_EQ(*state.get(Evolution::Key<int>(Counter.name())), 4) << "Keys are names with a type";

    state.reset(0);
    EXPECT_FALSE(state.has(Counter)) << "A new evolution starts without entries";
}

TEST_F(ModuleTest, customDataAreChecked)
{
    auto root = makeSteps(10, 10, 2);
    Evolution::EvolutionData state;
    root->add(std::make_shared<Evolution::LambdaStep>("Increment",
        [](Evolution::EvolutionData& state) { (*state.get(Counter))++; }, std::set<std::string>{Counter.name()}, std::set<std::string>{Counter.name()}));
    EXPECT_THROW(root->resume(state, 1), std::runtime_error) << "No step creates the counter";

    root->getInitialization()->add(std::make_shared<Evolution::LambdaStep>("CreateCounter",
        [](Evolution::EvolutionData& state) { state.set(Counter, std::make_shared<int>(0)); }, std::set<std::string>{}, std::set<std::string>{Counter.name()}));
    root->resume(state, 3);
    EXPECT_EQ(*state.get(Counter), 3);
    EXPECT_NE(root->summary().find("provides: Counter"), std::string::npos);
}

TEST_F(ModuleTest, mutationComponentsAreRequired)
{
    const Evolution::BasicEvolution::Keys population(namesOf("P"));
    const auto offspring = population.offspring;
    auto mutator = std::make_shared<Mutation::PointMutator>(0.5, 0.5, 0.5);

    // Components are fixed objects, only the mutated individuals and the generator are data of the state.
    Evolution::MutateStep mutate(offspring, mutator, generatorKey);
    EXPECT_EQ(mutate.requiredData(), (std::set<std::string>{offspring.name(), generatorKey.name()}));
    EXPECT_THROW(Evolution::MutateStep(offspring, nullptr, generatorKey), std::runtime_error);

    // The population is created with the generator found in the data.
    Evolution::InitPopulationStep init(population.individuals, lgpRep, 10, mutator, generatorKey);
    EXPECT_EQ(init.requiredData(), (std::set<std::string>{generatorKey.name()}));
    EXPECT_EQ(init.providedData(), (std::set<std::string>{population.individuals.name()}));
    EXPECT_THROW(Evolution::InitPopulationStep(population.individuals, lgpRep, 10, nullptr, generatorKey), std::runtime_error);
    EXPECT_THROW(Evolution::InitPopulationStep(population.individuals, nullptr, 10, mutator, generatorKey), std::runtime_error);

    Evolution::EvolutionData state;
    state.reset(0);
    EXPECT_THROW(init.apply(state), std::runtime_error) << "No generator in the data";
    setGenerator(state);
    ASSERT_NO_THROW(init.apply(state));
    EXPECT_EQ(state.getIndividuals(population.individuals).size(), 10U);
}

TEST_F(ModuleTest, twoPopulations)
{
    // Steps built from scratch, evolving two populations with the same kind of steps.
    auto root = std::make_shared<Evolution::Module>("Evolution");
    auto init = std::make_shared<Evolution::Module>("Initialization");
    root->setInitialization(init);
    Evolution::EvolutionData state(4);
    auto mutator = std::make_shared<Mutation::PointMutator>(0.2, 0.5, 0.5);
    auto metricTemplate = std::make_shared<Metrics::MetricMapTemplate>(std::make_unique<Metrics::RewardsMetric>());
    auto addClassicSteps = [&](const Evolution::BasicEvolution::Keys& population, size_t size, size_t nbOffspring) {
        const auto individuals = population.individuals;
        const auto parents = population.parents;
        const auto offspring = population.offspring;
        const auto metrics = population.trainingMetrics;
        const auto fitness = population.trainingFitness;
        auto assign = std::make_shared<Evolution::AssignFitnessStep>(metrics, fitness, std::make_shared<FitnessAssignment::DefaultAssigner>(),
                                                                      Metrics::RewardsMetric::staticHash());

        init->add(std::make_shared<Evolution::CreateGeneratorStep>(population.generator, lgpRep));
        init->add(std::make_shared<Evolution::InitPopulationStep>(individuals, lgpRep, size, mutator, population.generator));
        init->add(std::make_shared<Evolution::EvaluateStep>(std::vector<Evolution::Key<std::vector<std::shared_ptr<Individual>>>>{individuals}, metrics, problem,
                                                                     std::make_shared<Evaluation::Evaluator>(), metricTemplate, 2));
        init->add(assign);

        root->add(std::make_shared<Evolution::SelectStep>(std::vector<Evolution::Key<std::vector<std::shared_ptr<Individual>>>>{individuals}, fitness,
                                                               std::make_shared<Selection::RandomSelector>(true), nbOffspring, parents));
        root->add(std::make_shared<Evolution::ReproduceStep>(parents, std::make_shared<Reproduction::Replicator>(), nbOffspring, offspring));
        root->add(std::make_shared<Evolution::MutateStep>(offspring, mutator, population.generator));
        root->add(std::make_shared<Evolution::EvaluateStep>(std::vector<Evolution::Key<std::vector<std::shared_ptr<Individual>>>>{individuals, offspring}, metrics, problem,
                                                                 std::make_shared<Evaluation::Evaluator>(), metricTemplate, 2));
        root->add(assign);
        root->add(std::make_shared<Evolution::SelectStep>(std::vector<Evolution::Key<std::vector<std::shared_ptr<Individual>>>>{individuals, offspring}, fitness,
                                                               std::make_shared<Selection::TruncationSelector>(), size, individuals, true));
        return assign;
    };

    const Evolution::BasicEvolution::Keys populationA(namesOf("A"));
    const Evolution::BasicEvolution::Keys populationB(namesOf("B"));
    addClassicSteps(populationA, 10, 10);
    auto assignB = addClassicSteps(populationB, 6, 4);

    const std::string summary = root->summary();
    for(const std::string& text: {"A.trainingFitness", "B.trainingFitness", "B.parents"}) {
        EXPECT_NE(summary.find(text), std::string::npos) << text;
    }
    root->resume(state, 3);

    const std::vector<std::shared_ptr<const Individual>> individualsA = state.getIndividuals(populationA.individuals);
    const std::vector<std::shared_ptr<const Individual>> individualsB = state.getIndividuals(populationB.individuals);
    EXPECT_EQ(individualsA.size(), 10U);
    EXPECT_EQ(individualsB.size(), 6U);
    for(const auto& individual: individualsB) {
        EXPECT_EQ(std::find(individualsA.begin(), individualsA.end(), individual), individualsA.end()) << "The populations are separate";
    }
    EXPECT_NE(state.bestIndividual(populationB.individuals, populationB.trainingFitness), nullptr);

    // Steps of the second population are checked like the others.
    init->remove(assignB);
    root->remove(assignB);
    state.reset(4);
    EXPECT_THROW(root->resume(state, 1), std::runtime_error) << "No fitness for the second population";
}

TEST_F(ModuleTest, presetsShareARoot)
{
    // Two basic evolutions in the same root step, with different population names.
    auto root = std::make_shared<Evolution::Module>("Evolution");
    Evolution::EvolutionData state(4);
    Evolution::BasicEvolution first = makeEvolution(10, 10, 2);
    first.names = namesOf("First");
    root->add(first.build());
    Evolution::BasicEvolution second = makeEvolution(6, 6, 2);
    second.names = namesOf("Second");
    root->add(second.build());

    root->resume(state, 2);
    EXPECT_EQ(state.getIndividuals(Evolution::Key<std::vector<std::shared_ptr<Individual>>>("First")).size(), 10U);
    EXPECT_EQ(state.getIndividuals(Evolution::Key<std::vector<std::shared_ptr<Individual>>>("Second")).size(), 6U);
}

TEST_F(ModuleTest, evolutionAddedLater)
{
    // A second population joins after a few generations of the first one, without initializing the first one again.
    auto root = std::make_shared<Evolution::Module>("Evolution");
    Evolution::EvolutionData state(4);
    Evolution::BasicEvolution first = makeEvolution(10, 10, 2);
    first.names = namesOf("First");
    const std::shared_ptr<Evolution::Module> firstSteps = first.build();
    size_t nbInitializations = 0;
    firstSteps->getInitialization()->add(std::make_shared<Evolution::LambdaStep>("CountInitializations", [&nbInitializations](Evolution::EvolutionData&) { nbInitializations++; }));
    root->add(firstSteps);

    root->resume(state, 3);
    const Evolution::Key<std::vector<std::shared_ptr<Individual>>> secondIndividuals("Second");
    EXPECT_FALSE(state.has(secondIndividuals));

    Evolution::BasicEvolution second = makeEvolution(6, 6, 2);
    second.names = namesOf("Second");
    root->add(second.build());
    root->resume(state, 2);

    EXPECT_EQ(nbInitializations, 1U) << "The first population is initialized once";
    EXPECT_EQ(state.nbGenerationsDone, 5U);
    EXPECT_EQ(state.getIndividuals(secondIndividuals).size(), 6U);
    EXPECT_EQ(state.getIndividuals(Evolution::BasicEvolution::Keys(namesOf("First")).individuals).size(), 10U);

    // The new steps are checked against the data already in the state: a population needing data nobody makes is refused.
    root->add(std::make_shared<Evolution::LambdaStep>("NeedsCounter", [](Evolution::EvolutionData&) {}, std::set<std::string>{Counter.name()}));
    EXPECT_THROW(root->resume(state, 1), std::runtime_error);
    EXPECT_EQ(state.nbGenerationsDone, 5U) << "Nothing ran";
}

TEST_F(ModuleTest, leafInitialization)
{
    // Any step can have an initialization, which runs once before it first runs, in the initialization generation.
    Evolution::EvolutionData state;
    auto step = std::make_shared<Evolution::LambdaStep>("Increment", [](Evolution::EvolutionData& state) { (*state.get(Counter))++; },
                                                        std::set<std::string>{Counter.name()}, std::set<std::string>{Counter.name()});
    uint64_t generationOfInitialization = 0;
    step->setInitialization(std::make_shared<Evolution::LambdaStep>("Create", [&generationOfInitialization](Evolution::EvolutionData& state) {
        generationOfInitialization = state.generation;
        state.set(Counter, std::make_shared<int>(10));
    }, std::set<std::string>{}, std::set<std::string>{Counter.name()}));
    Evolution::Module root("Root");
    root.add(step);

    root.resume(state, 2);
    root.resume(state, 1);
    EXPECT_EQ(*state.get(Counter), 13);
    EXPECT_EQ(generationOfInitialization, Evolution::EvolutionData::initGeneration);
    EXPECT_NE(root.summary().find("initialization:"), std::string::npos) << root.summary();
    EXPECT_EQ(state.generation, 2U) << "The generation is the one running after the initialization";
}

TEST_F(ModuleTest, stepsOwnTheProblemAndTheRepresentation)
{
    auto ownedProblem = std::make_shared<Evaluation::ReinforcementProblem>(le, 3);
    std::weak_ptr<Evaluation::ReinforcementProblem> weakProblem = ownedProblem;

    auto ownedRep = std::make_shared<Representations::LGP>(le->getInputDimensions(), Data::DataType::type<double>({8}), set, Data::DataType::type<double>({3}), 10, 20);
    ownedRep->addOutputFunction(std::make_unique<Dimensions::ActivationFunctions::ArgMax<double>>(ownedRep->getDimensionFlow().getOutputDimension()));
    std::weak_ptr<Representations::LGP> weakRep = ownedRep;

    std::shared_ptr<Evolution::Module> root = [&]() {
        Evolution::BasicEvolution evolution;
        evolution.representation = std::move(ownedRep);
        evolution.problem = std::move(ownedProblem);
        evolution.sizePopulation = 10;
        evolution.nbOffspring = 10;
        return evolution.build();
    }();
    Evolution::EvolutionData state;
    EXPECT_FALSE(weakProblem.expired()) << "The steps keep the problem alive";
    root->resume(state, 2);

    // The individuals keep their representation alive, even without the steps.
    root.reset();
    EXPECT_TRUE(weakProblem.expired());
    EXPECT_FALSE(weakRep.expired()) << "The individuals keep the representation alive";
    EXPECT_EQ(state.getIndividuals(Evolution::BasicEvolution::Keys().individuals).size(), 10U);

    // Nothing keeps the representation once the individuals are gone.
    state.reset(0);
    EXPECT_TRUE(weakRep.expired());
}

TEST_F(ModuleTest, stepsAreMovable)
{
    Evolution::Module moved = std::move(*makeSteps(20, 20, 2));
    Evolution::EvolutionData state(4);
    moved.resume(state, 5);

    auto reference = makeSteps(20, 20, 2);
    Evolution::EvolutionData referenceState(4);
    reference->resume(referenceState, 5);
    EXPECT_EQ(state.rng.uniformSample<uint64_t>(0, UINT64_MAX), referenceState.rng.uniformSample<uint64_t>(0, UINT64_MAX));
}

TEST_F(ModuleTest, reproduceStep)
{
    const Evolution::BasicEvolution::Keys population(namesOf("P"));
    Evolution::EvolutionData state;
    initPopulation(state, population, 0);
    const auto offspring = population.offspring;

    Evolution::ReproduceStep step(population.individuals, std::make_shared<Reproduction::Replicator>(), 25, offspring);
    EXPECT_EQ(step.name(), "Reproduce");
    EXPECT_EQ(step.requiredData(), (std::set<std::string>{population.individuals.name()}));
    EXPECT_EQ(step.providedData(), (std::set<std::string>{offspring.name()}));
    ASSERT_NO_THROW(step.apply(state));

    // Offspring are new individuals copying the parents, which stay in the population.
    const std::vector<std::shared_ptr<const Individual>> parents = state.getIndividuals(population.individuals);
    const std::vector<std::shared_ptr<Individual>>& children = *state.get(offspring);
    EXPECT_EQ(parents.size(), 10U);
    ASSERT_EQ(children.size(), 25U);
    for(const std::shared_ptr<Individual>& child: children) {
        EXPECT_TRUE(std::any_of(parents.begin(), parents.end(), [&child](const std::shared_ptr<const Individual>& parent) {
            return parent != child && parent->getGenotype() == child->getGenotype();
        }));
    }
    EXPECT_TRUE(state.isUpToDate(offspring));
}

TEST_F(ModuleTest, mutateStep)
{
    const Evolution::BasicEvolution::Keys population(namesOf("P"));
    const auto offspring = population.offspring;
    auto mutator = std::make_shared<Mutation::PointMutator>(0.5, 0.5, 0.5);

    // The same offspring mutated by several threads.
    std::vector<std::vector<std::shared_ptr<Individual>>> mutated;
    for(size_t nbThreads: {2, 3}) {
        Evolution::EvolutionData state;
        initPopulation(state, population, 20);
        Evolution::MutateStep step(offspring, mutator, generatorKey, nbThreads);
        EXPECT_EQ(step.name(), "Mutate");
        EXPECT_EQ(step.getIndividuals(), offspring);
        EXPECT_EQ(step.getNbThreads(), nbThreads);
        EXPECT_EQ(step.requiredData(), (std::set<std::string>{offspring.name(), generatorKey.name()}));
        EXPECT_TRUE(step.providedData().empty()) << "Offspring are mutated in place";
        ASSERT_NO_THROW(step.apply(state));

        // Offspring are not copies of their parents anymore, and remain valid.
        const std::vector<std::shared_ptr<const Individual>> parents = state.getIndividuals(population.individuals);
        const std::vector<std::shared_ptr<Individual>>& children = *state.get(offspring);
        size_t nbChanged = 0;
        for(const std::shared_ptr<Individual>& child: children) {
            EXPECT_TRUE(child->isValid());
            const bool isCopy = std::any_of(parents.begin(), parents.end(), [&child](const std::shared_ptr<const Individual>& parent) {
                return parent->getGenotype() == child->getGenotype();
            });
            nbChanged += isCopy ? 0 : 1;
        }
        EXPECT_GT(nbChanged, 0U);
        mutated.push_back(children);
    }

    // Mutations do not depend on the number of threads.
    ASSERT_EQ(mutated.at(0).size(), mutated.at(1).size());
    for(size_t idx = 0; idx < mutated.at(0).size(); idx++) {
        EXPECT_TRUE(mutated.at(0).at(idx)->getGenotype() == mutated.at(1).at(idx)->getGenotype()) << idx;
    }
}

TEST_F(ModuleTest, evaluateStep)
{
    const Evolution::BasicEvolution::Keys population(namesOf("P"));
    Evolution::EvolutionData state;
    initPopulation(state, population, 5);
    const auto offspring = population.offspring;
    const auto metrics = population.trainingMetrics;
    auto evaluator = std::make_shared<Evaluation::Evaluator>();
    auto metricTemplate = std::make_shared<Metrics::MetricMapTemplate>(std::make_unique<Metrics::RewardsMetric>());

    EXPECT_THROW(Evolution::EvaluateStep({offspring}, metrics, nullptr, evaluator, metricTemplate, 2), std::runtime_error);
    Evolution::EvaluateStep step({population.individuals, offspring}, metrics, problem, evaluator, metricTemplate, 2, Evaluation::Mode::VALIDATION);
    EXPECT_EQ(step.name(), "Evaluate");
    EXPECT_EQ(step.requiredData(), (std::set<std::string>{population.individuals.name(), offspring.name()}));
    EXPECT_EQ(step.providedData(), (std::set<std::string>{metrics.name()}));
    EXPECT_EQ(step.details(), "validation, 2 iterations");
    ASSERT_NO_THROW(step.apply(state));

    // Metrics of the individuals of both lists, constant and mutable, indexed by ID.
    const std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>& results = *state.get(metrics);
    EXPECT_EQ(results.size(), 15U);
    for(const Evolution::Key<std::vector<std::shared_ptr<Individual>>>& list: std::vector<Evolution::Key<std::vector<std::shared_ptr<Individual>>>>{population.individuals, offspring}) {
        for(const std::shared_ptr<const Individual>& individual: state.getIndividuals(list)) {
            ASSERT_EQ(results.count(individual->getIndividualID()), 1U);
            EXPECT_FALSE(results.at(individual->getIndividualID())->getMetricValues<Metrics::RewardsMetric>(Metrics::RewardsMetric::staticHash()).empty());
        }
    }
    EXPECT_TRUE(state.isUpToDate(metrics));
}

TEST_F(ModuleTest, assignFitnessStep)
{
    const Evolution::BasicEvolution::Keys population(namesOf("P"));
    Evolution::EvolutionData state;
    initPopulation(state, population, 0);
    evaluatePopulation(state, population);
    const auto metrics = population.trainingMetrics;
    const auto fitness = population.parentFitness;

    Evolution::AssignFitnessStep step(metrics, fitness, std::make_shared<FitnessAssignment::DefaultAssigner>(), Metrics::RewardsMetric::staticHash());
    EXPECT_EQ(step.name(), "AssignFitness");
    EXPECT_EQ(step.getFitness(), fitness);
    EXPECT_EQ(step.requiredData(), (std::set<std::string>{metrics.name()}));
    EXPECT_EQ(step.providedData(), (std::set<std::string>{fitness.name()}));
    ASSERT_NO_THROW(step.apply(state));

    // One fitness per evaluated individual, and the best individual has the highest one.
    const std::map<uint64_t, double>& values = *state.get(fitness);
    EXPECT_EQ(values.size(), 10U);
    const std::shared_ptr<const Individual> best = state.bestIndividual(population.individuals, fitness);
    ASSERT_NE(best, nullptr);
    for(const auto& [id, value]: values) {
        EXPECT_LE(value, values.at(best->getIndividualID()));
    }
}

TEST_F(ModuleTest, exportStep)
{
    const Evolution::BasicEvolution::Keys population(namesOf("P"));
    Evolution::EvolutionData state;
    initPopulation(state, population, 0);
    evaluatePopulation(state, population);
    const std::string path = (std::filesystem::temp_directory_path() / "gegelati_export_step").string();
    const auto fitness = population.trainingFitness;

    Evolution::ExportStep all(population.individuals, path + "_all", true);
    Evolution::ExportStep best(population.individuals, path + "_best", false, fitness);
    EXPECT_EQ(all.name(), "Export");
    EXPECT_EQ(all.details(), path + "_all<generation>.dot");
    EXPECT_EQ(all.requiredData(), (std::set<std::string>{population.individuals.name()}));
    EXPECT_EQ(best.requiredData(), (std::set<std::string>{population.individuals.name(), fitness.name()}));
    EXPECT_TRUE(all.providedData().empty());

    // Nothing is exported during the initialization.
    std::filesystem::remove(path + "_all3.dot");
    std::filesystem::remove(path + "_best.dot");
    state.generation = Evolution::EvolutionData::initGeneration;
    ASSERT_NO_THROW(all.apply(state));
    EXPECT_FALSE(std::filesystem::exists(path + "_all3.dot"));

    // One file per generation, or one file overwritten with the best individual.
    state.generation = 3;
    ASSERT_NO_THROW(all.apply(state));
    ASSERT_NO_THROW(best.apply(state));
    ASSERT_TRUE(std::filesystem::exists(path + "_all3.dot"));
    ASSERT_TRUE(std::filesystem::exists(path + "_best.dot"));
    EXPECT_LT(std::filesystem::file_size(path + "_best.dot"), std::filesystem::file_size(path + "_all3.dot")) << "Only the best individual is exported";

    std::filesystem::remove(path + "_all3.dot");
    std::filesystem::remove(path + "_best.dot");
}

TEST_F(ModuleTest, nestedSteps)
{
    // A step holds steps: they nest, can be repeated, and are searched and edited from the root.
    Evolution::Module root("Root");
    Evolution::EvolutionData state(1);
    const Evolution::Key<int> counter("Counter");
    auto increment = [counter](Evolution::EvolutionData& state) { (*state.get(counter))++; };
    auto first = std::make_shared<Evolution::LambdaStep>("Increment", increment, std::set<std::string>{counter.name()}, std::set<std::string>{counter.name()});
    auto inner = std::make_shared<Evolution::Module>("Inner");
    inner->add(first);

    root.setInitialization(std::make_shared<Evolution::LambdaStep>("Create", [counter](Evolution::EvolutionData& state) { state.set(counter, std::make_shared<int>(0)); },
                                                                   std::set<std::string>{}, std::set<std::string>{counter.name()}));
    root.add(inner);
    EXPECT_EQ(root.size(), 1U);
    EXPECT_TRUE(root.has("Increment")) << "Steps of nested steps are found";
    EXPECT_EQ(root.find("Increment"), first);

    // Steps of nested steps are edited from the root.
    auto second = std::make_shared<Evolution::LambdaStep>("Second", increment, std::set<std::string>{counter.name()}, std::set<std::string>{counter.name()});
    root.insertAfter(first, second);
    EXPECT_EQ(inner->size(), 2U);

    const std::string summary = root.summary();
    EXPECT_NE(summary.find("initialization: Create"), std::string::npos) << summary;
    EXPECT_NE(summary.find(std::string("`- Inner") + std::string(1, 10) + "   |- Increment"), std::string::npos) << summary;
    EXPECT_NE(summary.find("   `- Second"), std::string::npos) << summary;
    EXPECT_NE(summary.find("requires: Counter | provides: Counter"), std::string::npos) << summary;

    root.resume(state, 2);
    EXPECT_EQ(*state.get(counter), 2 * 2) << "Two generations, two steps";
    EXPECT_GE(inner->getLastDuration(), first->getLastDuration() + second->getLastDuration()) << "The container includes its steps";

    root.remove(first);
    EXPECT_EQ(inner->size(), 1U);
}

TEST_F(ModuleTest, nestedStepsAreChecked)
{
    Evolution::Module root("Root");
    Evolution::EvolutionData state(1);
    const Evolution::Key<int> counter("Counter");
    auto inner = std::make_shared<Evolution::Module>("Inner");
    inner->add(std::make_shared<Evolution::LambdaStep>("NeedsCounter", [](Evolution::EvolutionData&) {}, std::set<std::string>{counter.name()}));
    root.add(inner);
    try {
        root.resume(state, 1);
        FAIL() << "The nested step requires data that nothing provides";
    } catch(const std::runtime_error& error) {
        EXPECT_NE(std::string(error.what()).find("NeedsCounter"), std::string::npos) << error.what();
    }

    // Data provided by the initialization are available in the nested step.
    root.setInitialization(std::make_shared<Evolution::LambdaStep>("Create", [counter](Evolution::EvolutionData& state) { state.set(counter, std::make_shared<int>(0)); },
                                                                   std::set<std::string>{}, std::set<std::string>{counter.name()}));
    EXPECT_NO_THROW(root.resume(state, 1));
}

TEST_F(ModuleTest, presetRunEveryOtherGeneration)
{
    // The generation of the second population runs every second generation of the first one.
    auto root = std::make_shared<Evolution::Module>("Evolution");
    Evolution::EvolutionData state(4);
    Evolution::BasicEvolution first = makeEvolution(10, 10, 2);
    first.names = namesOf("First");
    Evolution::BasicEvolution second = makeEvolution(6, 6, 2);
    second.names = namesOf("Second");

    root->add(first.build());
    const std::shared_ptr<Evolution::Module> slow = second.build();
    slow->setPeriod(2);
    size_t nbRuns = 0;
    slow->add(std::make_shared<Evolution::LambdaStep>("CountRuns", [&nbRuns](Evolution::EvolutionData&) { nbRuns++; }));
    root->add(slow);

    root->resume(state, 4);
    EXPECT_EQ(nbRuns, 2U) << "Generations 0 and 2";
    EXPECT_EQ(state.getIndividuals(Evolution::BasicEvolution::Keys(namesOf("First")).individuals).size(), 10U);
    EXPECT_EQ(state.getIndividuals(Evolution::BasicEvolution::Keys(namesOf("Second")).individuals).size(), 6U);
    EXPECT_NE(root->summary().find("Second (every 2 generations)"), std::string::npos) << root->summary();
}

TEST_F(ModuleTest, evolveCreatesTheDataAndResumeContinues)
{
    // evolve starts a new evolution and returns its data, resume goes on with the data of an evolution.
    auto module = makeSteps(20, 20, 2);
    Evolution::EvolutionData data = module->evolve(3, 4);
    EXPECT_EQ(data.nbGenerationsDone, 3U);
    EXPECT_EQ(data.getIndividuals(Evolution::BasicEvolution::Keys().individuals).size(), 20U);
    module->resume(data, 2);
    EXPECT_EQ(data.nbGenerationsDone, 5U);

    // The same evolution in one go, and a new evolution does not depend on the previous one.
    Evolution::EvolutionData reference = makeSteps(20, 20, 2)->evolve(5, 4);
    EXPECT_EQ(data.rng.uniformSample<uint64_t>(0, UINT64_MAX), reference.rng.uniformSample<uint64_t>(0, UINT64_MAX));
    EXPECT_EQ(module->evolve(1).nbGenerationsDone, 1U);
    EXPECT_EQ(data.nbGenerationsDone, 5U) << "The data of the first evolution are untouched";
}

TEST_F(ModuleTest, summaryShowsTheDataOfTheEvolution)
{
    // Validation every second generation: its fitness is older than the others after two generations.
    auto module = makeSteps(10, 10, 2, 2, 1, 2);
    Evolution::EvolutionData data(4);

    std::string summary = module->summary(data);
    EXPECT_NE(summary.find("initialization [pending]"), std::string::npos) << summary;
    EXPECT_NE(summary.find("(empty)"), std::string::npos) << summary;

    module->resume(data, 2);
    summary = module->summary(data);
    EXPECT_NE(summary.find("initialization [done]"), std::string::npos) << summary;
    EXPECT_EQ(summary.find("[pending]"), std::string::npos) << summary;
    EXPECT_NE(summary.find("Data after 2 generation(s):"), std::string::npos) << summary;
    EXPECT_NE(summary.find("Parents"), std::string::npos) << summary;
    EXPECT_NE(summary.find("10 elements"), std::string::npos) << summary;
    EXPECT_NE(summary.find("up to date"), std::string::npos) << summary;
    EXPECT_NE(summary.find("stored at generation 0"), std::string::npos) << "The validation fitness is older: " << summary;

    // Numbers are shown with their value.
    data.set(Counter, std::make_shared<int>(7));
    EXPECT_NE(data.summary().find("Counter"), std::string::npos);
    EXPECT_NE(data.summary().find("7"), std::string::npos);
}

TEST_F(ModuleTest, namesAreSettings)
{
    // Every name is a setting, changed before the build. Data and steps are found under their new names.
    Evolution::BasicEvolution evolution = makeEvolution(10, 10, 2);
    EXPECT_EQ(Evolution::BasicEvolution::Keys(evolution.names).parents.name(), "Parents") << "The names have defaults";
    evolution.names = namesOf("Pool");
    evolution.names.parents = "MatingPool";
    evolution.names.mutationStep = "Variation";
    evolution.names.initialEvaluationStep = "FirstEvaluation";
    evolution.names.initialization = "Start";

    auto module = evolution.build();
    EXPECT_EQ(module->name(), "Pool");
    EXPECT_TRUE(module->has("Variation"));
    EXPECT_FALSE(module->has("Mutation"));
    EXPECT_TRUE(module->getInitialization()->has("FirstEvaluation"));
    EXPECT_EQ(module->getInitialization()->name(), "Start");

    Evolution::EvolutionData data = module->evolve(2);
    const Evolution::BasicEvolution::Keys keys(evolution.names);
    EXPECT_EQ(keys.parents.name(), "MatingPool");
    EXPECT_EQ(keys.offspring.name(), "Pool.offspring");
    EXPECT_EQ(data.get(keys.parents)->size(), 10U);
    EXPECT_EQ(data.getIndividuals(keys.individuals).size(), 10U);
    EXPECT_FALSE(data.has(Evolution::Key<std::vector<std::shared_ptr<Individual>>>("Parents")));
}

TEST_F(ModuleTest, createGeneratorStep)
{
    const Evolution::Key<GraphBased::GenotypeGenerator> key("Generator");
    Evolution::EvolutionData state;

    // The naive generator of a representation.
    Evolution::CreateGeneratorStep naive(key, lgpRep);
    EXPECT_EQ(naive.name(), "CreateGenerator");
    EXPECT_TRUE(naive.requiredData().empty());
    EXPECT_EQ(naive.providedData(), (std::set<std::string>{key.name()}));
    naive.apply(state);
    ASSERT_TRUE(state.has(key));
    EXPECT_EQ(state.get(key)->size(), 1U);
    EXPECT_THROW(Evolution::CreateGeneratorStep(key, std::shared_ptr<const Representations::Representation>()), std::runtime_error);

    // A generator built from the data, again each time the step runs.
    size_t nbCalls = 0;
    Evolution::CreateGeneratorStep fromData(key, [this, &nbCalls](const Evolution::EvolutionData&) {
        nbCalls++;
        return std::make_shared<GraphBased::GenotypeGenerator>(lgpRep->getGenotypeConstraint());
    }, std::set<std::string>{"Pool"}, "FromData");
    EXPECT_EQ(fromData.name(), "FromData");
    EXPECT_EQ(fromData.requiredData(), (std::set<std::string>{"Pool"}));
    fromData.apply(state);
    fromData.apply(state);
    EXPECT_EQ(nbCalls, 2U);
    EXPECT_THROW(Evolution::CreateGeneratorStep(key, std::function<std::shared_ptr<GraphBased::GenotypeGenerator>(const Evolution::EvolutionData&)>()), std::runtime_error);
    Evolution::CreateGeneratorStep nothing(key, [](const Evolution::EvolutionData&) { return std::shared_ptr<GraphBased::GenotypeGenerator>(); });
    EXPECT_THROW(nothing.apply(state), std::runtime_error) << "A step must create a generator";
}

TEST_F(ModuleTest, presetCreatesTheGenerator)
{
    // The preset creates the generator first, once, from its settings.
    Evolution::BasicEvolution evolution = makeEvolution(10, 10, 2);
    const Evolution::BasicEvolution::Keys keys(evolution.names);
    auto module = evolution.build();
    EXPECT_EQ(module->getInitialization()->getSteps().front()->name(), "CreateGenerator");
    EXPECT_FALSE(module->has("CreateGenerator")) << "Only in the initialization";
    Evolution::EvolutionData data = module->evolve(2);
    EXPECT_TRUE(data.has(keys.generator));

    // A generator given in the settings is the one found in the data.
    evolution.generator = std::make_shared<GraphBased::GenotypeGenerator>(lgpRep->getGenotypeConstraint());
    Evolution::EvolutionData custom = evolution.build()->evolve(1);
    EXPECT_EQ(custom.get(keys.generator), evolution.generator);

    // Updating the generator is adding a step: the steps creating and mutating the population read it from the data.
    auto updated = makeEvolution(10, 10, 2).build();
    size_t nbCreations = 0;
    updated->insertBefore(updated->find("Mutation"), std::make_shared<Evolution::CreateGeneratorStep>(keys.generator, [this, &nbCreations](const Evolution::EvolutionData&) {
        nbCreations++;
        return std::make_shared<GraphBased::GenotypeGenerator>(lgpRep->getGenotypeConstraint());
    }, std::set<std::string>{}, "UpdateGenerator"));
    updated->evolve(3);
    EXPECT_EQ(nbCreations, 3U);
}

TEST_F(ModuleTest, loggerCustomColumns)
{
    Evolution::BasicEvolution evolution = makeEvolution(20, 20, 2);
    auto root = evolution.build();
    const Evolution::BasicEvolution::Keys population;

    // The time of the evaluation alone (the log adds the time of the generation), the training fitness, and the fitness of the 3 best individuals.
    const auto evaluation = root->find("TrainingEvaluation");
    std::vector<Evolution::LogStep::Column> columns{{"T_Eval", [evaluation](const Evolution::EvolutionData&) { return evaluation->getLastDuration(); }, 4}};
    const std::vector<Evolution::LogStep::Column> fitness = Evolution::fitnessColumns("Fit", population.individuals, population.trainingFitness);
    columns.insert(columns.end(), fitness.begin(), fitness.end());
    for(size_t rank = 0; rank < 3; rank++) {
        columns.push_back({"Best" + std::to_string(rank + 1), [population, rank](const Evolution::EvolutionData& data) {
            std::vector<double> values;
            const auto fitnessOfIndividuals = data.getIfAny(population.trainingFitness);
            for(const auto& individual: data.getIndividuals(population.individuals)) {
                const auto found = (fitnessOfIndividuals == nullptr) ? std::map<uint64_t, double>::const_iterator() : fitnessOfIndividuals->find(individual->getIndividualID());
                if(fitnessOfIndividuals != nullptr && found != fitnessOfIndividuals->end()) {
                    values.push_back(found->second);
                }
            }
            std::sort(values.begin(), values.end(), std::greater<double>());
            return (rank < values.size()) ? values.at(rank) : std::nan("");
        }});
    }
    std::ostringstream output;
    root->add(std::make_shared<Evolution::LogStep>(output, columns));
    root->evolve(3, 4);

    // The heading has the custom columns and the time of the generation, and the best fitnesses are sorted and the first one is the maximum.
    std::istringstream lines(output.str());
    std::string heading;
    std::getline(lines, heading);
    for(const std::string& name: {"T_Eval", "Fit_Avg", "Best1", "Best3", "T_Gen"}) {
        EXPECT_NE(heading.find(name), std::string::npos) << name;
    }
    std::string last;
    for(std::string line; std::getline(lines, line);) {
        last = line;
    }
    std::istringstream fields(last);
    std::string generation;
    double timeEvaluation, min, average, max, best1, best2, best3;
    fields >> generation >> timeEvaluation >> min >> average >> max >> best1 >> best2 >> best3;
    EXPECT_EQ(generation, "2");
    EXPECT_GE(best1, best2);
    EXPECT_GE(best2, best3);
    EXPECT_EQ(best1, max);
}
