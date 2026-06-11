/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2025) :
 *
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
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
    std::string getName() const override
    {
        return "None";
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
    ASSERT_EQ(descriptor->getName(), "ActionValues");
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
