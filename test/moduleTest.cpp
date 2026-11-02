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
                                            size_t nbValidationIterations = 0, uint64_t seed = 0, size_t nbThreads = 1, size_t validationPeriod = 1)
    {
        Evolution::BasicEvolution evolution;
        evolution.representation = lgpRep;
        evolution.problem = problem;
        evolution.sizePopulation = sizePopulation;
        evolution.nbOffspring = nbOffspring;
        evolution.nbTrainingIterations = nbTrainingIterations;
        evolution.nbValidationIterations = nbValidationIterations;
        evolution.validationPeriod = validationPeriod;
        evolution.seed = seed;
        evolution.nbThreads = nbThreads;
        return evolution;
    }

    // Module of the basic evolution, see makeEvolution.
    Evolution::Module makeModule(size_t sizePopulation, size_t nbOffspring, size_t nbTrainingIterations,
                                         size_t nbValidationIterations = 0, uint64_t seed = 0, size_t nbThreads = 1, size_t validationPeriod = 1)
    {
        return makeEvolution(sizePopulation, nbOffspring, nbTrainingIterations, nbValidationIterations, seed, nbThreads, validationPeriod).buildModule();
    }

    // Generator of the genotypes of the representation.
    std::shared_ptr<const GraphBased::GenotypeGenerator> generator() const
    {
        return std::shared_ptr<const GraphBased::GenotypeGenerator>(lgpRep->getGenotypeGenerator().cloneUniquePtr());
    }

    // Fill a state with the random individuals of a population and its offspring, ready for the first generation.
    void initPopulation(Evolution::EvolutionState& state, const std::shared_ptr<const Evolution::Population>& population, size_t nbOffspring) const
    {
        state.reset(0);
        Evolution::InitPopulationStep(population, std::make_shared<Mutation::PointMutator>(0.2, 0.5, 0.5), generator()).apply(state);
        state.generation = 0;
        Evolution::ReproduceStep(population->individuals(), std::make_shared<Reproduction::Replicator>(), nbOffspring, population->data("offspring")).apply(state);
    }

    // Evaluate the individuals of a population and compute their fitness, see initPopulation.
    void evaluatePopulation(Evolution::EvolutionState& state, const Evolution::Population& population) const
    {
        auto metricTemplate = std::make_shared<Metrics::MetricMapTemplate>(std::make_unique<Metrics::RewardsMetric>());
        Evolution::EvaluateStep({population.individuals()}, population.data("metrics"), problem, std::make_shared<Evaluation::Evaluator>(), metricTemplate, 2)
            .apply(state);
        Evolution::AssignFitnessStep(population.data("metrics"), population.data("fitness"), std::make_shared<FitnessAssignment::DefaultAssigner>(),
                                     Metrics::RewardsMetric::staticHash())
            .apply(state);
    }
};

TEST_F(ModuleTest, classicFlow)
{
    Evolution::Module module = makeModule(100, 100, 3, 0, 4, 10);
    module.evolve(20);

    const Evolution::Population& population = *module.getPopulations().at(0);
    const Evolution::EvolutionState& state = module.getState();
    EXPECT_EQ(state.nbGenerationsDone, 20U);
    const std::vector<std::shared_ptr<const Individual>> individuals = module.getState().getIndividuals(population.individuals());
    EXPECT_EQ(individuals.size(), 100U);
    EXPECT_EQ(state.get<std::vector<std::shared_ptr<const Individual>>>(population.data(defaults.parentsName))->size(), 100U);
    EXPECT_EQ(state.get<std::vector<std::shared_ptr<Individual>>>(population.data(defaults.offspringName))->size(), 100U);

    // The fitness and metrics are the ones of the last evaluation, of the population and of the offspring.
    const std::map<uint64_t, double>& fitness = *state.get<std::map<uint64_t, double>>(population.data(defaults.trainingFitnessName));
    EXPECT_EQ(fitness.size(), 200U);
    EXPECT_EQ((state.get<std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>>(population.data(defaults.trainingMetricsName))->size()), 200U);

    // The best individual is in the population, with the highest fitness of the population.
    std::shared_ptr<const Individual> best = module.getState().bestIndividual(population.individuals(), population.data(defaults.trainingFitnessName));
    ASSERT_NE(best, nullptr);
    EXPECT_NE(std::find(individuals.begin(), individuals.end(), best), individuals.end());
    for(const std::shared_ptr<const Individual>& individual: individuals) {
        EXPECT_LE(fitness.at(individual->getIndividualID()), fitness.at(best->getIndividualID()));
    }

    ASSERT_EQ(1704165046166301501U, module.getState().rng.uniformSample<uint64_t>(0, UINT64_MAX)) << "determinism";
}

TEST_F(ModuleTest, summary)
{
    Evolution::Module module = makeModule(10, 10, 3, 2);
    const std::string summary = module.summary();
    for(const std::string& text: {"Population (10 individuals)", "InitPopulation", "InitialEvaluation", "ParentSelection (10 from Population to Population.parents)",
                                  "Reproduction", "Mutation", "TrainingEvaluation (training, 3 iterations)",
                                  "SurvivorSelection (10 distinct from Population, Population.offspring to Population)",
                                  "ValidationEvaluation (validation, 2 iterations)", "ValidationFitness (Population.validationMetrics to Population.validationFitness)"}) {
        EXPECT_NE(summary.find(text), std::string::npos) << text << "\n" << summary;
    }
}

TEST_F(ModuleTest, settingsAreChecked)
{
    Evolution::BasicEvolution evolution;
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
    Evolution::Module module = makeModule(20, 20, 2, 0, 4);
    const Evolution::Population& population = *module.getPopulations().at(0);

    // A second mutation after the first one, then a custom step counting the generations.
    size_t nbCalls = 0;
    auto secondMutation = std::make_shared<Evolution::MutateStep>(population.data(defaults.offspringName), std::make_shared<Mutation::PointMutator>(0.5, 0.5, 0.5),
                                                                  generator(), 1, "SecondMutation");
    module.generationSteps()
        .insertAfter(module.generationSteps().find("Mutation"), secondMutation)
        .insertAfter(secondMutation, std::make_shared<Evolution::LambdaStep>("Count", [&nbCalls](Evolution::EvolutionState&) { nbCalls++; }));
    ASSERT_EQ(module.generationSteps().size(), 8);

    module.evolve(5);
    EXPECT_EQ(nbCalls, 5);
    for(const auto& individual: module.getState().getIndividuals(population.individuals())) {
        EXPECT_TRUE(individual->isValid());
    }

    // Time of each step is recorded.
    EXPECT_EQ(module.getState().stepTimes.size(), 8);
    EXPECT_EQ(module.getState().stepTimes.at(3).first, "SecondMutation");
}

TEST_F(ModuleTest, editPipeline)
{
    Evolution::Module module = makeModule(10, 10, 2);
    Evolution::Pipeline& steps = module.generationSteps();
    auto unknown = std::make_shared<Evolution::LambdaStep>("Unknown", [](Evolution::EvolutionState&) {});
    const std::shared_ptr<Evolution::MutateStep> mutation = steps.find<Evolution::MutateStep>("Mutation");

    EXPECT_TRUE(steps.has("Mutation"));
    EXPECT_FALSE(steps.has("Unknown"));
    EXPECT_FALSE(steps.has("InitPopulation")) << "Only in the initialization pipeline";
    EXPECT_THROW(steps.find("Unknown"), std::runtime_error);
    EXPECT_THROW(steps.find<Evolution::SelectStep>("Mutation"), std::runtime_error) << "Not a selection";
    EXPECT_THROW(steps.insertAfter(unknown, std::make_shared<Evolution::LambdaStep>("Step", [](Evolution::EvolutionState&) {})), std::runtime_error);
    EXPECT_THROW(steps.insertBefore(mutation, nullptr), std::runtime_error);

    // Steps are found by address, so names can be shared.
    auto before = std::make_shared<Evolution::LambdaStep>("Mutation", [](Evolution::EvolutionState&) {});
    steps.insertBefore(mutation, before);
    EXPECT_EQ(steps.getSteps().at(2), before);
    EXPECT_EQ(steps.getSteps().at(3), mutation);
    auto replaced = std::make_shared<Evolution::LambdaStep>("Replaced", [](Evolution::EvolutionState&) {});
    steps.replace(before, replaced);
    EXPECT_EQ(steps.getSteps().at(2), replaced);
    steps.remove(replaced);
    EXPECT_EQ(steps.getSteps().at(2), mutation);
    EXPECT_THROW(steps.remove(replaced), std::runtime_error);
}

TEST_F(ModuleTest, misorderedPipelineIsRejected)
{
    Evolution::Module module = makeModule(10, 10, 2);
    module.generationSteps().remove(module.generationSteps().find("ParentSelection"));

    // The reproduction now has no parents.
    EXPECT_THROW(module.evolve(1), std::runtime_error);
}

TEST_F(ModuleTest, resume)
{
    Evolution::Module reference = makeModule(20, 20, 2, 0, 4);
    reference.evolve(5);

    Evolution::Module resumed = makeModule(20, 20, 2, 0, 4);
    EXPECT_THROW(resumed.resume(1), std::runtime_error);
    resumed.evolve(3);
    resumed.resume(2);

    EXPECT_EQ(resumed.getState().nbGenerationsDone, 5);
    EXPECT_EQ(reference.getState().rng.uniformSample<uint64_t>(0, UINT64_MAX), resumed.getState().rng.uniformSample<uint64_t>(0, UINT64_MAX));
}

TEST_F(ModuleTest, stopCondition)
{
    Evolution::Module module = makeModule(10, 10, 2);
    module.addStopCondition([](const Evolution::EvolutionState& state) { return state.generation >= 2; });
    module.evolve(10);
    EXPECT_EQ(module.getState().nbGenerationsDone, 3);
}

TEST_F(ModuleTest, validationAndLogger)
{
    Evolution::Module module = makeModule(10, 10, 2, 2, 4, 1, 2);
    const Evolution::Population& population = *module.getPopulations().at(0);
    const std::string validationFitness = module.generationSteps().find<Evolution::AssignFitnessStep>("ValidationFitness")->getFitness();
    EXPECT_EQ(validationFitness, population.data(defaults.validationFitnessName));

    std::ostringstream output;
    module.addStep(std::make_shared<Evolution::LogStep>(output, population.individuals(), std::vector<std::pair<std::string, std::string>>{
        {"Fit", population.data(defaults.trainingFitnessName)}, {"Val", validationFitness}}));
    module.evolve(4);

    const std::string log = output.str();
    EXPECT_NE(log.find("Val_Avg"), std::string::npos);

    // Validation ran on generations 0 and 2 only.
    const Evolution::EvolutionState& state = module.getState();
    EXPECT_FALSE((state.get<std::map<uint64_t, double>>(validationFitness)->empty()));
    EXPECT_FALSE(state.isUpToDate(validationFitness));
    EXPECT_TRUE(state.isUpToDate(population.data(defaults.trainingFitnessName)));

    // Rows of generations 1 and 3 show "-" instead of the older validation values.
    std::istringstream lines(log);
    size_t nbRowsWithoutValidation = 0;
    for(std::string line; std::getline(lines, line);) {
        nbRowsWithoutValidation += (line.find(" -") != std::string::npos) ? 1 : 0;
    }
    EXPECT_EQ(nbRowsWithoutValidation, 2U);
    EXPECT_NE(module.summary().find("(validation, 2 iterations, every 2 generations)"), std::string::npos) << module.summary();
    EXPECT_THROW(module.generationSteps().find("ValidationEvaluation")->setPeriod(0), std::runtime_error);
}

TEST_F(ModuleTest, validationWithItsOwnProblemAndAssigner)
{
    auto validationProblem = std::make_shared<Evaluation::ReinforcementProblem>(le, 5);
    Evolution::BasicEvolution evolution = makeEvolution(10, 10, 2, 3);
    evolution.validationProblem = validationProblem;
    evolution.validationAssigner = std::make_shared<FitnessAssignment::DefaultAssigner>();
    Evolution::Module module = evolution.buildModule();
    const Evolution::Population& population = *module.getPopulations().at(0);

    const std::shared_ptr<Evolution::EvaluateStep> validation = module.generationSteps().find<Evolution::EvaluateStep>("ValidationEvaluation");
    EXPECT_EQ(validation->getProblem(), validationProblem);
    EXPECT_EQ(validation->getMode(), Evaluation::Mode::VALIDATION);
    EXPECT_EQ(module.generationSteps().find<Evolution::EvaluateStep>("TrainingEvaluation")->getProblem().get(), problem.get());
    module.evolve(2);

    EXPECT_EQ((module.getState().get<std::map<uint64_t, double>>(population.data(defaults.validationFitnessName))->size()), 10U);
    EXPECT_NE(module.getState().bestIndividual(population.individuals(), population.data(defaults.validationFitnessName)), nullptr);
}

TEST_F(ModuleTest, parentsWithTheirOwnFitness)
{
    Evolution::BasicEvolution evolution = makeEvolution(10, 10, 2);
    evolution.parentAssigner = std::make_shared<FitnessAssignment::DefaultAssigner>();
    Evolution::Module module = evolution.buildModule();
    const Evolution::Population& population = *module.getPopulations().at(0);

    EXPECT_EQ(module.generationSteps().find("ParentSelection")->requiredData().count(population.data(defaults.parentFitnessName)), 1U);
    EXPECT_TRUE(module.initSteps().has("ParentFitness")) << "Parents of the first generation need it";
    EXPECT_NE(module.summary().find("ParentFitness (Population.trainingMetrics to Population.parentFitness)"), std::string::npos);

    module.evolve(3);
    EXPECT_TRUE(module.getState().isUpToDate(population.data(defaults.parentFitnessName)));

    // Without the parent fitness, the parents cannot be selected.
    module.initSteps().remove(module.initSteps().find("ParentFitness"));
    module.generationSteps().remove(module.generationSteps().find("ParentFitness"));
    EXPECT_THROW(module.evolve(1), std::runtime_error);
}

TEST_F(ModuleTest, selectionIsGeneric)
{
    Evolution::EvolutionState state;
    auto a = std::make_shared<Individual>(*lgpRep);
    auto b = std::make_shared<Individual>(*lgpRep);
    auto c = std::make_shared<Individual>(*lgpRep);
    auto other = std::make_shared<Individual>(*lgpRep);
    const std::string listA("A");
    const std::string listB("B");
    const std::string fitness("Fitness");
    const std::string selected("Selected");
    state.set(listA, std::make_shared<std::vector<std::shared_ptr<const Individual>>>(std::vector<std::shared_ptr<const Individual>>{c, a}));
    state.set(listB, std::make_shared<std::vector<std::shared_ptr<Individual>>>(std::vector<std::shared_ptr<Individual>>{b}));
    state.set(fitness, std::make_shared<std::map<uint64_t, double>>(std::map<uint64_t, double>{
        {a->getIndividualID(), 3.0}, {b->getIndividualID(), 2.0}, {c->getIndividualID(), 1.0}, {other->getIndividualID(), 10.0}}));

    // The best candidates are kept once each, ordered by ID: the fitness of non candidates is ignored.
    Evolution::SelectStep distinct({listA, listB}, fitness, std::make_shared<Selection::TruncationSelector>(), 2, selected, true, "Best");
    EXPECT_EQ(distinct.name(), "Best");
    EXPECT_EQ(distinct.requiredData(), (std::set<std::string>{listA, listB, fitness}));
    distinct.apply(state);
    EXPECT_EQ(*state.get<std::vector<std::shared_ptr<const Individual>>>(selected), (std::vector<std::shared_ptr<const Individual>>{a, b}));

    // A selection can replace one of its candidate lists, and keep duplicates.
    Evolution::SelectStep withDuplicates({listA}, fitness, std::make_shared<Selection::RandomSelector>(true), 5, listA);
    EXPECT_EQ(withDuplicates.name(), "Select");
    withDuplicates.apply(state);
    EXPECT_EQ(state.get<std::vector<std::shared_ptr<const Individual>>>(listA)->size(), 5U);
    EXPECT_THROW(Evolution::SelectStep({}, fitness, std::make_shared<Selection::TruncationSelector>(), 2, selected), std::runtime_error);
}

TEST_F(ModuleTest, loggerColumns)
{
    Evolution::Module module = makeModule(10, 10, 2);
    const Evolution::Population& population = *module.getPopulations().at(0);
    const std::string offspring = population.data(defaults.offspringName);
    std::ostringstream output;
    module.addStep(std::make_shared<Evolution::LogStep>(output, population.individuals(),
        std::vector<std::pair<std::string, std::string>>{{"Fit", population.data(defaults.trainingFitnessName)}},
        std::vector<Evolution::LogStep::Column>{{"Nb_Offspring", [offspring](const Evolution::EvolutionState& state) {
            const auto children = state.getIfAny<std::vector<std::shared_ptr<Individual>>>(offspring);
            return (children == nullptr) ? size_t{0} : children->size();
        }}}));
    module.evolve(2);
    module.evolve(1);

    // Only the given fitness, the custom column instead of the population size, and one header per evolution.
    const std::string log = output.str();
    EXPECT_NE(log.find("Fit_Avg"), std::string::npos);
    EXPECT_EQ(log.find("Val_Avg"), std::string::npos);
    EXPECT_EQ(log.find("Pop_Size"), std::string::npos);
    size_t nbHeaders = 0;
    for(size_t pos = log.find("Nb_Offspring"); pos != std::string::npos; pos = log.find("Nb_Offspring", pos + 1)) {
        nbHeaders++;
    }
    EXPECT_EQ(nbHeaders, 2U);
}

TEST_F(ModuleTest, stepsOfThePreset)
{
    Evolution::Module module = makeModule(10, 10, 3);
    const Evolution::Population& population = *module.getPopulations().at(0);
    const auto evaluation = module.generationSteps().find<Evolution::EvaluateStep>("TrainingEvaluation");
    EXPECT_EQ(evaluation->getNbIterations(), 3U);
    EXPECT_EQ(evaluation->getEvaluated(), (std::vector<std::string>{population.individuals(), population.data(defaults.offspringName)}));
    EXPECT_EQ(module.initSteps().find<Evolution::EvaluateStep>("InitialEvaluation")->getEvaluated(), (std::vector<std::string>{population.individuals()}));
    EXPECT_EQ(evaluation->getMetrics(), population.data(defaults.trainingMetricsName));
    EXPECT_TRUE(module.initSteps().has("TrainingFitness"));
    EXPECT_TRUE(module.generationSteps().has("TrainingFitness"));
    EXPECT_FALSE(module.generationSteps().has("ValidationEvaluation"));
    EXPECT_EQ(module.initSteps().find<Evolution::InitPopulationStep>("InitPopulation")->getPopulation(), module.getPopulations().at(0));
}

TEST_F(ModuleTest, parentsComeFromPopulation)
{
    Evolution::Module module = makeModule(20, 20, 2, 0, 4);
    const std::string individuals = module.getPopulations().at(0)->individuals();
    const std::string parents = module.getPopulations().at(0)->data(defaults.parentsName);

    // Individuals discarded by the survival must not be selected as parents anymore.
    size_t nbParentsChecked = 0;
    module.generationSteps().insertAfter(module.generationSteps().find("ParentSelection"), std::make_shared<Evolution::LambdaStep>("CheckParents",
        [&nbParentsChecked, individuals, parents](Evolution::EvolutionState& state) {
            const std::vector<std::shared_ptr<const Individual>>& current = *state.get<std::vector<std::shared_ptr<const Individual>>>(individuals);
            for(const auto& parent: *state.get<std::vector<std::shared_ptr<const Individual>>>(parents)) {
                EXPECT_NE(std::find(current.begin(), current.end(), parent), current.end());
                nbParentsChecked++;
            }
        }));
    module.evolve(10);
    EXPECT_EQ(nbParentsChecked, 200U);
}

namespace {
    // Custom name of the tests.
    const std::string Counter{"Counter"};
}

TEST_F(ModuleTest, stateEntries)
{
    Evolution::EvolutionState state;
    EXPECT_FALSE(state.has(Counter));
    EXPECT_THROW(state.get<int>(Counter), std::runtime_error);

    state.set(Counter, std::make_shared<int>(3));
    ASSERT_TRUE(state.has(Counter));
    EXPECT_EQ(*state.get<int>(Counter), 3);
    EXPECT_EQ(*state.get<int>("Counter"), 3) << "Data are identified by their name";
    EXPECT_THROW(state.get<double>(Counter), std::runtime_error) << "The entry is not a double";
    EXPECT_EQ(state.getIfAny<int>("Other"), nullptr);

    const Evolution::EvolutionState& constState = state;
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

    // Individuals are read from both kinds of lists, and an absent list is empty.
    auto individual = std::make_shared<Individual>(*lgpRep);
    state.set("Constant", std::make_shared<std::vector<std::shared_ptr<const Individual>>>(std::vector<std::shared_ptr<const Individual>>{individual}));
    state.set("Mutable", std::make_shared<std::vector<std::shared_ptr<Individual>>>(std::vector<std::shared_ptr<Individual>>{individual}));
    EXPECT_EQ(state.getIndividuals("Constant"), (std::vector<std::shared_ptr<const Individual>>{individual}));
    EXPECT_EQ(state.getIndividuals("Mutable"), (std::vector<std::shared_ptr<const Individual>>{individual}));
    EXPECT_TRUE(state.getIndividuals("Absent").empty());
    state.set(Counter, std::make_shared<int>(4));
    EXPECT_THROW(state.getIndividuals(Counter), std::runtime_error) << "The counter is not a list of individuals";

    state.reset(0);
    EXPECT_FALSE(state.has(Counter)) << "A new evolution starts without entries";
}

TEST_F(ModuleTest, customDataAreChecked)
{
    Evolution::Module module = makeModule(10, 10, 2);
    module.addStep(std::make_shared<Evolution::LambdaStep>("Increment",
        [](Evolution::EvolutionState& state) { (*state.get<int>(Counter))++; }, std::set<std::string>{Counter}, std::set<std::string>{Counter}));
    EXPECT_THROW(module.evolve(1), std::runtime_error) << "No step creates the counter";

    module.addInitStep(std::make_shared<Evolution::LambdaStep>("CreateCounter",
        [](Evolution::EvolutionState& state) { state.set(Counter, std::make_shared<int>(0)); }, std::set<std::string>{}, std::set<std::string>{Counter}));
    module.evolve(3);
    EXPECT_EQ(*module.getState().get<int>(Counter), 3);
    EXPECT_NE(module.summary().find("provides: Counter"), std::string::npos);
}

TEST_F(ModuleTest, mutationComponentsAreRequired)
{
    auto population = std::make_shared<const Evolution::Population>(lgpRep, 10, "P");
    const std::string offspring = population->data(defaults.offspringName);
    auto mutator = std::make_shared<Mutation::PointMutator>(0.5, 0.5, 0.5);

    // Components are fixed objects, only the mutated individuals are data of the state.
    Evolution::MutateStep mutate(offspring, mutator, generator());
    EXPECT_EQ(mutate.requiredData(), (std::set<std::string>{offspring}));
    EXPECT_THROW(Evolution::MutateStep(offspring, nullptr, generator()), std::runtime_error);
    EXPECT_THROW(Evolution::MutateStep(offspring, mutator, nullptr), std::runtime_error);

    // The population is created with the generator of its representation if none is given.
    Evolution::InitPopulationStep init(population, mutator);
    EXPECT_TRUE(init.requiredData().empty());
    EXPECT_EQ(init.providedData(), (std::set<std::string>{population->individuals()}));
    EXPECT_THROW(Evolution::InitPopulationStep(population, nullptr), std::runtime_error);
    EXPECT_THROW(Evolution::InitPopulationStep(nullptr, mutator), std::runtime_error);

    Evolution::EvolutionState state;
    state.reset(0);
    ASSERT_NO_THROW(init.apply(state));
    EXPECT_EQ(state.getIndividuals(population->individuals()).size(), 10U);
}

TEST_F(ModuleTest, twoPopulations)
{
    // A module built from scratch, evolving two populations with the same kind of steps.
    Evolution::Module module(4);
    auto mutator = std::make_shared<Mutation::PointMutator>(0.2, 0.5, 0.5);
    auto metricTemplate = std::make_shared<Metrics::MetricMapTemplate>(std::make_unique<Metrics::RewardsMetric>());
    auto addClassicSteps = [&](const std::shared_ptr<const Evolution::Population>& population, size_t nbOffspring) {
        const std::string individuals = population->individuals();
        const std::string parents = population->data(defaults.parentsName);
        const std::string offspring = population->data(defaults.offspringName);
        const std::string metrics = population->data("metrics");
        const std::string fitness = population->data("fitness");
        auto assign = std::make_shared<Evolution::AssignFitnessStep>(metrics, fitness, std::make_shared<FitnessAssignment::DefaultAssigner>(),
                                                                      Metrics::RewardsMetric::staticHash());

        module.addInitStep(std::make_shared<Evolution::InitPopulationStep>(population, mutator));
        module.addInitStep(std::make_shared<Evolution::EvaluateStep>(std::vector<std::string>{individuals}, metrics, problem,
                                                                     std::make_shared<Evaluation::Evaluator>(), metricTemplate, 2));
        module.addInitStep(assign);

        module.addStep(std::make_shared<Evolution::SelectStep>(std::vector<std::string>{individuals}, fitness,
                                                               std::make_shared<Selection::RandomSelector>(true), nbOffspring, parents));
        module.addStep(std::make_shared<Evolution::ReproduceStep>(parents, std::make_shared<Reproduction::Replicator>(), nbOffspring, offspring));
        module.addStep(std::make_shared<Evolution::MutateStep>(offspring, mutator, generator()));
        module.addStep(std::make_shared<Evolution::EvaluateStep>(std::vector<std::string>{individuals, offspring}, metrics, problem,
                                                                 std::make_shared<Evaluation::Evaluator>(), metricTemplate, 2));
        module.addStep(assign);
        module.addStep(std::make_shared<Evolution::SelectStep>(std::vector<std::string>{individuals, offspring}, fitness,
                                                               std::make_shared<Selection::TruncationSelector>(), population->getSize(), individuals, true));
        return assign;
    };

    auto populationA = module.addPopulation(lgpRep, 10, "A");
    auto populationB = module.addPopulation(lgpRep, 6, "B");
    EXPECT_THROW(module.addPopulation(lgpRep, 6, "A"), std::runtime_error) << "Names are unique";
    addClassicSteps(populationA, 10);
    auto assignB = addClassicSteps(populationB, 4);

    const std::string summary = module.summary();
    for(const std::string& text: {"A (10 individuals)", "B (6 individuals)", "A.fitness", "B.fitness", "B.parents"}) {
        EXPECT_NE(summary.find(text), std::string::npos) << text;
    }
    module.evolve(3);

    const std::vector<std::shared_ptr<const Individual>> individualsA = module.getState().getIndividuals(populationA->individuals());
    const std::vector<std::shared_ptr<const Individual>> individualsB = module.getState().getIndividuals(populationB->individuals());
    EXPECT_EQ(individualsA.size(), 10U);
    EXPECT_EQ(individualsB.size(), 6U);
    for(const auto& individual: individualsB) {
        EXPECT_EQ(std::find(individualsA.begin(), individualsA.end(), individual), individualsA.end()) << "The populations are separate";
    }
    EXPECT_NE(module.getState().bestIndividual(populationB->individuals(), populationB->data("fitness")), nullptr);

    // Steps of the second population are checked like the others.
    module.initSteps().remove(assignB);
    module.generationSteps().remove(assignB);
    EXPECT_THROW(module.evolve(1), std::runtime_error) << "No fitness for the second population";
}

TEST_F(ModuleTest, presetsShareAModule)
{
    // Two basic evolutions in the same module, with different population names.
    Evolution::Module module(4);
    Evolution::BasicEvolution first = makeEvolution(10, 10, 2);
    first.populationName = "First";
    first.addTo(module);
    Evolution::BasicEvolution second = makeEvolution(6, 6, 2);
    second.populationName = "Second";
    second.addTo(module);
    EXPECT_THROW(second.addTo(module), std::runtime_error) << "The population name is taken";

    module.evolve(2);
    EXPECT_EQ(module.getState().getIndividuals("First").size(), 10U);
    EXPECT_EQ(module.getState().getIndividuals("Second").size(), 6U);
}

TEST_F(ModuleTest, moduleOwnsTheProblemAndThePopulations)
{
    auto ownedProblem = std::make_shared<Evaluation::ReinforcementProblem>(le, 3);
    std::weak_ptr<Evaluation::ReinforcementProblem> weakProblem = ownedProblem;

    auto ownedRep = std::make_shared<Representations::LGP>(le->getInputDimensions(), Data::DataType::type<double>({8}), set, Data::DataType::type<double>({3}), 10, 20);
    ownedRep->addOutputFunction(std::make_unique<Dimensions::ActivationFunctions::ArgMax<double>>(ownedRep->getDimensionFlow().getOutputDimension()));
    std::weak_ptr<Representations::LGP> weakRep = ownedRep;

    Evolution::Module module = [&]() {
        Evolution::BasicEvolution evolution;
        evolution.representation = std::move(ownedRep);
        evolution.problem = std::move(ownedProblem);
        evolution.sizePopulation = 10;
        evolution.nbOffspring = 10;
        return evolution.buildModule();
    }();
    EXPECT_FALSE(weakProblem.expired()) << "The steps keep the problem alive";
    module.evolve(2);

    // The individuals only reference the representation, which their population keeps alive.
    module.initSteps() = Evolution::Pipeline();
    module.generationSteps() = Evolution::Pipeline();
    EXPECT_FALSE(weakRep.expired()) << "The module keeps the representation of its population alive";
    EXPECT_EQ(module.getState().getIndividuals(module.getPopulations().at(0)->individuals()).size(), 10U);
}

TEST_F(ModuleTest, moduleIsMovable)
{
    Evolution::Module built = makeModule(20, 20, 2, 0, 4);
    Evolution::Module moved = std::move(built);
    moved.evolve(5);

    Evolution::Module reference = makeModule(20, 20, 2, 0, 4);
    reference.evolve(5);
    EXPECT_EQ(moved.getState().rng.uniformSample<uint64_t>(0, UINT64_MAX), reference.getState().rng.uniformSample<uint64_t>(0, UINT64_MAX));
}

TEST_F(ModuleTest, reproduceStep)
{
    auto population = std::make_shared<const Evolution::Population>(lgpRep, 10, "P");
    Evolution::EvolutionState state;
    initPopulation(state, population, 0);
    const std::string offspring = population->data("offspring");

    Evolution::ReproduceStep step(population->individuals(), std::make_shared<Reproduction::Replicator>(), 25, offspring);
    EXPECT_EQ(step.name(), "Reproduce");
    EXPECT_EQ(step.requiredData(), (std::set<std::string>{population->individuals()}));
    EXPECT_EQ(step.providedData(), (std::set<std::string>{offspring}));
    ASSERT_NO_THROW(step.apply(state));

    // Offspring are new individuals copying the parents, which stay in the population.
    const std::vector<std::shared_ptr<const Individual>> parents = state.getIndividuals(population->individuals());
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
    auto population = std::make_shared<const Evolution::Population>(lgpRep, 10, "P");
    const std::string offspring = population->data("offspring");
    auto mutator = std::make_shared<Mutation::PointMutator>(0.5, 0.5, 0.5);

    // The same offspring mutated by several threads.
    std::vector<std::vector<std::shared_ptr<Individual>>> mutated;
    for(size_t nbThreads: {2, 3}) {
        Evolution::EvolutionState state;
        initPopulation(state, population, 20);
        Evolution::MutateStep step(offspring, mutator, generator(), nbThreads);
        EXPECT_EQ(step.name(), "Mutate");
        EXPECT_EQ(step.getIndividuals(), offspring);
        EXPECT_EQ(step.getNbThreads(), nbThreads);
        EXPECT_EQ(step.requiredData(), (std::set<std::string>{offspring}));
        EXPECT_TRUE(step.providedData().empty()) << "Offspring are mutated in place";
        ASSERT_NO_THROW(step.apply(state));

        // Offspring are not copies of their parents anymore, and remain valid.
        const std::vector<std::shared_ptr<const Individual>> parents = state.getIndividuals(population->individuals());
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
    auto population = std::make_shared<const Evolution::Population>(lgpRep, 10, "P");
    Evolution::EvolutionState state;
    initPopulation(state, population, 5);
    const std::string offspring = population->data("offspring");
    const std::string metrics = population->data("metrics");
    auto evaluator = std::make_shared<Evaluation::Evaluator>();
    auto metricTemplate = std::make_shared<Metrics::MetricMapTemplate>(std::make_unique<Metrics::RewardsMetric>());

    EXPECT_THROW(Evolution::EvaluateStep({offspring}, metrics, nullptr, evaluator, metricTemplate, 2), std::runtime_error);
    Evolution::EvaluateStep step({population->individuals(), offspring}, metrics, problem, evaluator, metricTemplate, 2, Evaluation::Mode::VALIDATION);
    EXPECT_EQ(step.name(), "Evaluate");
    EXPECT_EQ(step.requiredData(), (std::set<std::string>{population->individuals(), offspring}));
    EXPECT_EQ(step.providedData(), (std::set<std::string>{metrics}));
    EXPECT_EQ(step.details(), "validation, 2 iterations");
    ASSERT_NO_THROW(step.apply(state));

    // Metrics of the individuals of both lists, constant and mutable, indexed by ID.
    const std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>& results = *state.get<std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>>(metrics);
    EXPECT_EQ(results.size(), 15U);
    for(const std::string& list: {population->individuals(), offspring}) {
        for(const std::shared_ptr<const Individual>& individual: state.getIndividuals(list)) {
            ASSERT_EQ(results.count(individual->getIndividualID()), 1U);
            EXPECT_FALSE(results.at(individual->getIndividualID())->getMetricValues<Metrics::RewardsMetric>(Metrics::RewardsMetric::staticHash()).empty());
        }
    }
    EXPECT_TRUE(state.isUpToDate(metrics));
}

TEST_F(ModuleTest, assignFitnessStep)
{
    auto population = std::make_shared<const Evolution::Population>(lgpRep, 10, "P");
    Evolution::EvolutionState state;
    initPopulation(state, population, 0);
    evaluatePopulation(state, *population);
    const std::string metrics = population->data("metrics");
    const std::string fitness = population->data("otherFitness");

    Evolution::AssignFitnessStep step(metrics, fitness, std::make_shared<FitnessAssignment::DefaultAssigner>(), Metrics::RewardsMetric::staticHash());
    EXPECT_EQ(step.name(), "AssignFitness");
    EXPECT_EQ(step.getFitness(), fitness);
    EXPECT_EQ(step.requiredData(), (std::set<std::string>{metrics}));
    EXPECT_EQ(step.providedData(), (std::set<std::string>{fitness}));
    ASSERT_NO_THROW(step.apply(state));

    // One fitness per evaluated individual, and the best individual has the highest one.
    const std::map<uint64_t, double>& values = *state.get<std::map<uint64_t, double>>(fitness);
    EXPECT_EQ(values.size(), 10U);
    const std::shared_ptr<const Individual> best = state.bestIndividual(population->individuals(), fitness);
    ASSERT_NE(best, nullptr);
    for(const auto& [id, value]: values) {
        EXPECT_LE(value, values.at(best->getIndividualID()));
    }
}

TEST_F(ModuleTest, exportStep)
{
    auto population = std::make_shared<const Evolution::Population>(lgpRep, 10, "P");
    Evolution::EvolutionState state;
    initPopulation(state, population, 0);
    evaluatePopulation(state, *population);
    const std::string path = (std::filesystem::temp_directory_path() / "gegelati_export_step").string();
    const std::string fitness = population->data("fitness");

    Evolution::ExportStep all(population->individuals(), path + "_all", true);
    Evolution::ExportStep best(population->individuals(), path + "_best", false, fitness);
    EXPECT_EQ(all.name(), "Export");
    EXPECT_EQ(all.details(), path + "_all<generation>.dot");
    EXPECT_EQ(all.requiredData(), (std::set<std::string>{population->individuals()}));
    EXPECT_EQ(best.requiredData(), (std::set<std::string>{population->individuals(), fitness}));
    EXPECT_TRUE(all.providedData().empty());

    // Nothing is exported during the initialization.
    std::filesystem::remove(path + "_all3.dot");
    std::filesystem::remove(path + "_best.dot");
    state.generation = Evolution::EvolutionState::initGeneration;
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
