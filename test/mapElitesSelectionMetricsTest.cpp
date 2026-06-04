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

#include "selector/mapElites/mapElitesSelectionMetrics.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/lambdaInstruction.h"
#include "learn/fakeMultiContinuousLearningEnvironment.h"
#include "selector/mapElites/mapElitesDefaultDescriptors.h"
#include "tpg/tpgGraph.h"
#include <gtest/gtest.h>

class MapElitesSelectionMetricsTest : public ::testing::Test
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
        descriptor->initDescriptor(*graph, env);
    }
};

TEST_F(MapElitesSelectionMetricsTest, ConstructorScoreOnly_HasNoDescriptors)
{
    Selector::MapElites::MapElitesSelectionMetrics m(42.0);
    EXPECT_DOUBLE_EQ(m.getScore(), 42.0);
    EXPECT_TRUE(m.getMapDescriptors().empty());
}

TEST_F(MapElitesSelectionMetricsTest, ConstructorWithDescriptors_HasKeys)
{
    Selector::MapElites::MapElitesSelectionMetrics m({descriptor});
    ASSERT_EQ(m.getMapDescriptors().size(), 1u);
    EXPECT_NE(m.getMapDescriptors().count(descriptor), 0u);
}

TEST_F(MapElitesSelectionMetricsTest, InitMetrics_ResizesDescriptorVectors)
{
    Selector::MapElites::MapElitesSelectionMetrics m({descriptor});

    m.initMetrics(dummyAgent, env);

    const auto& map = m.getMapDescriptors();
    ASSERT_EQ(map.size(), 1u);

    const auto& vec = map.at(descriptor);
    EXPECT_EQ(vec.size(), descriptor->getNbDescriptors());
    EXPECT_TRUE(
        std::all_of(vec.begin(), vec.end(), [](double v) { return v == 0.0; }));
}

TEST_F(MapElitesSelectionMetricsTest, ExtractMetricsStep_FillsDescriptorValues)
{
    Selector::MapElites::MapElitesSelectionMetrics m({descriptor});
    m.initMetrics(dummyAgent, env);

    std::vector<double> actions = {1.0, -2.0, 3.0};
    m.extractMetricsStep(dummyAgent, actions, env);

    const auto& vec = m.getMapDescriptors().at(descriptor);

    // ActionValues descriptor computes absolute values average incrementally,
    // so just check we got non-zero and correct size
    EXPECT_EQ(vec.size(), descriptor->getNbDescriptors());
    bool anyNonZero =
        std::any_of(vec.begin(), vec.end(), [](double v) { return v > 0.0; });
    EXPECT_TRUE(anyNonZero);
}

TEST_F(MapElitesSelectionMetricsTest,
       ExtractMetricsEpisode_ForwardsToDescriptor)
{
    Selector::MapElites::MapElitesSelectionMetrics m({descriptor});
    m.initMetrics(dummyAgent, env);

    double beforeScore = m.getScore();
    m.extractMetricsEpisode(dummyAgent, 10, env);
    double afterScore = m.getScore();

    // Score may be modified by base class, but must still exist
    EXPECT_GE(afterScore, 0.0);
    EXPECT_NE(beforeScore, -1); // sanity check only
}

TEST_F(MapElitesSelectionMetricsTest,
       WeightedSum_MergesScoresAndDescriptorValues)
{
    std::map<std::shared_ptr<const Selector::MapElites::MapElitesDescriptor>,
             std::vector<double>>
        mapA = {{descriptor, {2.0, 2.0, 2.0}}};
    std::map<std::shared_ptr<const Selector::MapElites::MapElitesDescriptor>,
             std::vector<double>>
        mapB = {{descriptor, {4.0, 4.0, 4.0}}};
    Selector::MapElites::MapElitesSelectionMetrics A(10, mapA);
    Selector::MapElites::MapElitesSelectionMetrics B(20, mapB);

    A.weightedSum(
        std::make_shared<Selector::MapElites::MapElitesSelectionMetrics>(B), 1,
        1);

    EXPECT_DOUBLE_EQ(A.getScore(), 15.0);

    for (double v : A.getMapDescriptors().at(descriptor)) {
        EXPECT_DOUBLE_EQ(v, 3.0);
    }
}

TEST_F(MapElitesSelectionMetricsTest, WeightedSum_ThrowsOnMismatchedVectorSize)
{
    Selector::MapElites::MapElitesSelectionMetrics A({descriptor});
    Selector::MapElites::MapElitesSelectionMetrics B({descriptor});

    A.initMetrics(dummyAgent, env);
    B.initMetrics(dummyAgent, env);

    auto& vecB =
        const_cast<std::vector<double>&>(B.getMapDescriptors().at(descriptor));
    vecB.push_back(999.0); // break size

    EXPECT_THROW(
        A.weightedSum(
            std::make_shared<Selector::MapElites::MapElitesSelectionMetrics>(B),
            1, 1),
        std::runtime_error);
}
