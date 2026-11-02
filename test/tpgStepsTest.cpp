#include <gtest/gtest.h>
#include <algorithm>

#include "evolution/module.h"
#include "evolution/originTPGSteps.h"
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

    std::shared_ptr<const Individual> makeVertex(const Representations::Representation& rep, const std::shared_ptr<const Individual>& member,
                                                 const std::vector<std::shared_ptr<const Individual>>& destinations)
    {
        auto group = std::make_unique<GraphBased::NodeGroup>();
        for(const std::shared_ptr<const Individual>& destination: destinations) {
            group->addNode(makeNode(member, Data::DataValue::scalar<std::shared_ptr<const Individual>>(destination)));
        }
        return std::make_shared<Individual>(rep, GraphBased::Genotype::singleGroupGenotype(std::move(group)));
    }

    std::shared_ptr<const Individual> makeLeaf(const Representations::Representation& rep, const std::shared_ptr<const Individual>& member)
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
    Representations::FakeRepresentation rep;
    std::shared_ptr<const Individual> member;
    std::shared_ptr<const Evolution::Population> roots;

    virtual void SetUp()
    {
        CounterReset::counterReset();
        member = std::make_shared<Individual>(rep);
        roots = std::make_shared<const Evolution::Population>(std::make_shared<Representations::FakeRepresentation>(), 3, "Roots");
    }

    // Replace the individuals of the population of roots.
    void setRoots(Evolution::EvolutionState& state, const std::vector<std::shared_ptr<const Individual>>& individuals) const
    {
        state.set(this->roots->individuals(), std::make_shared<std::vector<std::shared_ptr<const Individual>>>(individuals));
    }

    // Individuals of the population of roots.
    const std::vector<std::shared_ptr<const Individual>>& rootsOf(const Evolution::EvolutionState& state) const
    {
        return *state.get<std::vector<std::shared_ptr<const Individual>>>(this->roots->individuals());
    }

    // State where the roots have a "fitness", and empty "metrics".
    Evolution::EvolutionState makeState(const std::vector<std::shared_ptr<const Individual>>& individuals) const
    {
        Evolution::EvolutionState state;
        state.generation = 0;
        this->setRoots(state, individuals);
        auto fitness = std::make_shared<std::map<uint64_t, double>>();
        for(const std::shared_ptr<const Individual>& individual: individuals) {
            (*fitness)[individual->getIndividualID()] = 1.0;
        }
        state.set(this->roots->data("fitness"), fitness);
        state.set(this->roots->data("metrics"), std::make_shared<std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>>());
        return state;
    }
};

TEST_F(TPGStepsTest, graphHelpers)
{
    std::shared_ptr<const Individual> leaf1 = makeLeaf(rep, member);
    std::shared_ptr<const Individual> leaf2 = makeLeaf(rep, member);
    std::shared_ptr<const Individual> vertexA = makeVertex(rep, member, {leaf2, leaf1, leaf2});
    std::shared_ptr<const Individual> vertexB = makeVertex(rep, member, {vertexA});

    // Direct references, without duplicates and ordered by ID.
    EXPECT_EQ(idsOf(Evolution::getSubVertices(*vertexA)), (std::vector<uint64_t>{leaf1->getIndividualID(), leaf2->getIndividualID()}));
    EXPECT_TRUE(Evolution::getSubVertices(*leaf1).empty());

    // Everything reachable, roots included.
    std::vector<std::shared_ptr<const Individual>> all = Evolution::getAllVertices({vertexB});
    EXPECT_EQ(idsOf(all), (std::vector<uint64_t>{leaf1->getIndividualID(), leaf2->getIndividualID(), vertexA->getIndividualID(), vertexB->getIndividualID()}));
    EXPECT_EQ(Evolution::getAllVertices({vertexB, vertexA, leaf1}).size(), 4U);

    // Members are the ones used by the nodes.
    std::vector<std::shared_ptr<const Individual>> members = Evolution::getMembersOf(all);
    ASSERT_EQ(members.size(), 1U);
    EXPECT_EQ(members.at(0), member);
}

TEST_F(TPGStepsTest, protectAggregated)
{
    std::shared_ptr<const Individual> leaf1 = makeLeaf(rep, member);
    std::shared_ptr<const Individual> leaf2 = makeLeaf(rep, member);
    std::shared_ptr<const Individual> vertexA = makeVertex(rep, member, {leaf1});

    Evolution::ProtectAggregatedStep step(roots->individuals());
    EXPECT_EQ(step.providedData(), (std::set<std::string>{roots->individuals()}));
    Evolution::EvolutionState state = makeState({leaf1, leaf2, vertexA});
    ASSERT_NO_THROW(step.apply(state));

    // The leaf referenced by A is not a root anymore.
    EXPECT_EQ(idsOf(rootsOf(state)), (std::vector<uint64_t>{leaf2->getIndividualID(), vertexA->getIndividualID()}));
    EXPECT_EQ(step.getNbProtected(), 1U);

    // Nothing changes while the aggregator is alive.
    ASSERT_NO_THROW(step.apply(state));
    EXPECT_EQ(rootsOf(state).size(), 2U);
    EXPECT_EQ(step.getNbProtected(), 1U);

    // The aggregator disappears: the leaf comes back, ordered by ID.
    setRoots(state, {leaf2});
    ASSERT_NO_THROW(step.apply(state));
    EXPECT_EQ(idsOf(rootsOf(state)), (std::vector<uint64_t>{leaf1->getIndividualID(), leaf2->getIndividualID()}));
    EXPECT_EQ(step.getNbProtected(), 0U);

    // A new evolution forgets the protected vertices.
    setRoots(state, {leaf1, vertexA});
    step.apply(state);
    EXPECT_EQ(step.getNbProtected(), 1U);
    state.generation = Evolution::EvolutionState::initGeneration;
    step.apply(state);
    EXPECT_EQ(step.getNbProtected(), 0U);
}

TEST_F(TPGStepsTest, protectAggregatedChains)
{
    std::shared_ptr<const Individual> leaf = makeLeaf(rep, member);
    std::shared_ptr<const Individual> vertexA = makeVertex(rep, member, {leaf});
    std::shared_ptr<const Individual> vertexB = makeVertex(rep, member, {vertexA});

    Evolution::ProtectAggregatedStep step(roots->individuals());
    Evolution::EvolutionState state = makeState({leaf, vertexA, vertexB});
    step.apply(state);

    // Only the top of the chain is a root, but the whole chain stays protected while it lives.
    EXPECT_EQ(idsOf(rootsOf(state)), (std::vector<uint64_t>{vertexB->getIndividualID()}));
    EXPECT_EQ(step.getNbProtected(), 2U);
    step.apply(state);
    EXPECT_EQ(step.getNbProtected(), 2U);
    EXPECT_EQ(idsOf(rootsOf(state)), (std::vector<uint64_t>{vertexB->getIndividualID()}));

    // When the top dies, A comes back as a root with its own protected leaf.
    setRoots(state, {});
    step.apply(state);
    EXPECT_EQ(idsOf(rootsOf(state)), (std::vector<uint64_t>{vertexA->getIndividualID()}));
    EXPECT_EQ(step.getNbProtected(), 1U);
}

TEST_F(TPGStepsTest, archiveIsADataOfThePopulation)
{
    Evolution::CreateArchiveStep create(roots->data("archive"), {Dimensions::Requirement(Data::DataType::type<int>({2}))});
    Evolution::UpdateArchiveStep update(roots->individuals(), roots->data("metrics"), roots->data("archive"), 10, false);
    EXPECT_EQ(create.providedData(), (std::set<std::string>{roots->data("archive")}));
    EXPECT_EQ(update.requiredData().count(roots->data("archive")), 1U);

    Evolution::EvolutionState state;
    state.generation = Evolution::EvolutionState::initGeneration;
    EXPECT_THROW(update.apply(state), std::runtime_error) << "The archive must be created first";

    ASSERT_NO_THROW(create.apply(state));
    const std::shared_ptr<Evaluation::DataSet> archive = state.get<Evaluation::DataSet>(roots->data("archive"));
    ASSERT_NE(archive, nullptr);
    EXPECT_THROW(update.apply(state), std::runtime_error) << "The update samples the metrics of the population";
    setRoots(state, {});
    state.set(roots->data("metrics"), std::make_shared<std::map<uint64_t, std::unique_ptr<Metrics::MetricMap>>>());

    // Nothing to sample from an empty population.
    ASSERT_NO_THROW(update.apply(state));
    EXPECT_EQ(archive->size(), 0U);

    // Later generations of a fixed archive only update answers, which is nothing for an empty archive.
    Evolution::UpdateArchiveStep fixed(roots->individuals(), roots->data("metrics"), roots->data("archive"), 10, true);
    state.generation = 3;
    ASSERT_NO_THROW(fixed.apply(state));
    EXPECT_EQ(state.get<Evaluation::DataSet>(roots->data("archive")), archive);
    EXPECT_EQ(archive->size(), 0U);

    // A new evolution starts without archive.
    state.reset(0);
    EXPECT_FALSE(state.has(roots->data("archive")));
}

TEST_F(TPGStepsTest, cacheMemberOutputs)
{
    Evaluation::DataSet dataSet = createSmallDataSet();
    auto problem = std::make_shared<Evaluation::PredictionProblem>(dataSet);

    // Members output a constant, whatever the sample.
    Representations::FakeRepresentation memberRep(problem->getInputDimensions(), Dimensions::Requirement(Data::DataType::type<double>()));
    std::shared_ptr<const Individual> cachedMember = std::make_shared<Individual>(memberRep,
        GraphBased::Genotype::singleNodeGenotype(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0})));
    ASSERT_TRUE(cachedMember->isValid());
    std::shared_ptr<const Individual> vertex = makeLeaf(rep, cachedMember);

    // The members of the roots and of the offspring are cached.
    const std::string offspring = roots->data("offspring");
    Evolution::CacheMemberOutputsStep step({roots->individuals(), offspring}, roots->data("cache"), problem, std::make_shared<Evaluation::Evaluator>(), 2);
    Evolution::EvolutionState state = makeState({vertex});
    state.set(offspring, std::make_shared<std::vector<std::shared_ptr<Individual>>>());
    EXPECT_EQ(step.requiredData(), (std::set<std::string>{roots->individuals(), offspring}));
    EXPECT_EQ(step.providedData(), (std::set<std::string>{roots->data("cache")}));
    EXPECT_THROW(Evolution::CacheMemberOutputsStep({offspring}, roots->data("cache"), nullptr, std::make_shared<Evaluation::Evaluator>(), 2), std::runtime_error);

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
    ASSERT_TRUE(state.has(roots->data("cache")));
    EXPECT_EQ(state.get<std::vector<std::shared_ptr<const Individual>>>(roots->data("cache"))->size(), 1U);
    setRoots(state, {});
    ASSERT_NO_THROW(step.apply(state));
    EXPECT_EQ(cachedMember->getNbCachedOutputs(), 0U);
    EXPECT_TRUE(state.get<std::vector<std::shared_ptr<const Individual>>>(roots->data("cache"))->empty());
}

TEST_F(TPGStepsTest, cachedOutputsAreClearedWithTheGenotype)
{
    Representations::FakeRepresentation memberRep(std::vector<Dimensions::Requirement>{Dimensions::Requirement(Data::DataType::type<double>())},
                                                  Dimensions::Requirement(Data::DataType::type<double>()));
    auto member = std::make_shared<Individual>(memberRep, GraphBased::Genotype::singleNodeGenotype(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0})));
    const double input = 0.0;

    member->cacheOutput(&input, Data::DataValue::scalar(42.0));
    ASSERT_NE(member->getCachedOutput(&input), nullptr);
    EXPECT_EQ(member->getCachedOutput(&input)->getScalar<double>(), 42.0);

    member->setGenotype(GraphBased::Genotype::singleNodeGenotype(std::make_unique<GraphBased::GPNode>(std::vector<size_t>{0})));
    EXPECT_EQ(member->getCachedOutput(&input), nullptr) << "Outputs of the previous genotype are not valid anymore";
}

TEST_F(TPGStepsTest, vertexSteps)
{
    const std::vector<Dimensions::Requirement> inputs{Dimensions::Requirement(Data::DataType::type<double>())};
    auto tpg = std::make_shared<Representations::TPG>(inputs, 3, 2, 10);
    auto memberRep = std::make_shared<Representations::FakeRepresentation>(inputs, Dimensions::Requirement(Data::DataType::type<double>()));
    auto transfer = std::make_shared<Mutation::NodeTransferMutation>(0.1, 0.7, 0.7);
    auto memberMutator = std::make_shared<Mutation::PointMutator>(0.2, 0.5, 0.5);
    auto vertices = std::make_shared<const Evolution::Population>(tpg, 5, "Vertices");
    const std::string offspring = vertices->data("offspring");
    const std::string archive = vertices->data("archive");

    // The vertices are created from the archive if it exists, so it is not required.
    Evolution::InitVerticesStep init(vertices, transfer, memberRep, memberMutator, archive);
    EXPECT_EQ(init.name(), "InitVertices");
    EXPECT_TRUE(init.requiredData().empty());
    EXPECT_EQ(init.providedData(), (std::set<std::string>{vertices->individuals()}));
    EXPECT_THROW(Evolution::InitVerticesStep(roots, transfer, memberRep, memberMutator, archive), std::runtime_error) << "The roots are not TPG vertices";
    EXPECT_THROW(Evolution::InitVerticesStep(vertices, transfer, nullptr, memberMutator, archive), std::runtime_error);

    // Mutated vertices transfer the nodes of the roots.
    Evolution::MutateVerticesStep mutate(offspring, vertices->individuals(), archive, tpg, transfer, memberRep, memberMutator, 2);
    EXPECT_EQ(mutate.name(), "MutateVertices");
    EXPECT_EQ(mutate.requiredData(), (std::set<std::string>{offspring, vertices->individuals()}));
    EXPECT_TRUE(mutate.providedData().empty()) << "Vertices are mutated in place";
    EXPECT_THROW(Evolution::MutateVerticesStep(offspring, vertices->individuals(), archive, nullptr, transfer, memberRep, memberMutator), std::runtime_error);
    EXPECT_THROW(Evolution::MutateVerticesStep(offspring, vertices->individuals(), archive, tpg, nullptr, memberRep, memberMutator), std::runtime_error);
}
