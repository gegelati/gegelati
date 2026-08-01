#include "instructions/addPrimitiveType.h"
#include "instructions/lambdaInstruction.h"
#include "learn/fakeMultiContinuousLearningEnvironment.h"
#include "selector/mapElites/mapElitesDefaultDescriptors.h"
#include "selector/mapElites/mapElitesDescriptor.h"
#include "evoGraph/graph.h"
#include <gtest/gtest.h>
#if 0
class MapElitesDescriptorsTest : public ::testing::Test
{
  protected:
    std::shared_ptr<EvoGraph::Graph> graph;
    FakeMultiContinuousLearningEnvironment env;
    std::shared_ptr<Selector::MapElites::DefaultDescriptors::ActionValues>
        descriptor;
    const EvoGraph::Vertex* dummyIndividual;
    Environment* e = NULL;
    Learn::LearningParameters params;
    Instructions::Set set;
    const size_t size1{24};
    const size_t size2{32};
    std::vector<std::reference_wrapper<const Data::DataHandler>> vect;

    void SetUp() override
    {
        vect.push_back(
            *(new Data::PrimitiveTypeArray<double>((unsigned int)size1)));
        vect.push_back(
            *(new Data::PrimitiveTypeArray<float>((unsigned int)size2)));

        set.add(*(new Instructions::AddPrimitiveType<float>()));
        auto minus = [](double a, double b) -> double { return a - b; };
        set.add(*(new Instructions::LambdaInstruction<double, double>(minus)));

        params.representation.lgp.nbRegisters = 8;
        params.representation.lgp.nbProgramConstant = 1;
        e = new Environment(set, params, vect, 3);
        graph = std::make_shared<EvoGraph::Graph>(*e);
        dummyIndividual = &graph->addNewTeam();

        descriptor = std::make_shared<
            Selector::MapElites::DefaultDescriptors::ActionValues>();
    }
};

class FakeDescriptor : public Selector::MapElites::MapElitesDescriptor
{
    void initDescriptor(
        const EvoGraph::Graph& graph,
        const Learn::LearningEnvironment& learningEnvironment) override
    {
    }
    std::string getName() const override {return "None";}
};

TEST_F(MapElitesDescriptorsTest, EmptyMethods)
{
    FakeDescriptor fakeD;
    const EvoGraph::Vertex* fakeIndividual = nullptr;

    std::vector<double> metrics(1, 0.0);
    fakeD.extractMetricsStep(metrics, fakeIndividual, {0.4, 0.2, 1.0}, env);
    fakeD.extractMetricsEpisode(metrics, fakeIndividual, 4, env);
}

TEST_F(MapElitesDescriptorsTest, InitDescriptorMarksInitAndSetsValues)
{
    EXPECT_FALSE(descriptor->isInit());

    descriptor->initDescriptor(*graph, env);

    EXPECT_TRUE(descriptor->isInit());
    EXPECT_EQ(descriptor->getNbDescriptors(), env.getNbActions());

    auto range = descriptor->getMinAndMaxRange();
    EXPECT_DOUBLE_EQ(range.first, 0.0);
    EXPECT_DOUBLE_EQ(range.second, 1.0);
}

TEST_F(MapElitesDescriptorsTest, InitDescriptorSetsCorrectDescriptorCount)
{
    descriptor->initDescriptor(*graph, env);
    EXPECT_EQ(descriptor->getNbDescriptors(), env.getNbActions());
}

TEST_F(MapElitesDescriptorsTest,
       ExtractMetricsStepAccumulatesAbsoluteActionValues)
{
    ASSERT_EQ(descriptor->getName(), "ActionValues");
    descriptor->initDescriptor(*graph, env);
    size_t n = descriptor->getNbDescriptors();
    std::vector<double> metrics(n, 0.0);

    const EvoGraph::Vertex* fakeIndividual = nullptr;

    std::vector<double> actions1 = {0.5, -0.2, 1.0};
    std::vector<double> actions2 = {0.1, -0.8, -0.4};

    descriptor->extractMetricsStep(metrics, fakeIndividual, actions1, env);
    descriptor->extractMetricsStep(metrics, fakeIndividual, actions2, env);

    EXPECT_DOUBLE_EQ(metrics[0], 0.5 + 0.1);
    EXPECT_DOUBLE_EQ(metrics[1], 0.2 + 0.8);
    EXPECT_DOUBLE_EQ(metrics[2], 1.0 + 0.4);
}

TEST_F(MapElitesDescriptorsTest, ExtractMetricsEpisodeConvertsSumToAverage)
{
    descriptor->initDescriptor(*graph, env);
    size_t n = descriptor->getNbDescriptors();
    std::vector<double> metrics(n, 0.0);

    const EvoGraph::Vertex* fakeIndividual = nullptr;

    // Simulate 4 steps
    descriptor->extractMetricsStep(metrics, fakeIndividual, {0.4, 0.2, 1.0}, env);
    descriptor->extractMetricsStep(metrics, fakeIndividual, {0.6, 0.8, 0.0}, env);
    descriptor->extractMetricsStep(metrics, fakeIndividual, {0.0, 0.0, 2.0}, env);
    descriptor->extractMetricsStep(metrics, fakeIndividual, {1.0, 0.0, 0.0}, env);

    // Apply episode finalization
    descriptor->extractMetricsEpisode(metrics, fakeIndividual, 4, env);

    EXPECT_DOUBLE_EQ(metrics[0], (0.4 + 0.6 + 0.0 + 1.0) / 4.0);
    EXPECT_DOUBLE_EQ(metrics[1], (0.2 + 0.8 + 0.0 + 0.0) / 4.0);
    EXPECT_DOUBLE_EQ(metrics[2], (1.0 + 0.0 + 2.0 + 0.0) / 4.0);
}

TEST_F(MapElitesDescriptorsTest, ExtractMetricsEpisodeWithOneStep)
{
    descriptor->initDescriptor(*graph, env);
    size_t n = descriptor->getNbDescriptors();
    std::vector<double> metrics(n, 0.0);

    const EvoGraph::Vertex* fakeIndividual = nullptr;
    descriptor->extractMetricsStep(metrics, fakeIndividual, {0.7, 0.3, 1.5}, env);
    descriptor->extractMetricsEpisode(metrics, fakeIndividual, 1, env);

    EXPECT_DOUBLE_EQ(metrics[0], 0.7);
    EXPECT_DOUBLE_EQ(metrics[1], 0.3);
    EXPECT_DOUBLE_EQ(metrics[2], 1.5);
}

TEST_F(MapElitesDescriptorsTest, DoesNotCrashOnZeroMetricsBeforeEpisode)
{
    descriptor->initDescriptor(*graph, env);
    size_t n = descriptor->getNbDescriptors();
    std::vector<double> metrics(n, 0.0);

    const EvoGraph::Vertex* fakeIndividual = nullptr;
    EXPECT_NO_THROW(
        descriptor->extractMetricsEpisode(metrics, fakeIndividual, 5, env));

    for (double v : metrics) {
        EXPECT_DOUBLE_EQ(v, 0.0);
    }
}

TEST_F(MapElitesDescriptorsTest, ExtractMetricsStepIgnoresIndividualAndEnvContent)
{
    descriptor->initDescriptor(*graph, env);
    size_t n = descriptor->getNbDescriptors();
    std::vector<double> metrics(n, 0.0);

    EXPECT_NO_THROW(descriptor->extractMetricsStep(metrics, nullptr,
                                                   {0.3, -0.3, 0.3}, env));
}

#endif