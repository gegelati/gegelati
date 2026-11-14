#include <gtest/gtest.h>
#include "evolutionTestHelpers.h"
#include "util/individualUtils.h"
#include <algorithm>

#include "evolution/basicEvolutionConfig.h"
#include "learn/populationNames.h"
#include "evolution/module.h"
#include "evolution/originTPGEvolutionConfig.h"
#include "evolution/steps/tpgSteps.h"
#include "evaluation/predictionProblem.h"
#include "metrics/inputMetric.h"
#include "mutation/pointMutator.h"

#include "learn/dataSetExamples.h"
#include "learn/fakeRepresentation.h"
#include "util/counterReset.h"

namespace {
    // Node bidding with the member for a destination which is either an action or a vertex.
    std::unique_ptr<GraphBased::GPNode> makeNode(const std::shared_ptr<const Individual>& member, Data::DataValue destination)
    {
        std::vector<Data::DataValue> values;
        values.push_back(Data::DataValue::scalar<std::shared_ptr<const Individual>>(member));
        values.push_back(std::move(destination));
        return std::make_unique<GraphBased::GPNode>(values);
    }

    std::shared_ptr<Individual> makeVertex(std::shared_ptr<const Representations::Representation> rep, const std::shared_ptr<const Individual>& member,
                                           const std::vector<std::shared_ptr<const Individual>>& destinations)
    {
        auto group = std::make_unique<GraphBased::NodeGroup>();
        for(const std::shared_ptr<const Individual>& destination: destinations) {
            group->addNode(makeNode(member, Data::DataValue::scalar<std::shared_ptr<const Individual>>(destination)));
        }
        return std::make_shared<Individual>(rep, GraphBased::Genotype::singleGroupGenotype(std::move(group)));
    }

    std::shared_ptr<Individual> makeLeaf(std::shared_ptr<const Representations::Representation> rep, const std::shared_ptr<const Individual>& member)
    {
        auto group = std::make_unique<GraphBased::NodeGroup>();
        group->addNode(makeNode(member, Data::DataValue::scalar<size_t>(0)));
        return std::make_shared<Individual>(rep, GraphBased::Genotype::singleGroupGenotype(std::move(group)));
    }

    std::vector<uint64_t> idsOf(const std::vector<std::shared_ptr<const Individual>>& individuals)
    {
        std::vector<uint64_t> ids;
        for(const std::shared_ptr<const Individual>& individual: individuals) {
            ids.push_back(individual->getIndividualID());
        }
        return ids;
    }
}

class TPGStepsTest : public ::testing::Test
{
  protected:
    std::shared_ptr<Representations::FakeRepresentation> rep = std::make_shared<Representations::FakeRepresentation>();
    // Representation of the members, another one than the one of the vertices (see Evolution::getMembersOf).
    std::shared_ptr<Representations::FakeRepresentation> memberRep = std::make_shared<Representations::FakeRepresentation>();
    std::shared_ptr<const Individual> member;
    // Keys of the population of roots.
    const Evolution::BasicEvolutionConfig::Names roots{namesOf("Roots")};

    // Keys of data of the steps.
    const std::string archiveKey{"archive"};
    const std::string cacheKey{"cache"};
    const std::string protectedKey{"protectedVertices"};
    const std::string aggregatorsKey{"aggregators"};

    virtual void SetUp()
    {
        CounterReset::counterReset();
        member = std::make_shared<Individual>(memberRep);
    }

    // Replace the individuals of the population of roots.
    void setRoots(Evolution::EvolutionData& state, const std::vector<std::shared_ptr<Individual>>& individuals) const
    {
        state.set(this->roots.population, std::make_shared<std::vector<std::shared_ptr<Individual>>>(individuals));
    }

    // Number of vertices protected by a ProtectAggregatedStep.
    size_t nbProtected(const Evolution::EvolutionData& state) const
    {
        return state.get<std::map<uint64_t, std::shared_ptr<Individual>>>(this->protectedKey)->size();
    }

    // Initialize the data of a ProtectAggregatedStep, as the initialization of an evolution does.
    void initProtection(Evolution::ProtectAggregatedStep& step, Evolution::EvolutionData& state) const
    {
        const uint64_t generation = state.generation;
        state.generation = Evolution::EvolutionData::initGeneration;
        state.erase(protectedKey);
    state.erase(aggregatorsKey);
    step.apply(state);
        state.generation = generation;
    }

    // Individuals of the population of roots.
    std::vector<std::shared_ptr<const Individual>> rootsOf(const Evolution::EvolutionData& state) const
    {
        return constIndividuals(*state.get<std::vector<std::shared_ptr<Individual>>>(this->roots.population));
    }

    // State where the roots have a "fitness", and empty "metrics".
    Evolution::EvolutionData makeState(const std::vector<std::shared_ptr<Individual>>& individuals) const
    {
        Evolution::EvolutionData state;
        state.generation = 1;
        this->setRoots(state, individuals);
        auto fitness = std::make_shared<std::map<uint64_t, double>>();
        for(const std::shared_ptr<const Individual>& individual: individuals) {
            (*fitness)[individual->getIndividualID()] = 1.0;
        }
        state.set(this->roots.trainingFitness, fitness);
        state.set(this->roots.trainingMetrics, std::make_shared<std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>>());
        return state;
    }
};

TEST_F(TPGStepsTest, graphHelpers)
{
    std::shared_ptr<Individual> leaf1 = makeLeaf(rep, member);
    std::shared_ptr<Individual> leaf2 = makeLeaf(rep, member);
    std::shared_ptr<Individual> vertexA = makeVertex(rep, member, {leaf2, leaf1, leaf2});
    std::shared_ptr<Individual> vertexB = makeVertex(rep, member, {vertexA});

    // Direct references, without duplicates and ordered by ID.
    EXPECT_EQ(idsOf(Evolution::getSubVertices(*vertexA)), (std::vector<uint64_t>{leaf1->getIndividualID(), leaf2->getIndividualID()}));
    EXPECT_TRUE(Evolution::getSubVertices(*leaf1).empty());

    // Everything reachable, roots included.
    std::vector<std::shared_ptr<const Individual>> all = Evolution::getAllVertices({vertexB});
    EXPECT_EQ(idsOf(all), (std::vector<uint64_t>{leaf1->getIndividualID(), leaf2->getIndividualID(), vertexA->getIndividualID(), vertexB->getIndividualID()}));
    EXPECT_EQ(Evolution::getAllVertices({vertexB, vertexA, leaf1}).size(), 4U);

    // Members are the ones used by the nodes.
    std::vector<std::shared_ptr<const Individual>> members = Evolution::getMembersOf(all, *memberRep);
    ASSERT_EQ(members.size(), 1U);
    EXPECT_EQ(members.at(0), member);
}

TEST_F(TPGStepsTest, sharedVerticesByHeight)
{
    std::shared_ptr<Individual> leaf = makeLeaf(rep, member);
    std::shared_ptr<Individual> vertexA = makeVertex(rep, member, {leaf});
    std::shared_ptr<Individual> vertexB = makeVertex(rep, member, {leaf});
    std::shared_ptr<Individual> vertexC = makeVertex(rep, member, {vertexA});
    using Layers = std::vector<std::vector<uint64_t>>;
    const auto layersOf = [](const std::vector<std::shared_ptr<const Individual>>& evaluated) {
        Layers ids;
        for(const auto& layer: Evolution::CacheVertexOutputsStep::getSharedVerticesByHeight(evaluated)) {
            ids.push_back(idsOf(layer));
        }
        return ids;
    };

    // Vertices executed once per sample are not cached.
    EXPECT_TRUE(layersOf({vertexC}).empty());
    EXPECT_TRUE(layersOf({}).empty());

    // The leaf is reached by two vertices.
    EXPECT_EQ(layersOf({vertexA, vertexB}), (Layers{{leaf->getIndividualID()}}));

    // A is reached by C and evaluated itself: it comes after the leaf it points to, which A and B share.
    EXPECT_EQ(layersOf({vertexA, vertexB, vertexC}), (Layers{{leaf->getIndividualID()}, {vertexA->getIndividualID()}}));

    // A vertex referenced by a single vertex is cached only if it is also evaluated, and it is never in the layer of its sub-vertices.
    EXPECT_EQ(layersOf({vertexC, vertexA}), (Layers{{}, {vertexA->getIndividualID()}}));
}

TEST_F(TPGStepsTest, protectAggregated)
{
    std::shared_ptr<Individual> leaf1 = makeLeaf(rep, member);
    std::shared_ptr<Individual> leaf2 = makeLeaf(rep, member);
    std::shared_ptr<Individual> vertexA = makeVertex(rep, member, {leaf1});

    Evolution::ProtectAggregatedStep step{roots.population, protectedKey, aggregatorsKey};
    EXPECT_EQ(step.providedData(), (std::set<std::string>{roots.population, protectedKey, aggregatorsKey}));
    Evolution::EvolutionData state = makeState({leaf1, leaf2, vertexA});
    EXPECT_THROW(step.apply(state), std::runtime_error) << "The protection must be initialized first";
    initProtection(step, state);
    ASSERT_NO_THROW(step.apply(state));

    // The leaf referenced by A is not a root anymore.
    EXPECT_EQ(idsOf(rootsOf(state)), (std::vector<uint64_t>{leaf2->getIndividualID(), vertexA->getIndividualID()}));
    EXPECT_EQ(nbProtected(state), 1U);

    // Nothing changes while the aggregator is alive.
    ASSERT_NO_THROW(step.apply(state));
    EXPECT_EQ(rootsOf(state).size(), 2U);
    EXPECT_EQ(nbProtected(state), 1U);

    // The aggregator disappears: the leaf comes back, ordered by ID.
    setRoots(state, {leaf2});
    ASSERT_NO_THROW(step.apply(state));
    EXPECT_EQ(idsOf(rootsOf(state)), (std::vector<uint64_t>{leaf1->getIndividualID(), leaf2->getIndividualID()}));
    EXPECT_EQ(nbProtected(state), 0U);

    // A new evolution forgets the protected vertices.
    setRoots(state, {leaf1, vertexA});
    step.apply(state);
    EXPECT_EQ(nbProtected(state), 1U);
    state.generation = Evolution::EvolutionData::initGeneration;
    state.erase(protectedKey);
    state.erase(aggregatorsKey);
    step.apply(state);
    EXPECT_EQ(nbProtected(state), 0U);

    // The protected vertices are data of the state, the step keeps nothing: another step object continues the evolution.
    state.generation = 1;
    setRoots(state, {leaf1, vertexA});
    step.apply(state);
    Evolution::ProtectAggregatedStep otherStep{roots.population, protectedKey, aggregatorsKey};
    EXPECT_EQ(nbProtected(state), 1U);
    setRoots(state, {leaf2});
    otherStep.apply(state);
    EXPECT_EQ(nbProtected(state), 0U) << "The leaf comes back when its aggregator disappears";
    EXPECT_EQ(idsOf(rootsOf(state)), (std::vector<uint64_t>{leaf1->getIndividualID(), leaf2->getIndividualID()}));
}

TEST_F(TPGStepsTest, protectAggregatedChains)
{
    std::shared_ptr<Individual> leaf = makeLeaf(rep, member);
    std::shared_ptr<Individual> vertexA = makeVertex(rep, member, {leaf});
    std::shared_ptr<Individual> vertexB = makeVertex(rep, member, {vertexA});

    Evolution::ProtectAggregatedStep step{roots.population, protectedKey, aggregatorsKey};
    Evolution::EvolutionData state = makeState({leaf, vertexA, vertexB});
    initProtection(step, state);
    step.apply(state);

    // Only the top of the chain is a root, but the whole chain stays protected while it lives.
    EXPECT_EQ(idsOf(rootsOf(state)), (std::vector<uint64_t>{vertexB->getIndividualID()}));
    EXPECT_EQ(nbProtected(state), 2U);
    step.apply(state);
    EXPECT_EQ(nbProtected(state), 2U);
    EXPECT_EQ(idsOf(rootsOf(state)), (std::vector<uint64_t>{vertexB->getIndividualID()}));

    // When the top dies, A comes back as a root with its own protected leaf.
    setRoots(state, {});
    step.apply(state);
    EXPECT_EQ(idsOf(rootsOf(state)), (std::vector<uint64_t>{vertexA->getIndividualID()}));
    EXPECT_EQ(nbProtected(state), 1U);
}

TEST_F(TPGStepsTest, archiveIsADataOfThePopulation)
{
    Evolution::CreateArchiveStep create(archiveKey, {Dimensions::Requirement(Data::DataType::type<int>({2}))});
    Evolution::UpdateArchiveStep update(roots.population, roots.trainingMetrics, archiveKey, memberRep->getName(), 10, false);
    EXPECT_EQ(create.providedData(), (std::set<std::string>{archiveKey}));
    EXPECT_EQ(update.requiredData().count(archiveKey), 1U);

    Evolution::EvolutionData state;
    state.set(memberRep->getName(), std::shared_ptr<const Representations::Representation>(memberRep));
    state.generation = Evolution::EvolutionData::initGeneration;
    EXPECT_THROW(update.apply(state), std::runtime_error) << "The archive must be created first";

    ASSERT_NO_THROW(create.apply(state));
    const std::shared_ptr<Evaluation::DataSet> archive = state.get<Evaluation::DataSet>(archiveKey);
    ASSERT_NE(archive, nullptr);
    EXPECT_THROW(update.apply(state), std::runtime_error) << "The update samples the metrics of the population";
    setRoots(state, {});
    state.set(roots.trainingMetrics, std::make_shared<std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>>());

    // Nothing to sample from an empty population.
    ASSERT_NO_THROW(update.apply(state));
    EXPECT_EQ(archive->size(), 0U);

    // Later generations of a fixed archive only update answers, which is nothing for an empty archive.
    Evolution::UpdateArchiveStep fixed(roots.population, roots.trainingMetrics, archiveKey, memberRep->getName(), 10, true);
    state.generation = 3;
    ASSERT_NO_THROW(fixed.apply(state));
    EXPECT_EQ(state.get<Evaluation::DataSet>(archiveKey), archive);
    EXPECT_EQ(archive->size(), 0U);

    // A new evolution creates the archive again, replacing the previous one.
    ASSERT_NO_THROW(create.apply(state));
    EXPECT_NE(state.get<Evaluation::DataSet>(archiveKey), archive);
}

TEST_F(TPGStepsTest, cacheMemberOutputs)
{
    Evaluation::DataSet dataSet = createSmallDataSet();
    auto problem = std::make_shared<Evaluation::PredictionProblem>(dataSet);

    // Members output a constant, whatever the sample.
    auto memberRep = std::make_shared<Representations::FakeRepresentation>(problem->getInputDimensions(), Dimensions::Requirement(Data::DataType::type<double>()));
    std::shared_ptr<const Individual> cachedMember = std::make_shared<Individual>(memberRep,
        GraphBased::Genotype::singleNodeGenotype(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0})));
    ASSERT_TRUE(cachedMember->isValid());
    std::shared_ptr<Individual> vertex = makeLeaf(rep, cachedMember);

    // The members of the roots and of the offspring are cached.
    const auto offspring = roots.offspring;
    Evolution::CacheMemberOutputsStep step({roots.population, offspring}, cacheKey, problem->getName(), memberRep->getName(), 2);
    Evolution::EvolutionData state = makeState({vertex});
    state.set(offspring, std::make_shared<std::vector<std::shared_ptr<Individual>>>());
    EXPECT_EQ(step.requiredData(), (std::set<std::string>{roots.population, offspring, problem->getName(), memberRep->getName()}));
    EXPECT_EQ(step.providedData(), (std::set<std::string>{cacheKey}));
    EXPECT_THROW(step.apply(state), std::runtime_error) << "The problem is a component of the data";
    state.set(problem->getName(), std::shared_ptr<Evaluation::Problem>(problem));
    state.set(memberRep->getName(), std::shared_ptr<const Representations::Representation>(memberRep));

    EXPECT_EQ(cachedMember->getNbCachedOutputs(), 0U);
    ASSERT_NO_THROW(step.apply(state));

    // One output by sample of the evaluation, indexed by the address of the sample.
    EXPECT_EQ(cachedMember->getNbCachedOutputs(), 2U);
    size_t nbFound = 0;
    for(size_t idx = 0; idx < dataSet.size(); idx++) {
        if(const Data::DataValue* output = cachedMember->getCachedOutput(dataSet.getInputsAt(idx).at(0).getPtr())) {
            EXPECT_EQ(output->getScalar<double>(), 1.0);
            nbFound++;
        }
    }
    EXPECT_EQ(nbFound, 2U) << "Outputs are cached for the samples of the dataset";
    EXPECT_EQ(cachedMember->getCachedOutput(nullptr), nullptr);

    // The cached members are a data of the population, and the next run clears their outputs.
    ASSERT_TRUE(state.has(cacheKey));
    EXPECT_EQ(state.get<std::vector<std::shared_ptr<const Individual>>>(cacheKey)->size(), 1U);
    setRoots(state, {});
    ASSERT_NO_THROW(step.apply(state));
    EXPECT_EQ(cachedMember->getNbCachedOutputs(), 0U);
    EXPECT_TRUE(state.get<std::vector<std::shared_ptr<const Individual>>>(cacheKey)->empty());
}

TEST_F(TPGStepsTest, cachedOutputsAreClearedWithTheGenotype)
{
    auto memberRep = std::make_shared<Representations::FakeRepresentation>(std::vector<Dimensions::Requirement>{Dimensions::Requirement(Data::DataType::type<double>())},
                                                                          Dimensions::Requirement(Data::DataType::type<double>()));
    auto member = std::make_shared<Individual>(memberRep, GraphBased::Genotype::singleNodeGenotype(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0})));
    const double input = 0.0;

    member->cacheOutput(&input, Data::DataValue::scalar(42.0));
    ASSERT_NE(member->getCachedOutput(&input), nullptr);
    EXPECT_EQ(member->getCachedOutput(&input)->getScalar<double>(), 42.0);

    member->setGenotype(GraphBased::Genotype::singleNodeGenotype(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0})));
    EXPECT_EQ(member->getCachedOutput(&input), nullptr) << "Outputs of the previous genotype are not valid anymore";
}

TEST_F(TPGStepsTest, mutateVerticesStep)
{
    auto transfer = std::make_shared<Mutation::NodeTransferMutation>(0.1, 0.7, 0.7);
    const Evolution::BasicEvolutionConfig::Names vertices(namesOf("Vertices"));
    const auto offspring = vertices.offspring;
    const std::string generatorKey("VertexGenerator");

    // Mutated vertices transfer the nodes of the roots, with the generator and the mutation found in the data.
    Evolution::MutateVerticesStep mutate(offspring, vertices.population, generatorKey, transfer->getName(), 2);
    EXPECT_EQ(mutate.name(), "MutateVertices");
    EXPECT_EQ(mutate.requiredData(), (std::set<std::string>{offspring, vertices.population, generatorKey, transfer->getName()}));
    EXPECT_TRUE(mutate.providedData().empty()) << "Vertices are mutated in place";

    // The mutation must be a NodeTransferMutation.
    Evolution::EvolutionData state;
    state.set(transfer->getName(), std::shared_ptr<Mutation::Mutator>(std::make_shared<Mutation::PointMutator>(0.2, 0.5, 0.5)));
    EXPECT_THROW(mutate.apply(state), std::runtime_error);
}
