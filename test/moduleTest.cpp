#include <gtest/gtest.h>
#include "evolutionTestHelpers.h"
#include "util/individualUtils.h"
#include <algorithm>
#include <iterator>
#include <filesystem>
#include <memory>
#include <sstream>
#include <vector>

#include "instructions/lambdaInstruction.h"
#include "instructions/set.h"
#include "learn/stickGameWithOpponent.h"

#include "evaluation/reinforcementProblem.h"
#include "evolution/basicEvolutionConfig.h"
#include "learn/populationNames.h"
#include "evolution/steps/exportStep.h"
#include "evolution/steps/logStep.h"
#include "evolution/steps/assignFitnessStep.h"
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
#include "selection/tournamentSelector.h"
#include "mutation/nodeTransferMutation.h"
#include "fitnessAssignment/noveltySearchAssigner.h"
#include "fitnessAssignment/classificationF1Assigner.h"
#include "fitnessAssignment/noveltyDescriptors/scalarListDescriptor.h"
#include "fitnessAssignment/noveltyDescriptors/averageVectorDescriptor.h"
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
    const Evolution::BasicEvolutionConfig defaults;

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
    Evolution::BasicEvolutionConfig makeEvolution(size_t sizePopulation, size_t nbOffspring, size_t nbTrainingIterations,
                                            size_t nbValidationIterations = 0, size_t nbThreads = 1, size_t validationPeriod = 1)
    {
        Evolution::BasicEvolutionConfig evolution;
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
        return makeEvolution(sizePopulation, nbOffspring, nbTrainingIterations, nbValidationIterations, nbThreads, validationPeriod).buildModule();
    }

    // Names of the data and components in the tests of the steps.
    const std::string generatorKey{"Generator"};
    const std::string mutatorKey{"PointMutator"};
    const std::string replicatorKey{"Replicator"};
    const std::string assignerKey{"DefaultAssigner"};
    const std::string metricTemplateKey{"RewardsTemplate"};

    // Store the generator of the representation in a state, as the components of a preset are.
    void setGenerator(Evolution::EvolutionData& state) const
    {
        state.set(generatorKey, std::make_shared<GraphBased::GenotypeGenerator>(lgpRep->getGenotypeConstraint()));
    }

    // Store in a state the components read by the steps of the tests, under the type the steps read.
    void addComponents(Evolution::EvolutionData& state) const
    {
        state.set(lgpRep->getName(), std::shared_ptr<const Representations::Representation>(lgpRep));
        state.set(problem->getName(), std::shared_ptr<Evaluation::Problem>(problem));
        state.set(mutatorKey, std::shared_ptr<Mutation::Mutator>(std::make_shared<Mutation::PointMutator>(0.2, 0.5, 0.5)));
        state.set(replicatorKey, std::shared_ptr<Reproduction::Reproducer>(std::make_shared<Reproduction::Replicator>()));
        state.set(assignerKey, std::shared_ptr<FitnessAssignment::Assigner>(std::make_shared<FitnessAssignment::DefaultAssigner>(Metrics::RewardsMetric::staticHash())));
        state.set(metricTemplateKey, std::make_shared<Metrics::MetricMapTemplate>(std::make_unique<Metrics::RewardsMetric>()));
    }

    // Fill a state with the components, the random individuals of a population and its offspring, ready for the first generation.
    void initPopulation(Evolution::EvolutionData& state, const Evolution::BasicEvolutionConfig::Names& population, size_t nbOffspring) const
    {
        setGenerator(state);
        addComponents(state);
        Evolution::InitPopulationStep(population.population, lgpRep->getName(), 10, mutatorKey, generatorKey).apply(state);
        state.generation = 1;
        Evolution::ReproduceStep(population.population, replicatorKey, nbOffspring, population.offspring).apply(state);
    }

    // Evaluate the individuals of a population and compute their fitness, see initPopulation.
    void evaluatePopulation(Evolution::EvolutionData& state, const Evolution::BasicEvolutionConfig::Names& population) const
    {
        Evolution::EvaluateStep({population.population}, population.trainingMetrics, problem->getName(), metricTemplateKey, 2).apply(state);
        Evolution::AssignFitnessStep(population.trainingMetrics, population.trainingFitness, assignerKey).apply(state);
    }
};

TEST_F(ModuleTest, classicFlow)
{
    auto evolutionModule = makeSteps(100, 100, 3, 0, 10);
    Evolution::EvolutionData& state = evolutionModule->evolve(20, 4);

    const Evolution::BasicEvolutionConfig::Names population;
    EXPECT_EQ(state.nbGenerationsDone, 20U);
    const std::vector<std::shared_ptr<const Individual>> individuals = constIndividuals(*state.get<std::vector<std::shared_ptr<Individual>>>(population.population));
    EXPECT_EQ(individuals.size(), 100U);
    EXPECT_EQ(state.get<std::vector<std::shared_ptr<Individual>>>(population.parents)->size(), 100U);
    EXPECT_EQ(state.get<std::vector<std::shared_ptr<Individual>>>(population.offspring)->size(), 100U);

    // The fitness and metrics are the ones of the last evaluation, of the population and of the offspring.
    const std::map<uint64_t, double>& fitness = *state.get<std::map<uint64_t, double>>(population.trainingFitness);
    EXPECT_EQ(fitness.size(), 200U);
    EXPECT_EQ((state.get<std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>>(population.trainingMetrics)->size()), 200U);

    // The best individual is in the population, with the highest fitness of the population.
    std::shared_ptr<const Individual> best = Util::bestIndividual(constIndividuals(*state.get<std::vector<std::shared_ptr<Individual>>>(population.population)), *state.get<std::map<uint64_t, double>>(population.trainingFitness));
    ASSERT_NE(best, nullptr);
    EXPECT_NE(std::find(individuals.begin(), individuals.end(), best), individuals.end());
    for(const std::shared_ptr<const Individual>& individual: individuals) {
        EXPECT_LE(fitness.at(individual->getIndividualID()), fitness.at(best->getIndividualID()));
    }

    ASSERT_EQ(1704165046166301501U, state.rng.uniformSample<uint64_t>(0, UINT64_MAX)) << "determinism";
}

TEST_F(ModuleTest, summary)
{
    auto evolutionModule = makeSteps(10, 10, 3, 2);
    const std::string summary = evolutionModule->summary();
    // Each step with its numeric parameters only.
    for(const std::string& text: {"InitPopulation (10 individuals)", "InitialEvaluation (3 iterations, 1 thread(s))", "ParentSelection (10 selected)",
                                  "Reproduction (10 offspring)", "Mutation (1 thread(s))", "TrainingEvaluation (3 iterations, 1 thread(s))",
                                  "SurvivorSelection (10 selected)", "ValidationEvaluation (2 iterations, 1 thread(s))", "ValidationFitness"}) {
        EXPECT_NE(summary.find(text), std::string::npos) << text << "\n" << summary;
    }
    EXPECT_NE(summary.find("requires: Parents, Replicator | provides: Offspring"), std::string::npos) << summary;
}

TEST_F(ModuleTest, summaryDepth)
{
    // With one level, the validation is one line, with the data required and provided by its steps.
    auto evolutionModule = makeSteps(10, 10, 3, 2);
    const std::string all = evolutionModule->summary();
    const std::string oneLevel = evolutionModule->summary(1);
    EXPECT_NE(all.find("ValidationEvaluation"), std::string::npos) << all;
    EXPECT_EQ(oneLevel.find("- ValidationEvaluation ("), std::string::npos) << oneLevel;
    EXPECT_NE(oneLevel.find("provides: validationFitness, validationMetrics"), std::string::npos) << oneLevel;
    EXPECT_NE(oneLevel.find("`- (ValidationEvaluation, ValidationFitness)"), std::string::npos) << "The hidden steps are listed: " << oneLevel;
    EXPECT_NE(oneLevel.find("InitPopulation"), std::string::npos) << "The steps of the initialization are at the first level: " << oneLevel;
    EXPECT_EQ(evolutionModule->summary(0).find("Mutation"), std::string::npos) << "Only the module itself";
}

TEST_F(ModuleTest, settingsAreChecked)
{
    Evolution::BasicEvolutionConfig evolution;
    EXPECT_THROW(evolution.buildModule(), std::runtime_error) << "No representation nor problem";
    evolution.representation = lgpRep;
    evolution.problem = problem;
    EXPECT_THROW(evolution.buildModule(), std::runtime_error) << "No population size";
    evolution.sizePopulation = 10;
    evolution.nbOffspring = 10;
    EXPECT_NO_THROW(evolution.buildModule());
    evolution.nbValidationIterations = 2;
    evolution.validationPeriod = 0;
    EXPECT_THROW(evolution.buildModule(), std::runtime_error) << "The validation period cannot be zero";
}

TEST_F(ModuleTest, insertSecondMutation)
{
    auto evolutionModule = makeSteps(20, 20, 2);
    const Evolution::BasicEvolutionConfig::Names population;

    // A second mutation after the first one, with its own mutator in the data, then a custom step counting the generations.
    size_t nbCalls = 0;
    evolutionModule->getData().set("StrongMutator", std::shared_ptr<Mutation::Mutator>(std::make_shared<Mutation::PointMutator>(0.5, 0.5, 0.5, "StrongMutator")));
    auto secondMutation = std::make_shared<Evolution::MutateStep>(population.offspring, "StrongMutator", population.generator, 1, "SecondMutation");
    (*evolutionModule)
        .insertAfter("Mutation", secondMutation)
        .insertAfter("SecondMutation", std::make_shared<Evolution::LambdaStep>("Count", [&nbCalls](Evolution::EvolutionData&) { nbCalls++; }));
    ASSERT_EQ(evolutionModule->size(), 8) << "The preset is one step holding the steps of a generation";

    Evolution::EvolutionData& state = evolutionModule->evolve(5, 4);
    EXPECT_EQ(nbCalls, 5U);
    for(const auto& individual: constIndividuals(*state.get<std::vector<std::shared_ptr<Individual>>>(population.population))) {
        EXPECT_TRUE(individual->isValid());
    }

    // Each step knows how long it took, and a step holding steps lasts at least as long as them.
    EXPECT_GT(secondMutation->getLastDuration(), 0.0);
    EXPECT_GT(evolutionModule->find("Mutation")->getLastDuration(), 0.0);
    EXPECT_EQ(Evolution::LambdaStep("Unused", [](Evolution::EvolutionData&) {}).getLastDuration(), 0.0) << "Zero until the step ran";
}

TEST_F(ModuleTest, editPipeline)
{
    auto evolutionModule = makeSteps(10, 10, 2);
    Evolution::Module& steps = *evolutionModule;
    const std::shared_ptr<Evolution::MutateStep> mutation = steps.find<Evolution::MutateStep>("Mutation");

    EXPECT_TRUE(steps.has("Mutation"));
    EXPECT_FALSE(steps.has("Unknown"));
    EXPECT_FALSE(steps.has("InitPopulation")) << "Only in the initialization";
    EXPECT_THROW(steps.find("Unknown"), std::runtime_error);
    EXPECT_THROW(steps.find<Evolution::SelectStep>("Mutation"), std::runtime_error) << "Not a selection";
    EXPECT_THROW(steps.insertAfter("Unknown", std::make_shared<Evolution::LambdaStep>("Step", [](Evolution::EvolutionData&) {})), std::runtime_error);
    EXPECT_THROW(steps.insertBefore("Mutation", nullptr), std::runtime_error);

    // Steps are identified by their names, unique in a module.
    EXPECT_THROW(steps.insertBefore("Mutation", std::make_shared<Evolution::LambdaStep>("Mutation", [](Evolution::EvolutionData&) {})), std::runtime_error);
    auto before = std::make_shared<Evolution::LambdaStep>("Before", [](Evolution::EvolutionData&) {});
    steps.insertBefore("Mutation", before);
    EXPECT_EQ(steps.getSteps().at(2), before);
    EXPECT_EQ(steps.getSteps().at(3), mutation);
    auto replaced = std::make_shared<Evolution::LambdaStep>("Replaced", [](Evolution::EvolutionData&) {});
    steps.replace("Before", replaced);
    EXPECT_EQ(steps.getSteps().at(2), replaced);
    steps.remove("Replaced");
    EXPECT_EQ(steps.getSteps().at(2), mutation);
    EXPECT_THROW(steps.remove("Replaced"), std::runtime_error);
}

TEST_F(ModuleTest, misorderedPipelineIsRejected)
{
    auto evolutionModule = makeSteps(10, 10, 2);
    evolutionModule->remove("ParentSelection");

    // The reproduction now has no parents.
    EXPECT_THROW(evolutionModule->evolve(1, 0), std::runtime_error);
}

TEST_F(ModuleTest, evolveContinues)
{
    // Running more generations continues the same evolution.
    auto reference = makeSteps(20, 20, 2);
    Evolution::EvolutionData& referenceState = reference->evolve(5, 4);
    const uint64_t referenceDraw = referenceState.rng.uniformSample<uint64_t>(0, UINT64_MAX);

    auto continued = makeSteps(20, 20, 2);
    Evolution::EvolutionData& state = continued->evolve(3, 4);
    continued->runGenerations(2);
    EXPECT_EQ(state.nbGenerationsDone, 5U);
    EXPECT_EQ(referenceDraw, state.rng.uniformSample<uint64_t>(0, UINT64_MAX));

    // Evolving again starts a new evolution, from the initialization: the same seed gives the same evolution.
    continued->evolve(5, 4);
    EXPECT_EQ(state.nbGenerationsDone, 5U);
    EXPECT_EQ(referenceDraw, state.rng.uniformSample<uint64_t>(0, UINT64_MAX));
    EXPECT_EQ(constIndividuals(*state.get<std::vector<std::shared_ptr<Individual>>>(Evolution::BasicEvolutionConfig::Names().population)).size(), 20U);
}

TEST_F(ModuleTest, customStopIsALoop)
{
    // Stopping is the business of the caller: it evolves one generation at a time.
    auto evolutionModule = makeSteps(10, 10, 2);
    const Evolution::EvolutionData& state = evolutionModule->evolve(0, 0);
    while(state.nbGenerationsDone < 2) {
        evolutionModule->runGenerations(1);
    }
    EXPECT_EQ(state.nbGenerationsDone, 2U);
}

TEST_F(ModuleTest, validationAndLogger)
{
    std::ostringstream output;
    Evolution::BasicEvolutionConfig evolution = makeEvolution(10, 10, 2, 2, 1, 2);
    evolution.logOutput = &output;
    auto evolutionModule = evolution.buildModule();
    const Evolution::BasicEvolutionConfig::Names population;
    const std::string validationFitness = evolutionModule->find("Validation")->find<Evolution::AssignFitnessStep>("ValidationFitness")->getFitness();
    EXPECT_EQ(validationFitness, (population.validationFitness));
    EXPECT_EQ(evolutionModule->getSteps().back()->name(), "Log") << "The log is the last step of a generation";

    Evolution::EvolutionData& state = evolutionModule->evolve(3, 4);

    const std::string log = output.str();
    EXPECT_NE(log.find("Val_Avg"), std::string::npos);

    // Validation ran on the generations 0 (the initialization) and 2 only.
    EXPECT_FALSE((state.get<std::map<uint64_t, double>>(validationFitness)->empty()));
    EXPECT_FALSE(state.isUpToDate(validationFitness));
    EXPECT_TRUE(state.isUpToDate(population.trainingFitness));

    // Rows of generations 1 and 3 show "-" instead of the older validation values.
    std::istringstream lines(log);
    size_t nbRowsWithoutValidation = 0;
    for(std::string line; std::getline(lines, line);) {
        std::istringstream fields(line);
        const std::vector<std::string> values{std::istream_iterator<std::string>(fields), std::istream_iterator<std::string>()};
        nbRowsWithoutValidation += (std::find(values.begin(), values.end(), "-") != values.end()) ? 1 : 0;
    }
    EXPECT_EQ(nbRowsWithoutValidation, 2U);
    EXPECT_NE(evolutionModule->summary().find("every 2 generations)"), std::string::npos) << evolutionModule->summary();
    EXPECT_THROW(evolutionModule->find("Validation")->setPeriod(0), std::runtime_error);
}

TEST_F(ModuleTest, validationWithItsOwnProblemAndAssigner)
{
    // Components of the validation have their own names, to be told apart from the training ones in the data.
    auto validationProblem = std::make_shared<Evaluation::ReinforcementProblem>(le, 5, "ValidationProblem");
    Evolution::BasicEvolutionConfig evolution = makeEvolution(10, 10, 2, 3);
    evolution.validationProblem = validationProblem;
    evolution.validationAssigner = std::make_shared<FitnessAssignment::DefaultAssigner>(Metrics::RewardsMetric::staticHash(), "ValidationAssigner");
    auto evolutionModule = evolution.buildModule();
    const Evolution::BasicEvolutionConfig::Names population;

    const std::shared_ptr<Evolution::EvaluateStep> validation = evolutionModule->find("Validation")->find<Evolution::EvaluateStep>("ValidationEvaluation");
    EXPECT_EQ(validation->getProblem(), validationProblem->getName());
    EXPECT_EQ(validation->getMode(), Evaluation::Mode::VALIDATION);
    EXPECT_EQ(evolutionModule->find<Evolution::EvaluateStep>("TrainingEvaluation")->getProblem(), problem->getName());
    Evolution::EvolutionData& state = evolutionModule->evolve(2, 0);

    EXPECT_EQ((state.get<std::map<uint64_t, double>>(population.validationFitness)->size()), 10U);
    EXPECT_NE(Util::bestIndividual(constIndividuals(*state.get<std::vector<std::shared_ptr<Individual>>>(population.population)), *state.get<std::map<uint64_t, double>>(population.validationFitness)), nullptr);
}

TEST_F(ModuleTest, parentsWithTheirOwnFitness)
{
    Evolution::BasicEvolutionConfig evolution = makeEvolution(10, 10, 2);
    evolution.parentAssigner = std::make_shared<FitnessAssignment::DefaultAssigner>(Metrics::RewardsMetric::staticHash(), "ParentAssigner");
    auto evolutionModule = evolution.buildModule();
    const Evolution::BasicEvolutionConfig::Names population;

    // The parent fitness is the first step of a generation, computed from the metrics of the previous one.
    EXPECT_EQ(evolutionModule->find("ParentSelection")->requiredData().count(population.parentFitness), 1U);
    EXPECT_EQ(evolutionModule->getSteps().front()->name(), "ParentFitness");
    EXPECT_EQ(evolutionModule->find("ParentFitness")->requiredData(), (std::set<std::string>{"ParentAssigner", population.trainingMetrics}));

    Evolution::EvolutionData& state = evolutionModule->evolve(3, 0);
    EXPECT_TRUE(state.isUpToDate(population.parentFitness));

    // Without the parent fitness, the parents cannot be selected.
    auto withoutParentFitness = evolution.buildModule();
    withoutParentFitness->remove("ParentFitness");
    EXPECT_THROW(withoutParentFitness->evolve(1, 0), std::runtime_error);
}

TEST_F(ModuleTest, selectionIsGeneric)
{
    Evolution::EvolutionData state;
    auto a = std::make_shared<Individual>(lgpRep);
    auto b = std::make_shared<Individual>(lgpRep);
    auto c = std::make_shared<Individual>(lgpRep);
    auto other = std::make_shared<Individual>(lgpRep);
    const std::string listA("A");
    const std::string listB("B");
    const std::string fitness("Fitness");
    const std::string selected("Selected");
    state.set(listA, std::make_shared<std::vector<std::shared_ptr<Individual>>>(std::vector<std::shared_ptr<Individual>>{c, a}));
    state.set(listB, std::make_shared<std::vector<std::shared_ptr<Individual>>>(std::vector<std::shared_ptr<Individual>>{b}));
    state.set(fitness, std::make_shared<std::map<uint64_t, double>>(std::map<uint64_t, double>{
        {a->getIndividualID(), 3.0}, {b->getIndividualID(), 2.0}, {c->getIndividualID(), 1.0}, {other->getIndividualID(), 10.0}}));
    state.set("TruncationSelector", std::shared_ptr<Selection::Selector>(std::make_shared<Selection::TruncationSelector>()));
    state.set("RandomSelector", std::shared_ptr<Selection::Selector>(std::make_shared<Selection::RandomSelector>(true)));

    // The best candidates are kept once each, ordered by ID: the fitness of non candidates is ignored.
    Evolution::SelectStep best({listA, listB}, fitness, "TruncationSelector", 2, selected, "Best");
    EXPECT_EQ(best.name(), "Best");
    EXPECT_EQ(best.requiredData(), (std::set<std::string>{listA, listB, fitness, "TruncationSelector"}));
    best.apply(state);
    EXPECT_EQ(constIndividuals(*state.get<std::vector<std::shared_ptr<Individual>>>(selected)), (std::vector<std::shared_ptr<const Individual>>{a, b}));

    // A selection can replace one of its candidate lists, and keep duplicates.
    Evolution::SelectStep withDuplicates({listA}, fitness, "RandomSelector", 5, listA);
    EXPECT_EQ(withDuplicates.name(), "Select");
    withDuplicates.apply(state);
    EXPECT_EQ(state.get<std::vector<std::shared_ptr<Individual>>>(listA)->size(), 5U);
    EXPECT_THROW(Evolution::SelectStep({}, fitness, "TruncationSelector", 2, selected), std::runtime_error);
}

TEST_F(ModuleTest, loggerColumns)
{
    std::ostringstream output;
    Evolution::BasicEvolutionConfig evolution = makeEvolution(10, 10, 2);
    evolution.logOutput = &output;
    const Evolution::BasicEvolutionConfig::Names population;
    const std::string offspring = population.offspring;

    // The columns of the preset log are its steps, and a custom one is added like any step.
    auto evolutionModule = evolution.buildModule();
    const std::shared_ptr<Evolution::LogModule> log = evolutionModule->find<Evolution::LogModule>("Log");
    EXPECT_TRUE(log->has("Pop_Size"));
    EXPECT_TRUE(log->has("Fit_Avg"));
    log->addSubModule(std::make_shared<Evolution::LogColumnStep>("Nb_Offspring", [offspring](const Evolution::EvolutionData& data) {
        return static_cast<double>(data.get<std::vector<std::shared_ptr<Individual>>>(offspring)->size());
    }, 0, std::set<std::string>{offspring}));

    // Two evolutions of the same module write in the same log.
    evolutionModule->evolve(2, 0);
    evolutionModule->evolve(1, 0);

    // The training fitness without the validation, the custom column, and one header per evolution.
    const std::string text = output.str();
    EXPECT_NE(text.find("Fit_Avg"), std::string::npos);
    EXPECT_NE(text.find("Pop_Size"), std::string::npos);
    EXPECT_EQ(text.find("Val_Avg"), std::string::npos);
    size_t nbHeaders = 0;
    for(size_t pos = text.find("Nb_Offspring"); pos != std::string::npos; pos = text.find("Nb_Offspring", pos + 1)) {
        nbHeaders++;
    }
    EXPECT_EQ(nbHeaders, 2U);
}

TEST_F(ModuleTest, loggerRequiresColumnData)
{
    Evolution::BasicEvolutionConfig evolution = makeEvolution(10, 10, 2);
    auto evolutionModule = evolution.buildModule();
    std::ostringstream output;

    // The log requires the data of its columns, so the evolution refuses a column reading what no step provides.
    auto log = std::make_shared<Evolution::LogModule>(output);
    log->addSubModule(std::make_shared<Evolution::LogColumnStep>("Nowhere_Size", [](const Evolution::EvolutionData&) { return 0.0; }, 0, std::set<std::string>{"Nowhere"}));
    EXPECT_EQ(log->requiredData(), (std::set<std::string>{"Nowhere"}));
    evolutionModule->addSubModule(log);
    EXPECT_THROW(evolutionModule->evolve(1, 0), std::runtime_error);
}

TEST_F(ModuleTest, loggerSeparator)
{
    // One log aligned for the console (the preset one), and one written for another tool: the size of the population and the time
    // of each step of a generation, separated by ";".
    std::ostringstream table;
    std::ostringstream csv;
    Evolution::BasicEvolutionConfig evolution = makeEvolution(10, 10, 2);
    evolution.logOutput = &table;
    auto evolutionModule = evolution.buildModule();
    evolutionModule->find<Evolution::LogModule>("Log")->setColumnWidth(10);
    Evolution::BasicLogConfig csvConfig;
    csvConfig.output = &csv;
    csvConfig.separator = ";";
    csvConfig.name = "CsvLog";
    std::shared_ptr<Evolution::LogModule> csvLog = csvConfig.buildModule();
    Evolution::BasicLogConfig::addStepTimeColumns(*csvLog, *evolutionModule);
    evolutionModule->addSubModule(csvLog);
    evolutionModule->evolve(3, 0);

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
        EXPECT_EQ(line.substr(0, line.find(';')), std::to_string(nbRows)) << "The log starts with the generation 0, the initial state";
        nbRows++;
    }
    EXPECT_EQ(nbRows, 4U) << "The generation 0 (the initial state) and the three generations";
}

TEST_F(ModuleTest, stepsOfThePreset)
{
    auto evolutionModule = makeSteps(10, 10, 3);
    const Evolution::BasicEvolutionConfig::Names population;
    const auto evaluation = evolutionModule->find<Evolution::EvaluateStep>("TrainingEvaluation");
    EXPECT_EQ(evaluation->getNbIterations(), 3U);
    EXPECT_EQ(evaluation->getEvaluated(), (std::vector<std::string>{population.population, population.offspring}));
    EXPECT_EQ(evolutionModule->getInitialization()->find<Evolution::EvaluateStep>("InitialEvaluation")->getEvaluated(), (std::vector<std::string>{population.population}));
    EXPECT_EQ(evaluation->getMetrics(), (population.trainingMetrics));
    EXPECT_EQ(evaluation->getMetricTemplate(), (population.trainingMetricTemplate));
    EXPECT_TRUE(evolutionModule->getInitialization()->has("TrainingFitness"));
    EXPECT_TRUE(evolutionModule->has("TrainingFitness"));
    EXPECT_FALSE(evolutionModule->has("ValidationEvaluation"));
}

TEST_F(ModuleTest, parentsComeFromPopulation)
{
    auto evolutionModule = makeSteps(20, 20, 2);
    const auto individuals = Evolution::BasicEvolutionConfig::Names().population;
    const auto parents = Evolution::BasicEvolutionConfig::Names().parents;

    // Individuals discarded by the survival must not be selected as parents anymore.
    size_t nbParentsChecked = 0;
    evolutionModule->insertAfter("ParentSelection", std::make_shared<Evolution::LambdaStep>("CheckParents",
        [&nbParentsChecked, individuals, parents](Evolution::EvolutionData& state) {
            const std::vector<std::shared_ptr<const Individual>> current = constIndividuals(*state.get<std::vector<std::shared_ptr<Individual>>>(individuals));
            for(const auto& parent: *state.get<std::vector<std::shared_ptr<Individual>>>(parents)) {
                EXPECT_NE(std::find(current.begin(), current.end(), parent), current.end());
                nbParentsChecked++;
            }
        }));
    evolutionModule->evolve(10, 4);
    EXPECT_EQ(nbParentsChecked, 200U);
}

namespace {
    // Custom name of the tests.
    const std::string Counter{"Counter"};
}

TEST_F(ModuleTest, stateEntries)
{
    Evolution::EvolutionData state;
    EXPECT_FALSE(state.has(Counter));
    EXPECT_THROW(state.get<int>(Counter), std::runtime_error);

    state.set(Counter, std::make_shared<int>(3));
    ASSERT_TRUE(state.has(Counter));
    EXPECT_EQ(*state.get<int>(Counter), 3);
    EXPECT_EQ(*state.get<int>("Counter"), 3) << "Keys are identified by their name";
    EXPECT_THROW(state.get<double>("Counter"), std::runtime_error) << "The entry is not a double";
    EXPECT_FALSE(state.has("Other"));
    EXPECT_THROW(state.get<int>("Other"), std::runtime_error) << "There is no such entry";

    const Evolution::EvolutionData& constState = state;
    EXPECT_EQ(*constState.get<int>(Counter), 3);

    // Entries record the generation in which they are stored, not when they are modified.
    state.generation = 4;
    EXPECT_FALSE(state.isUpToDate(Counter));
    (*state.get<int>(Counter))++;
    EXPECT_FALSE(state.isUpToDate(Counter));
    state.set(Counter, std::make_shared<int>(5));
    EXPECT_TRUE(state.isUpToDate(Counter));

    state.erase(Counter);
    EXPECT_FALSE(state.has(Counter));
    EXPECT_FALSE(state.isUpToDate(Counter));

    // Individuals are stored in lists identified by keys, and the algorithms of the library take them as individuals that cannot be modified.
    auto individual = std::make_shared<Individual>(lgpRep);
    auto other = std::make_shared<Individual>(lgpRep);
    const std::string first("First");
    const std::string second("Second");
    state.set(first, std::make_shared<std::vector<std::shared_ptr<Individual>>>(std::vector<std::shared_ptr<Individual>>{individual}));
    state.set(second, std::make_shared<std::vector<std::shared_ptr<Individual>>>(std::vector<std::shared_ptr<Individual>>{other}));
    EXPECT_EQ(constIndividuals(*state.get<std::vector<std::shared_ptr<Individual>>>(first)), (std::vector<std::shared_ptr<const Individual>>{individual}));
    EXPECT_FALSE(state.has("Absent"));
    EXPECT_THROW(state.get<std::vector<std::shared_ptr<Individual>>>("Absent"), std::runtime_error);
    state.set(Counter, std::make_shared<int>(4));
    EXPECT_THROW(constIndividuals(*state.get<std::vector<std::shared_ptr<Individual>>>(Counter)), std::runtime_error) << "The counter is not a list of individuals";
    EXPECT_THROW(state.get<double>(Counter), std::runtime_error) << "A key of the wrong type is refused at runtime too, when built from a name";
    EXPECT_EQ(*state.get<int>(Counter), 4) << "Keys are names with a type";
}

TEST_F(ModuleTest, customDataAreChecked)
{
    auto evolutionModule = makeSteps(10, 10, 2);
    evolutionModule->addSubModule(std::make_shared<Evolution::LambdaStep>("Increment",
        [](Evolution::EvolutionData& state) { (*state.get<int>(Counter))++; }, std::set<std::string>{Counter}, std::set<std::string>{Counter}));
    EXPECT_THROW(evolutionModule->evolve(1, 0), std::runtime_error) << "No step creates the counter";

    evolutionModule->getInitialization()->addSubModule(std::make_shared<Evolution::LambdaStep>("CreateCounter",
        [](Evolution::EvolutionData& state) { state.set(Counter, std::make_shared<int>(0)); }, std::set<std::string>{}, std::set<std::string>{Counter}));
    Evolution::EvolutionData& state = evolutionModule->evolve(3, 0);
    EXPECT_EQ(*state.get<int>(Counter), 3);
    EXPECT_NE(evolutionModule->summary().find("provides: Counter"), std::string::npos);
}

TEST_F(ModuleTest, mutationComponentsAreRequired)
{
    const Evolution::BasicEvolutionConfig::Names population(namesOf("P"));
    const auto offspring = population.offspring;

    // Components are read by name from the data, like the mutated individuals and the generator.
    Evolution::MutateStep mutate(offspring, mutatorKey, generatorKey);
    EXPECT_EQ(mutate.requiredData(), (std::set<std::string>{offspring, mutatorKey, generatorKey}));

    // The population is created with the representation, the mutator and the generator found in the data.
    Evolution::InitPopulationStep init(population.population, lgpRep->getName(), 10, mutatorKey, generatorKey);
    EXPECT_EQ(init.requiredData(), (std::set<std::string>{lgpRep->getName(), mutatorKey, generatorKey}));
    EXPECT_EQ(init.providedData(), (std::set<std::string>{population.population}));

    Evolution::EvolutionData state;
    addComponents(state);
    EXPECT_THROW(init.apply(state), std::runtime_error) << "No generator in the data";
    setGenerator(state);
    ASSERT_NO_THROW(init.apply(state));
    EXPECT_EQ(constIndividuals(*state.get<std::vector<std::shared_ptr<Individual>>>(population.population)).size(), 10U);
}

namespace {
    // Steps of a classic evolution of one population, added to a module and its initialization, reading the components stored by addComponents.
    std::shared_ptr<Evolution::AssignFitnessStep> addClassicSteps(Evolution::Module& evolutionModule, Evolution::Module& init, const Evolution::BasicEvolutionConfig::Names& population,
                                                                  const std::string& representation, const std::string& problem, size_t size, size_t nbOffspring)
    {
        // The steps of the two populations are in the same modules: their names start with the name of the population.
        const std::string prefix = population.population + ".";
        auto assign = std::make_shared<Evolution::AssignFitnessStep>(population.trainingMetrics, population.trainingFitness, "DefaultAssigner", prefix + "Fitness");
        init.addSubModule(std::make_shared<Evolution::InitPopulationStep>(population.population, representation, size, "PointMutator", population.generator, prefix + "Init"));
        init.addSubModule(std::make_shared<Evolution::EvaluateStep>(std::vector<std::string>{population.population}, population.trainingMetrics, problem, "RewardsTemplate", 2,
                                                                    Evaluation::Mode::TRAINING, 1, prefix + "Evaluation"));
        init.addSubModule(assign);

        evolutionModule.addSubModule(std::make_shared<Evolution::SelectStep>(std::vector<std::string>{population.population}, population.trainingFitness,
                                                                             "RandomSelector", nbOffspring, population.parents, prefix + "ParentSelection"));
        evolutionModule.addSubModule(std::make_shared<Evolution::ReproduceStep>(population.parents, "Replicator", nbOffspring, population.offspring, prefix + "Reproduction"));
        evolutionModule.addSubModule(std::make_shared<Evolution::MutateStep>(population.offspring, "PointMutator", population.generator, 1, prefix + "Mutation"));
        evolutionModule.addSubModule(std::make_shared<Evolution::EvaluateStep>(std::vector<std::string>{population.population, population.offspring},
                                                                               population.trainingMetrics, problem, "RewardsTemplate", 2, Evaluation::Mode::TRAINING, 1,
                                                                               prefix + "Evaluation"));
        evolutionModule.addSubModule(assign);
        evolutionModule.addSubModule(std::make_shared<Evolution::SelectStep>(std::vector<std::string>{population.population, population.offspring},
                                                                             population.trainingFitness, "TruncationSelector", size, population.population,
                                                                             prefix + "SurvivorSelection"));
        return assign;
    }
}

TEST_F(ModuleTest, twoPopulations)
{
    // Steps built from scratch, evolving two populations with the same kind of steps and the same components.
    const Evolution::BasicEvolutionConfig::Names populationA(namesOf("A"));
    const Evolution::BasicEvolutionConfig::Names populationB(namesOf("B"));
    std::shared_ptr<Evolution::AssignFitnessStep> assignB;
    const std::function<std::shared_ptr<Evolution::Module>(bool)> makeModule = [&](bool withFitnessOfB) {
        auto evolutionModule = std::make_shared<Evolution::Module>("Evolution");
        Evolution::Module& init = evolutionModule->setInitialization();
        Evolution::EvolutionData& data = evolutionModule->getData();
        addComponents(data);
        data.set("RandomSelector", std::shared_ptr<Selection::Selector>(std::make_shared<Selection::RandomSelector>(true)));
        data.set("TruncationSelector", std::shared_ptr<Selection::Selector>(std::make_shared<Selection::TruncationSelector>()));
        data.set(populationA.generator, std::make_shared<GraphBased::GenotypeGenerator>(lgpRep->getGenotypeConstraint()));
        data.set(populationB.generator, std::make_shared<GraphBased::GenotypeGenerator>(lgpRep->getGenotypeConstraint()));

        addClassicSteps(*evolutionModule, init, populationA, lgpRep->getName(), problem->getName(), 10, 10);
        assignB = addClassicSteps(*evolutionModule, init, populationB, lgpRep->getName(), problem->getName(), 6, 4);
        if(!withFitnessOfB) {
            init.remove(assignB->name());
            evolutionModule->remove(assignB->name());
        }
        return evolutionModule;
    };

    std::shared_ptr<Evolution::Module> evolutionModule = makeModule(true);
    const std::set<std::string> provided = evolutionModule->providedData();
    for(const std::string& name: {"A.trainingFitness", "B.trainingFitness", "B.parents"}) {
        EXPECT_EQ(provided.count(name), 1U) << name;
    }
    Evolution::EvolutionData& state = evolutionModule->evolve(3, 4);

    const std::vector<std::shared_ptr<const Individual>> individualsA = constIndividuals(*state.get<std::vector<std::shared_ptr<Individual>>>(populationA.population));
    const std::vector<std::shared_ptr<const Individual>> individualsB = constIndividuals(*state.get<std::vector<std::shared_ptr<Individual>>>(populationB.population));
    EXPECT_EQ(individualsA.size(), 10U);
    EXPECT_EQ(individualsB.size(), 6U);
    for(const auto& individual: individualsB) {
        EXPECT_EQ(std::find(individualsA.begin(), individualsA.end(), individual), individualsA.end()) << "The populations are separate";
    }
    EXPECT_NE(Util::bestIndividual(constIndividuals(*state.get<std::vector<std::shared_ptr<Individual>>>(populationB.population)), *state.get<std::map<uint64_t, double>>(populationB.trainingFitness)), nullptr);

    // Steps of the second population are checked like the others.
    EXPECT_THROW(makeModule(false)->evolve(1, 4), std::runtime_error) << "No fitness for the second population";
}

TEST_F(ModuleTest, presetsShareARoot)
{
    // Two basic evolutions in the same module, with different population names. The components they share are stored once.
    auto evolutionModule = std::make_shared<Evolution::Module>("Evolution");
    Evolution::BasicEvolutionConfig first = makeEvolution(10, 10, 2);
    first.names = namesOf("First");
    evolutionModule->addSubModule(first.buildModule());
    Evolution::BasicEvolutionConfig second = makeEvolution(6, 6, 2);
    second.names = namesOf("Second");
    evolutionModule->addSubModule(second.buildModule());

    Evolution::EvolutionData& state = evolutionModule->evolve(2, 4);
    EXPECT_EQ(constIndividuals(*state.get<std::vector<std::shared_ptr<Individual>>>("First")).size(), 10U);
    EXPECT_EQ(constIndividuals(*state.get<std::vector<std::shared_ptr<Individual>>>("Second")).size(), 6U);
}

TEST_F(ModuleTest, evolutionAddedLater)
{
    // A second population added after a few generations of the first one takes part in the next evolution.
    auto evolutionModule = std::make_shared<Evolution::Module>("Evolution");
    Evolution::BasicEvolutionConfig first = makeEvolution(10, 10, 2);
    first.names = namesOf("First");
    const std::shared_ptr<Evolution::Module> firstSteps = first.buildModule();
    size_t nbInitializations = 0;
    firstSteps->getInitialization()->addSubModule(std::make_shared<Evolution::LambdaStep>("CountInitializations", [&nbInitializations](Evolution::EvolutionData&) { nbInitializations++; }));
    evolutionModule->addSubModule(firstSteps);

    Evolution::EvolutionData& state = evolutionModule->evolve(3, 4);
    const std::string secondIndividuals("Second");
    EXPECT_FALSE(state.has(secondIndividuals));

    Evolution::BasicEvolutionConfig second = makeEvolution(6, 6, 2);
    second.names = namesOf("Second");
    evolutionModule->addSubModule(second.buildModule());
    EXPECT_THROW(evolutionModule->runGenerations(2), std::runtime_error) << "The second population is not initialized";

    evolutionModule->evolve(2, 4);
    EXPECT_EQ(nbInitializations, 2U) << "A new evolution initializes the first population again";
    EXPECT_EQ(state.nbGenerationsDone, 2U);
    EXPECT_EQ(constIndividuals(*state.get<std::vector<std::shared_ptr<Individual>>>(secondIndividuals)).size(), 6U);
    EXPECT_EQ(state.get<std::vector<std::shared_ptr<Individual>>>(Evolution::BasicEvolutionConfig::Names(namesOf("First")).population)->size(), 10U);

    // The new steps are checked against the data of the module: a step needing data nobody makes is refused before anything runs.
    evolutionModule->addSubModule(std::make_shared<Evolution::LambdaStep>("NeedsCounter", [](Evolution::EvolutionData&) {}, std::set<std::string>{Counter}));
    EXPECT_THROW(evolutionModule->evolve(1, 4), std::runtime_error);
    EXPECT_EQ(nbInitializations, 2U) << "Nothing ran";
}

TEST_F(ModuleTest, requiredDataMustBeProvided)
{
    // A step requiring a data that no previous step provides is refused before anything runs.
    auto noop = [](Evolution::EvolutionData&) {};
    Evolution::Module wrong("Wrong");
    wrong.addSubModule(std::make_shared<Evolution::LambdaStep>("Require", noop, std::set<std::string>{"Shared"}));
    EXPECT_THROW(wrong.evolve(1, 0), std::runtime_error);

    Evolution::Module right("Right");
    right.addSubModule(std::make_shared<Evolution::LambdaStep>("Provide", noop, std::set<std::string>{}, std::set<std::string>{"Shared"}));
    right.addSubModule(std::make_shared<Evolution::LambdaStep>("Require", noop, std::set<std::string>{"Shared"}));
    EXPECT_NO_THROW(right.evolve(1, 0));

    // The data already in the data of the module count too.
    wrong.getData().set("Shared", std::make_shared<int>(1));
    EXPECT_NO_THROW(wrong.evolve(1, 0));
}

TEST_F(ModuleTest, setCreatesOrReplaces)
{
    Evolution::EvolutionData state;
    state.set("Data", std::make_shared<int>(1));
    EXPECT_EQ(*state.get<int>("Data"), 1) << "Set creates";
    state.set("Data", std::make_shared<int>(3));
    EXPECT_EQ(*state.get<int>("Data"), 3) << "Set replaces";
}

TEST_F(ModuleTest, leafInitialization)
{
    // Any step can have an initialization, which runs once before it first runs, in the initialization generation.
    auto step = std::make_shared<Evolution::LambdaStep>("Increment", [](Evolution::EvolutionData& state) { (*state.get<int>(Counter))++; },
                                                        std::set<std::string>{Counter}, std::set<std::string>{Counter});
    uint64_t generationOfInitialization = 0;
    step->setInitialization(std::make_shared<Evolution::LambdaStep>("Create", [&generationOfInitialization](Evolution::EvolutionData& state) {
        generationOfInitialization = state.generation;
        state.set(Counter, std::make_shared<int>(10));
    }, std::set<std::string>{}, std::set<std::string>{Counter}));
    Evolution::Module evolutionModule("Root");
    evolutionModule.addSubModule(step);

    Evolution::EvolutionData& state = evolutionModule.evolve(2, 0);
    evolutionModule.runGenerations(1);
    EXPECT_EQ(*state.get<int>(Counter), 13);
    EXPECT_EQ(generationOfInitialization, Evolution::EvolutionData::initGeneration);
    EXPECT_NE(evolutionModule.summary().find("initialization:"), std::string::npos) << evolutionModule.summary();
    EXPECT_EQ(state.generation, 3U) << "The generation is the one running after the initialization";
}

TEST_F(ModuleTest, moduleOwnsTheProblemAndTheRepresentation)
{
    auto ownedProblem = std::make_shared<Evaluation::ReinforcementProblem>(le, 3);
    std::weak_ptr<Evaluation::ReinforcementProblem> weakProblem = ownedProblem;

    auto ownedRep = std::make_shared<Representations::LGP>(le->getInputDimensions(), Data::DataType::type<double>({8}), set, Data::DataType::type<double>({3}), 10, 20);
    ownedRep->addOutputFunction(std::make_unique<Dimensions::ActivationFunctions::ArgMax<double>>(ownedRep->getDimensionFlow().getOutputDimension()));
    std::weak_ptr<Representations::LGP> weakRep = ownedRep;

    std::shared_ptr<Evolution::Module> evolutionModule = [&]() {
        Evolution::BasicEvolutionConfig evolution;
        evolution.representation = std::move(ownedRep);
        evolution.problem = std::move(ownedProblem);
        evolution.sizePopulation = 10;
        evolution.nbOffspring = 10;
        evolution.logOutput = nullptr;
        return evolution.buildModule();
    }();
    EXPECT_FALSE(weakProblem.expired()) << "The data of the module keep the problem alive";
    Evolution::EvolutionData& state = evolutionModule->evolve(2, 0);
    EXPECT_EQ(constIndividuals(*state.get<std::vector<std::shared_ptr<Individual>>>(Evolution::BasicEvolutionConfig::Names().population)).size(), 10U);

    // The components and the individuals are data of the module: nothing keeps them once it is gone.
    evolutionModule.reset();
    EXPECT_TRUE(weakProblem.expired());
    EXPECT_TRUE(weakRep.expired());
}

TEST_F(ModuleTest, stepsAreMovable)
{
    Evolution::Module moved = std::move(*makeSteps(20, 20, 2));
    Evolution::EvolutionData& state = moved.evolve(5, 4);

    auto reference = makeSteps(20, 20, 2);
    Evolution::EvolutionData& referenceState = reference->evolve(5, 4);
    EXPECT_EQ(state.rng.uniformSample<uint64_t>(0, UINT64_MAX), referenceState.rng.uniformSample<uint64_t>(0, UINT64_MAX));
}

TEST_F(ModuleTest, reproduceStep)
{
    const Evolution::BasicEvolutionConfig::Names population(namesOf("P"));
    Evolution::EvolutionData state;
    initPopulation(state, population, 0);
    const auto offspring = population.offspring;

    Evolution::ReproduceStep step(population.population, replicatorKey, 25, offspring);
    EXPECT_EQ(step.name(), "Reproduce");
    EXPECT_EQ(step.requiredData(), (std::set<std::string>{population.population, replicatorKey}));
    EXPECT_EQ(step.providedData(), (std::set<std::string>{offspring}));
    ASSERT_NO_THROW(step.apply(state));

    // Offspring are new individuals copying the parents, which stay in the population.
    const std::vector<std::shared_ptr<const Individual>> parents = constIndividuals(*state.get<std::vector<std::shared_ptr<Individual>>>(population.population));
    const std::vector<std::shared_ptr<Individual>>& children = *state.get<std::vector<std::shared_ptr<Individual>>>(offspring);
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
    const Evolution::BasicEvolutionConfig::Names population(namesOf("P"));
    const auto offspring = population.offspring;
    const std::string strongMutator("StrongMutator");

    // The same offspring mutated by several threads.
    std::vector<std::vector<std::shared_ptr<Individual>>> mutated;
    for(size_t nbThreads: {2, 3}) {
        Evolution::EvolutionData state;
        initPopulation(state, population, 20);
        state.set(strongMutator, std::shared_ptr<Mutation::Mutator>(std::make_shared<Mutation::PointMutator>(0.5, 0.5, 0.5)));
        Evolution::MutateStep step(offspring, strongMutator, generatorKey, nbThreads);
        EXPECT_EQ(step.name(), "Mutate");
        EXPECT_EQ(step.getIndividuals(), offspring);
        EXPECT_EQ(step.getNbThreads(), nbThreads);
        EXPECT_EQ(step.requiredData(), (std::set<std::string>{offspring, strongMutator, generatorKey}));
        EXPECT_TRUE(step.providedData().empty()) << "Offspring are mutated in place";
        ASSERT_NO_THROW(step.apply(state));

        // Offspring are not copies of their parents anymore, and remain valid.
        const std::vector<std::shared_ptr<const Individual>> parents = constIndividuals(*state.get<std::vector<std::shared_ptr<Individual>>>(population.population));
        const std::vector<std::shared_ptr<Individual>>& children = *state.get<std::vector<std::shared_ptr<Individual>>>(offspring);
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
    const Evolution::BasicEvolutionConfig::Names population(namesOf("P"));
    Evolution::EvolutionData state;
    initPopulation(state, population, 5);
    const auto offspring = population.offspring;
    const auto metrics = population.trainingMetrics;

    Evolution::EvaluateStep step({population.population, offspring}, metrics, problem->getName(), metricTemplateKey, 2, Evaluation::Mode::VALIDATION, 3);
    EXPECT_EQ(step.name(), "Evaluate");
    EXPECT_EQ(step.requiredData(), (std::set<std::string>{population.population, offspring, problem->getName(), metricTemplateKey}));
    EXPECT_EQ(step.providedData(), (std::set<std::string>{metrics}));
    EXPECT_EQ(step.details(), "2 iterations, 3 thread(s)");
    ASSERT_NO_THROW(step.apply(state));

    // Metrics of the individuals of both lists, constant and mutable, indexed by ID.
    const std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>& results = *state.get<std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>>(metrics);
    EXPECT_EQ(results.size(), 15U);
    for(std::string& list: std::vector<std::string>{population.population, offspring}) {
        for(const std::shared_ptr<const Individual>& individual: constIndividuals(*state.get<std::vector<std::shared_ptr<Individual>>>(list))) {
            ASSERT_EQ(results.count(individual->getIndividualID()), 1U);
            EXPECT_FALSE(results.at(individual->getIndividualID())->getMetricValues<Metrics::RewardsMetric>(Metrics::RewardsMetric::staticHash()).empty());
        }
    }
    EXPECT_TRUE(state.isUpToDate(metrics));
}

TEST_F(ModuleTest, assignFitnessStep)
{
    const Evolution::BasicEvolutionConfig::Names population(namesOf("P"));
    Evolution::EvolutionData state;
    initPopulation(state, population, 0);
    evaluatePopulation(state, population);
    const auto metrics = population.trainingMetrics;
    const auto fitness = population.parentFitness;

    Evolution::AssignFitnessStep step(metrics, fitness, assignerKey);
    EXPECT_EQ(step.name(), "AssignFitness");
    EXPECT_EQ(step.getFitness(), fitness);
    EXPECT_EQ(step.requiredData(), (std::set<std::string>{metrics, assignerKey}));
    EXPECT_EQ(step.providedData(), (std::set<std::string>{fitness}));
    ASSERT_NO_THROW(step.apply(state));

    // One fitness per evaluated individual, and the best individual has the highest one.
    const std::map<uint64_t, double>& values = *state.get<std::map<uint64_t, double>>(fitness);
    EXPECT_EQ(values.size(), 10U);
    const std::shared_ptr<const Individual> best = Util::bestIndividual(constIndividuals(*state.get<std::vector<std::shared_ptr<Individual>>>(population.population)), *state.get<std::map<uint64_t, double>>(fitness));
    ASSERT_NE(best, nullptr);
    for(const auto& [id, value]: values) {
        EXPECT_LE(value, values.at(best->getIndividualID()));
    }
}

TEST_F(ModuleTest, exportStep)
{
    const Evolution::BasicEvolutionConfig::Names population(namesOf("P"));
    Evolution::EvolutionData state;
    initPopulation(state, population, 0);
    evaluatePopulation(state, population);
    const std::string path = (std::filesystem::temp_directory_path() / "gegelati_export_step").string();
    const auto fitness = population.trainingFitness;

    Evolution::ExportStep all(population.population, path + "_all", true);
    Evolution::ExportStep best(population.population, path + "_best", false, fitness);
    EXPECT_EQ(all.name(), "Export");
    EXPECT_EQ(all.requiredData(), (std::set<std::string>{population.population}));
    EXPECT_EQ(best.requiredData(), (std::set<std::string>{population.population, fitness}));
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
    // A step holds steps: they nest and can be repeated. A module only searches and edits its own steps, findRecursiveSubModule the nested ones too.
    Evolution::Module evolutionModule("Root");
    const std::string counter("Counter");
    auto increment = [counter](Evolution::EvolutionData& state) { (*state.get<int>(counter))++; };
    auto first = std::make_shared<Evolution::LambdaStep>("Increment", increment, std::set<std::string>{counter}, std::set<std::string>{counter});
    auto inner = std::make_shared<Evolution::Module>("Inner");
    inner->addSubModule(first);

    evolutionModule.setInitialization(std::make_shared<Evolution::LambdaStep>("Create", [counter](Evolution::EvolutionData& state) { state.set(counter, std::make_shared<int>(0)); },
                                                                   std::set<std::string>{}, std::set<std::string>{counter}));
    evolutionModule.addSubModule(inner);
    EXPECT_EQ(evolutionModule.size(), 1U);
    EXPECT_FALSE(evolutionModule.has("Increment")) << "Steps of nested modules are not steps of the module";
    EXPECT_THROW(evolutionModule.find("Increment"), std::runtime_error);
    EXPECT_EQ(evolutionModule.findRecursiveSubModule("Increment"), first);
    EXPECT_EQ(evolutionModule.findRecursiveSubModule("Unknown"), nullptr);

    // Steps of nested modules are edited from the module holding them.
    auto second = std::make_shared<Evolution::LambdaStep>("Second", increment, std::set<std::string>{counter}, std::set<std::string>{counter});
    EXPECT_THROW(evolutionModule.insertAfter("Increment", second), std::runtime_error);
    evolutionModule.find("Inner")->insertAfter("Increment", second);
    EXPECT_EQ(inner->size(), 2U);

    const std::string summary = evolutionModule.summary();
    EXPECT_NE(summary.find("initialization: Create"), std::string::npos) << summary;
    EXPECT_NE(summary.find(std::string("`- Inner") + std::string(1, 10) + "   |- Increment"), std::string::npos) << summary;
    EXPECT_NE(summary.find("   `- Second"), std::string::npos) << summary;
    EXPECT_NE(summary.find("requires: Counter | provides: Counter"), std::string::npos) << summary;

    Evolution::EvolutionData& state = evolutionModule.evolve(2, 1);
    EXPECT_EQ(*state.get<int>(counter), 2 * 2) << "Two generations, two steps";
    EXPECT_GE(inner->getLastDuration(), first->getLastDuration() + second->getLastDuration()) << "The container includes its steps";

    inner->remove("Increment");
    EXPECT_EQ(inner->size(), 1U);
}

TEST_F(ModuleTest, nestedStepsAreChecked)
{
    Evolution::Module evolutionModule("Root");
    const std::string counter("Counter");
    auto inner = std::make_shared<Evolution::Module>("Inner");
    inner->addSubModule(std::make_shared<Evolution::LambdaStep>("NeedsCounter", [](Evolution::EvolutionData&) {}, std::set<std::string>{counter}));
    evolutionModule.addSubModule(inner);
    try {
        evolutionModule.evolve(1, 1);
        FAIL() << "The nested step requires data that nothing provides";
    } catch(const std::runtime_error& error) {
        EXPECT_NE(std::string(error.what()).find("NeedsCounter"), std::string::npos) << error.what();
    }

    // Data provided by the initialization are available in the nested step.
    evolutionModule.setInitialization(std::make_shared<Evolution::LambdaStep>("Create", [counter](Evolution::EvolutionData& state) { state.set(counter, std::make_shared<int>(0)); },
                                                                   std::set<std::string>{}, std::set<std::string>{counter}));
    EXPECT_NO_THROW(evolutionModule.evolve(1, 1));
}

TEST_F(ModuleTest, presetRunEveryOtherGeneration)
{
    // The generation of the second population runs every second generation of the first one.
    auto evolutionModule = std::make_shared<Evolution::Module>("Evolution");
    Evolution::BasicEvolutionConfig first = makeEvolution(10, 10, 2);
    first.names = namesOf("First");
    Evolution::BasicEvolutionConfig second = makeEvolution(6, 6, 2);
    second.names = namesOf("Second");

    evolutionModule->addSubModule(first.buildModule());
    const std::shared_ptr<Evolution::Module> slow = second.buildModule();
    slow->setPeriod(2);
    size_t nbRuns = 0;
    slow->addSubModule(std::make_shared<Evolution::LambdaStep>("CountRuns", [&nbRuns](Evolution::EvolutionData&) { nbRuns++; }));
    evolutionModule->addSubModule(slow);

    Evolution::EvolutionData& state = evolutionModule->evolve(4, 4);
    EXPECT_EQ(nbRuns, 2U) << "Generations 0 and 2";
    EXPECT_EQ(state.get<std::vector<std::shared_ptr<Individual>>>(Evolution::BasicEvolutionConfig::Names(namesOf("First")).population)->size(), 10U);
    EXPECT_EQ(state.get<std::vector<std::shared_ptr<Individual>>>(Evolution::BasicEvolutionConfig::Names(namesOf("Second")).population)->size(), 6U);
    EXPECT_NE(evolutionModule->summary().find("Second (every 2 generations)"), std::string::npos) << evolutionModule->summary();
}

TEST_F(ModuleTest, initializeCreatesTheDataAndEvolveContinues)
{
    // evolve(n, seed) starts the evolution and returns the data of the module, runGenerations(n) goes on with them.
    auto module = makeSteps(20, 20, 2);
    Evolution::EvolutionData& data = module->evolve(3, 4);
    EXPECT_EQ(&data, &module->getData()) << "The data of the evolution are the ones of the module";
    EXPECT_EQ(data.nbGenerationsDone, 3U);
    EXPECT_EQ(constIndividuals(*data.get<std::vector<std::shared_ptr<Individual>>>(Evolution::BasicEvolutionConfig::Names().population)).size(), 20U);
    module->runGenerations(2);
    EXPECT_EQ(data.nbGenerationsDone, 5U);

    // The same evolution in one go, and another module is a new evolution, which does not depend on the first one.
    auto referenceModule = makeSteps(20, 20, 2);
    Evolution::EvolutionData& reference = referenceModule->evolve(5, 4);
    EXPECT_EQ(data.rng.uniformSample<uint64_t>(0, UINT64_MAX), reference.rng.uniformSample<uint64_t>(0, UINT64_MAX));
    auto otherModule = makeSteps(20, 20, 2);
    EXPECT_EQ(otherModule->evolve(1, 4).nbGenerationsDone, 1U);
    EXPECT_EQ(data.nbGenerationsDone, 5U) << "The data of the first evolution are untouched";
}

TEST_F(ModuleTest, componentsDescribeThemselves)
{
    // The core steps show these descriptions in the summary.
    EXPECT_EQ(Selection::RandomSelector(true).getName(), "RandomSelector");
    EXPECT_EQ(Selection::TournamentSelector(3).getName(), "TournamentSelector");
    EXPECT_EQ(Mutation::PointMutator(0.2, 0.5, 0.5).getName(), "PointMutator");
    EXPECT_EQ(Mutation::NodeTransferMutation(0.1, 0.7, 0.7).getName(), "NodeTransferMutation");
    EXPECT_EQ(Reproduction::Replicator().getName(), "Replicator");
    EXPECT_EQ(FitnessAssignment::DefaultAssigner(0).getName(), "DefaultAssigner");
    EXPECT_EQ(FitnessAssignment::NoveltySearchAssigner(15, std::make_shared<Descriptors::AverageVectorDescriptor>(0), 0).getName(), "NoveltySearchAssigner");
    EXPECT_EQ(Descriptors::AverageVectorDescriptor(0).getName(), "AverageVectorDescriptor");

    // The steps only show their numeric parameters.
    EXPECT_EQ(makeSteps(10, 10, 2)->find("Mutation")->details(), "1 thread(s)");
    EXPECT_EQ(makeSteps(10, 10, 2)->find("SurvivorSelection")->details(), "10 selected");
}

TEST_F(ModuleTest, componentsCanBeNamed)
{
    // The name identifies a component in the data, and tells apart two components of the same class in a summary.
    auto descriptor = std::make_shared<Descriptors::AverageVectorDescriptor>(0, "OutputsOnTheArchive");
    EXPECT_EQ(descriptor->getName(), "OutputsOnTheArchive");
    EXPECT_EQ(Descriptors::ScalarListDescriptor(0, "PerImage").getName(), "PerImage");

    EXPECT_EQ(Selection::RandomSelector(true, "RandomParents").getName(), "RandomParents");
    EXPECT_EQ(Selection::TournamentSelector(3, true, "Small").getName(), "Small");
    EXPECT_EQ(Selection::TournamentSelector(8, true, "Big").getName(), "Big");
    EXPECT_EQ(Selection::TournamentSelector(8, false).getName(), "TournamentSelector");
    EXPECT_EQ(Selection::TruncationSelector("Best").getName(), "Best");
    EXPECT_EQ(Mutation::PointMutator(0.2, 0.5, 0.5, "Gentle").getName(), "Gentle");
    EXPECT_EQ(Mutation::NodeTransferMutation(0.1, 0.7, 0.7, "Transfer").getName(), "Transfer");
    EXPECT_EQ(Reproduction::Replicator("Copy").getName(), "Copy");
    EXPECT_EQ(FitnessAssignment::DefaultAssigner(0, "Reward").getName(), "Reward");
    EXPECT_EQ(FitnessAssignment::ClassificationF1Assigner("F1").getName(), "F1");
    EXPECT_EQ(FitnessAssignment::NoveltySearchAssigner(15, descriptor, 0, "Novelty").getName(), "Novelty");
    EXPECT_EQ(FitnessAssignment::NoveltySearchAssigner(15, descriptor, 0).getName(), "NoveltySearchAssigner");

    std::shared_ptr<Evaluation::ReinforcementEnvironment> game = std::make_shared<StickGameWithOpponent>();
    Evaluation::ReinforcementProblem named(game, 0, "Sticks");
    EXPECT_EQ(named.getName(), "Sticks");
    ASSERT_TRUE(named.isCopyable());
    EXPECT_EQ(named.cloneUniquePtr()->getName(), "Sticks") << "A clone keeps the name";
    EXPECT_EQ(Evaluation::ReinforcementProblem(game).getName(), "ReinforcementProblem");

    // The details have the parameters, the name alone is the default.
    EXPECT_EQ(Selection::TournamentSelector(3, true, "Small").getDetails(), "Small(size 3, with replacement)");
    EXPECT_EQ(Selection::TournamentSelector(8, false).getDetails(), "TournamentSelector(size 8, without replacement)");
    EXPECT_EQ(Selection::RandomSelector(false, "Rnd").getDetails(), "Rnd(without replacement)");
    EXPECT_EQ(Selection::TruncationSelector("Best").getDetails(), "Best") << "No parameter, so the details are the name";
    EXPECT_EQ(Mutation::PointMutator(0.2, 0.5, 0.5, "Gentle").getDetails(), "Gentle(mutate 0.2, add 0.5, remove 0.5)");
    EXPECT_EQ(Mutation::NodeTransferMutation(0.1, 0.7, 0.7).getDetails(), "NodeTransferMutation(mutate 0.1, add 0.7, remove 0.7)");
    EXPECT_EQ(Reproduction::Replicator("Copy").getDetails(), "Copy");
    EXPECT_EQ(FitnessAssignment::NoveltySearchAssigner(15, descriptor, 0, "Novelty").getDetails(), "Novelty(15 neighbors, OutputsOnTheArchive)");
    std::ostringstream streamed;
    streamed << Selection::TournamentSelector(3, true, "Small") << " / " << *descriptor;
    EXPECT_EQ(streamed.str(), "Small / OutputsOnTheArchive") << "Streaming an object writes its name, not its details";

    // The steps read the components by their names.
    Evolution::SelectStep small({"A"}, "fitness", "Small", 2, "B");
    EXPECT_EQ(small.requiredData().count("Small"), 1U);
}

TEST_F(ModuleTest, evolveRunsTheInitializations)
{
    auto module = makeSteps(10, 10, 2);
    const Evolution::EvolutionData& data = module->getData();
    size_t nbInitializations = 0;
    module->getInitialization()->addSubModule(std::make_shared<Evolution::LambdaStep>("Count", [&nbInitializations](Evolution::EvolutionData&) { nbInitializations++; }));

    // Generations cannot run before an evolution is started.
    EXPECT_THROW(module->runGenerations(1), std::runtime_error);
    EXPECT_EQ(nbInitializations, 0U);

    // An evolution runs the initialization, with the default seed if none is given.
    module->evolve(0);
    EXPECT_EQ(nbInitializations, 1U);
    EXPECT_EQ(data.nbGenerationsDone, 0U) << "The initialization is not a generation";
    module->runGenerations(1);
    EXPECT_EQ(nbInitializations, 1U) << "Running more generations does not initialize again";
    EXPECT_EQ(data.nbGenerationsDone, 1U);

    // A module added meanwhile is initialized by the next evolution only.
    size_t nbAddedInitializations = 0;
    auto added = std::make_shared<Evolution::LambdaStep>("Added", [](Evolution::EvolutionData&) {});
    added->setInitialization(std::make_shared<Evolution::LambdaStep>("InitAdded", [&nbAddedInitializations](Evolution::EvolutionData&) { nbAddedInitializations++; }));
    module->addSubModule(added);
    module->runGenerations(1);
    EXPECT_EQ(nbAddedInitializations, 0U);
    EXPECT_EQ(data.nbGenerationsDone, 2U);

    // A new evolution runs every initialization again, replacing the data of the previous one.
    const std::shared_ptr<const std::vector<std::shared_ptr<Individual>>> previous = data.get<std::vector<std::shared_ptr<Individual>>>(Evolution::BasicEvolutionConfig::Names().population);
    module->evolve(1, 4);
    EXPECT_EQ(nbInitializations, 2U);
    EXPECT_EQ(nbAddedInitializations, 1U);
    EXPECT_EQ(data.nbGenerationsDone, 1U);
    EXPECT_NE(data.get<std::vector<std::shared_ptr<Individual>>>(Evolution::BasicEvolutionConfig::Names().population), previous);
}

TEST_F(ModuleTest, stopwatchIsInTheData)
{
    auto module = makeSteps(10, 10, 2);
    Evolution::EvolutionData& data = module->evolve(1, 4);
    const double elapsed = data.stopwatch.elapsed();
    EXPECT_GT(elapsed, 0.0);
    EXPECT_GE(elapsed, module->find("Mutation")->getLastDuration()) << "The durations of the steps are read from the stopwatch of the data";
    module->evolve(0, 4);
    EXPECT_LT(data.stopwatch.elapsed(), elapsed) << "A new evolution restarts the stopwatch";
}

TEST_F(ModuleTest, summaryShowsTheDataOfTheEvolution)
{
    // Validation every second generation: its fitness is older than the others after two generations.
    auto module = makeSteps(10, 10, 2, 2, 1, 2);
    const Evolution::EvolutionData& data = module->getData();

    // Before the initialization, the data only hold the components.
    std::string summary = module->summary(data);
    EXPECT_NE(summary.find("initialization [pending]"), std::string::npos) << summary;
    EXPECT_NE(summary.find(problem->getName()), std::string::npos) << summary;
    EXPECT_EQ(data.summary().find("Parents"), std::string::npos) << data.summary();

    module->evolve(3, 4);
    summary = module->summary(data);
    EXPECT_NE(summary.find("initialization [done]"), std::string::npos) << summary;
    EXPECT_EQ(summary.find("[pending]"), std::string::npos) << summary;
    EXPECT_NE(summary.find("Data after 3 generation(s):"), std::string::npos) << summary;
    EXPECT_NE(summary.find("Parents"), std::string::npos) << summary;
    EXPECT_NE(summary.find("10 elements"), std::string::npos) << summary;
    EXPECT_NE(summary.find("up to date"), std::string::npos) << summary;
    EXPECT_NE(summary.find("stored at generation 2"), std::string::npos) << "The validation fitness is older: " << summary;

    // Numbers are shown with their value.
    module->getData().set(Counter, std::make_shared<int>(7));
    EXPECT_NE(data.summary().find("Counter"), std::string::npos);
    EXPECT_NE(data.summary().find("7"), std::string::npos);
}

TEST_F(ModuleTest, namesAreSettings)
{
    // Every name is a setting, changed before the build. Data and steps are found under their new names.
    Evolution::BasicEvolutionConfig evolution = makeEvolution(10, 10, 2);
    EXPECT_EQ(Evolution::BasicEvolutionConfig::Names(evolution.names).parents, "Parents") << "The names have defaults";
    evolution.names = namesOf("Pool");
    evolution.names.parents = "MatingPool";
    evolution.names.mutationStep = "Variation";
    evolution.names.initialEvaluationStep = "FirstEvaluation";

    auto module = evolution.buildModule();
    EXPECT_EQ(module->name(), "Pool");
    EXPECT_TRUE(module->has("Variation"));
    EXPECT_FALSE(module->has("Mutation"));
    EXPECT_TRUE(module->getInitialization()->has("FirstEvaluation"));

    Evolution::EvolutionData& data = module->evolve(2, 0);
    const Evolution::BasicEvolutionConfig::Names keys(evolution.names);
    EXPECT_EQ(keys.parents, "MatingPool");
    EXPECT_EQ(keys.offspring, "Pool.offspring");
    EXPECT_EQ(data.get<std::vector<std::shared_ptr<Individual>>>(keys.parents)->size(), 10U);
    EXPECT_EQ(constIndividuals(*data.get<std::vector<std::shared_ptr<Individual>>>(keys.population)).size(), 10U);
    EXPECT_FALSE(data.has("Parents"));
}

TEST_F(ModuleTest, generatorIsAComponent)
{
    // The generator is a component stored in the data of the module by the preset, from its settings.
    Evolution::BasicEvolutionConfig evolution = makeEvolution(10, 10, 2);
    const Evolution::BasicEvolutionConfig::Names keys(evolution.names);
    auto module = evolution.buildModule();
    EXPECT_TRUE(module->getData().has(keys.generator)) << "Stored before the evolution";
    module->evolve(2, 0);

    // A generator given in the settings is the one found in the data.
    evolution.generator = std::make_shared<GraphBased::GenotypeGenerator>(lgpRep->getGenotypeConstraint());
    auto custom = evolution.buildModule();
    EXPECT_EQ(custom->getData().get<GraphBased::GenotypeGenerator>(keys.generator), evolution.generator);

    // Updating the generator is adding a step: the steps creating and mutating the population read it from the data.
    auto updated = makeEvolution(10, 10, 2).buildModule();
    size_t nbCreations = 0;
    const std::shared_ptr<const Representations::LGP> representation = lgpRep;
    const std::string generator = keys.generator;
    updated->insertBefore("Mutation", std::make_shared<Evolution::LambdaStep>("UpdateGenerator", [representation, generator, &nbCreations](Evolution::EvolutionData& data) {
        nbCreations++;
        data.set(generator, std::make_shared<GraphBased::GenotypeGenerator>(representation->getGenotypeConstraint()));
    }, std::set<std::string>{}, std::set<std::string>{generator}));
    updated->evolve(3, 0);
    EXPECT_EQ(nbCreations, 3U);
}

TEST_F(ModuleTest, loggerCustomColumns)
{
    Evolution::BasicEvolutionConfig evolution = makeEvolution(20, 20, 2);
    auto evolutionModule = evolution.buildModule();
    const Evolution::BasicEvolutionConfig::Names population;

    // The time of the evaluation alone (the log adds the time of the generation), the training fitness, and the fitness of the 3 best individuals.
    const auto evaluation = evolutionModule->find("TrainingEvaluation");
    std::ostringstream output;
    Evolution::BasicLogConfig logConfig;
    logConfig.output = &output;
    std::shared_ptr<Evolution::LogModule> log = logConfig.buildModule();
    log->insertBefore("Pop_Size", std::make_shared<Evolution::LogColumnStep>("T_Eval", [evaluation](const Evolution::EvolutionData&) { return evaluation->getLastDuration(); }, 4));
    for(size_t rank = 0; rank < 3; rank++) {
        log->insertBefore("T_Gen", std::make_shared<Evolution::LogColumnStep>("Best" + std::to_string(rank + 1), [population, rank](const Evolution::EvolutionData& data) {
            std::vector<double> values;
            const auto fitnessOfIndividuals = data.get<std::map<uint64_t, double>>(population.trainingFitness);
            for(const auto& individual: constIndividuals(*data.get<std::vector<std::shared_ptr<Individual>>>(population.population))) {
                const auto found = fitnessOfIndividuals->find(individual->getIndividualID());
                if(found != fitnessOfIndividuals->end()) {
                    values.push_back(found->second);
                }
            }
            std::sort(values.begin(), values.end(), std::greater<double>());
            return (rank < values.size()) ? values.at(rank) : std::nan("");
        }));
    }
    evolutionModule->addSubModule(log);
    evolutionModule->evolve(3, 4);

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
    double timeEvaluation, size, min, average, max, best1, best2, best3;
    fields >> generation >> timeEvaluation >> size >> min >> average >> max >> best1 >> best2 >> best3;
    EXPECT_EQ(size, 20.0);
    EXPECT_EQ(generation, "3");
    EXPECT_GE(best1, best2);
    EXPECT_GE(best2, best3);
    EXPECT_EQ(best1, max);
}
