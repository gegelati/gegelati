#include "instructions/addPrimitiveType.h"
#include "instructions/lambdaInstruction.h"
#include "learn/fakeMultiContinuousLearningEnvironment.h"
#include "selector/mapElites/mapElitesDefaultDescriptors.h"
#include "selector/mapElites/mapElitesDescriptor.h"
#include "tpg/tpgGraph.h"
#include <gtest/gtest.h>

class MapElitesDescriptorsTest : public ::testing::Test
{
  protected:
    std::shared_ptr<TPG::TPGGraph> graph;
    FakeMultiContinuousLearningEnvironment env;
    std::shared_ptr<Selector::MapElites::DefaultDescriptors::ActionValues>
        descriptor;
    const TPG::TPGVertex* dummyAgent;
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

        params.nbRegisters = 8;
        params.nbProgramConstant = 1;
        e = new Environment(set, params, vect, 3);
        graph = std::make_shared<TPG::TPGGraph>(*e);
        dummyAgent = &graph->addNewTeam();

        descriptor = std::make_shared<
            Selector::MapElites::DefaultDescriptors::ActionValues>();
    }
};

class FakeDescriptor : public Selector::MapElites::MapElitesDescriptor
{
    void initDescriptor(
        const TPG::TPGGraph& graph,
        const Learn::LearningEnvironment& learningEnvironment) override
    {
    }
};

TEST_F(MapElitesDescriptorsTest, EmptyMethods)
{
    FakeDescriptor fakeD;
    const TPG::TPGVertex* fakeAgent = nullptr;

    std::vector<double> metrics(1, 0.0);
    fakeD.extractMetricsStep(metrics, fakeAgent, {0.4, 0.2, 1.0}, env);
    fakeD.extractMetricsEpisode(metrics, fakeAgent, 4, env);
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
    descriptor->initDescriptor(*graph, env);
    size_t n = descriptor->getNbDescriptors();
    std::vector<double> metrics(n, 0.0);

    const TPG::TPGVertex* fakeAgent = nullptr;

    std::vector<double> actions1 = {0.5, -0.2, 1.0};
    std::vector<double> actions2 = {0.1, -0.8, -0.4};

    descriptor->extractMetricsStep(metrics, fakeAgent, actions1, env);
    descriptor->extractMetricsStep(metrics, fakeAgent, actions2, env);

    EXPECT_DOUBLE_EQ(metrics[0], 0.5 + 0.1);
    EXPECT_DOUBLE_EQ(metrics[1], 0.2 + 0.8);
    EXPECT_DOUBLE_EQ(metrics[2], 1.0 + 0.4);
}

TEST_F(MapElitesDescriptorsTest, ExtractMetricsEpisodeConvertsSumToAverage)
{
    descriptor->initDescriptor(*graph, env);
    size_t n = descriptor->getNbDescriptors();
    std::vector<double> metrics(n, 0.0);

    const TPG::TPGVertex* fakeAgent = nullptr;

    // Simulate 4 steps
    descriptor->extractMetricsStep(metrics, fakeAgent, {0.4, 0.2, 1.0}, env);
    descriptor->extractMetricsStep(metrics, fakeAgent, {0.6, 0.8, 0.0}, env);
    descriptor->extractMetricsStep(metrics, fakeAgent, {0.0, 0.0, 2.0}, env);
    descriptor->extractMetricsStep(metrics, fakeAgent, {1.0, 0.0, 0.0}, env);

    // Apply episode finalization
    descriptor->extractMetricsEpisode(metrics, fakeAgent, 4, env);

    EXPECT_DOUBLE_EQ(metrics[0], (0.4 + 0.6 + 0.0 + 1.0) / 4.0);
    EXPECT_DOUBLE_EQ(metrics[1], (0.2 + 0.8 + 0.0 + 0.0) / 4.0);
    EXPECT_DOUBLE_EQ(metrics[2], (1.0 + 0.0 + 2.0 + 0.0) / 4.0);
}

TEST_F(MapElitesDescriptorsTest, ExtractMetricsEpisodeWithOneStep)
{
    descriptor->initDescriptor(*graph, env);
    size_t n = descriptor->getNbDescriptors();
    std::vector<double> metrics(n, 0.0);

    const TPG::TPGVertex* fakeAgent = nullptr;
    descriptor->extractMetricsStep(metrics, fakeAgent, {0.7, 0.3, 1.5}, env);
    descriptor->extractMetricsEpisode(metrics, fakeAgent, 1, env);

    EXPECT_DOUBLE_EQ(metrics[0], 0.7);
    EXPECT_DOUBLE_EQ(metrics[1], 0.3);
    EXPECT_DOUBLE_EQ(metrics[2], 1.5);
}

TEST_F(MapElitesDescriptorsTest, DoesNotCrashOnZeroMetricsBeforeEpisode)
{
    descriptor->initDescriptor(*graph, env);
    size_t n = descriptor->getNbDescriptors();
    std::vector<double> metrics(n, 0.0);

    const TPG::TPGVertex* fakeAgent = nullptr;
    EXPECT_NO_THROW(
        descriptor->extractMetricsEpisode(metrics, fakeAgent, 5, env));

    for (double v : metrics) {
        EXPECT_DOUBLE_EQ(v, 0.0);
    }
}

TEST_F(MapElitesDescriptorsTest, ExtractMetricsStepIgnoresAgentAndEnvContent)
{
    descriptor->initDescriptor(*graph, env);
    size_t n = descriptor->getNbDescriptors();
    std::vector<double> metrics(n, 0.0);

    EXPECT_NO_THROW(descriptor->extractMetricsStep(metrics, nullptr,
                                                   {0.3, -0.3, 0.3}, env));
}
