#include "selector/mapElites/mapElitesArchive.h"
#include "instructions/addPrimitiveType.h"
#include "instructions/lambdaInstruction.h"
#include "representation/lgp/environment.h"
#include <gtest/gtest.h>

/*
class MapElitesArchiveTest : public ::testing::Test
{
  protected:
    const size_t bins = 5;
    const size_t desc = 2;
    const double minV = 0.0;
    const double maxV = 10.0;

    std::shared_ptr<EvoGraph::Graph> graph;
    const EvoGraph::Vertex* dummyAgent;
    Environment* e = NULL;
    Learn::LearningParameters params;
    Instructions::Set set;
    const size_t size1{24};
    const size_t size2{32};
    std::vector<std::reference_wrapper<const Data::DataHandler>> vect;

    Selector::MapElites::MapElitesArchive* archive;

    // dummy vertex and evaluation
    std::shared_ptr<const EvoGraph::Vertex> dummyVertex;
    std::shared_ptr<const EvoGraph::Vertex> dummyVertex2;
    std::shared_ptr<Learn::EvaluationResult> dummyEval;

    void SetUp() override
    {
        archive =
            new Selector::MapElites::MapElitesArchive(bins, desc, minV, maxV);
        dummyEval = std::make_shared<Learn::EvaluationResult>(
            std::make_shared<Selector::SelectionMetrics>(1), 0);

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
        graph = std::make_shared<EvoGraph::Graph>();
        dummyVertex = graph->addNewTeam();
        dummyVertex2 = graph->addNewTeam();
    }

    void TearDown() override
    {
        delete archive;
    }
};

TEST_F(MapElitesArchiveTest, SizeAndDimensions)
{
    EXPECT_EQ(archive->size(), pow(bins, desc));
    auto [b, d] = archive->getDimensions();
    EXPECT_EQ(b, bins) << "Number of bins mismatch.";
    EXPECT_EQ(d, desc) << "Number of descriptors mismatch.";
}

TEST_F(MapElitesArchiveTest, ArchiveLimitsSize)
{
    EXPECT_EQ(archive->getArchiveLimits().size(), bins)
        << "Archive limits size mismatch.";
}

TEST_F(MapElitesArchiveTest, IndexWithinBounds)
{
    EXPECT_EQ(archive->getIndexArchive(0.1), 0u)
        << "Index calculation incorrect.";
    EXPECT_EQ(archive->getIndexArchive(9.9), bins - 1)
        << "Index calculation incorrect.";
}

TEST_F(MapElitesArchiveTest, IndexOutOfBoundsClamps)
{
    EXPECT_EQ(archive->getIndexArchive(-5), 0u)
        << "Index calculation incorrect.";
    EXPECT_EQ(archive->getIndexArchive(100), bins - 1)
        << "Index calculation incorrect.";
}

TEST_F(MapElitesArchiveTest, LinearIndexRoundTrip)
{
    std::vector<uint64_t> multi{2, 4};
    uint64_t linear = archive->computeLinearIndex(multi);
    auto back = archive->computeIndices(linear);
    EXPECT_EQ(multi, back) << "Linear index round-trip failed.";
}

TEST_F(MapElitesArchiveTest, SetAndGetByIndices)
{
    archive->setArchiveAt(dummyVertex, dummyEval, {2, 3});
    auto result = archive->getArchiveAt({2, 3});
    EXPECT_EQ(result.first, dummyEval)
        << "Failed to retrieve correct evaluation result.";
    EXPECT_EQ(result.second, dummyVertex)
        << "Failed to retrieve correct vertex.";
}

TEST_F(MapElitesArchiveTest, SetAndGetByDescriptors)
{
    archive->setArchiveFromDescriptors(dummyVertex, dummyEval, {5.2, 9.8});
    auto result = archive->getArchiveFromDescriptors({5.2, 9.8});
    EXPECT_EQ(result.first, dummyEval)
        << "Failed to retrieve correct evaluation result.";
    EXPECT_EQ(result.second, dummyVertex)
        << "Failed to retrieve correct vertex.";
}

TEST_F(MapElitesArchiveTest, ContainsRoot)
{
    archive->setArchiveAt(dummyVertex, dummyEval, {1, 1});
    EXPECT_TRUE(archive->containsRoot(dummyVertex))
        << "Archive should contain the added root.";
    EXPECT_FALSE(archive->containsRoot(dummyVertex2))
        << "Archive should not contain this root.";
}

TEST_F(MapElitesArchiveTest, RemoveRootDirect)
{
    archive->setArchiveAt(dummyVertex, dummyEval, {1, 1});
    archive->removeRootFromArchive(dummyVertex, 0);

    EXPECT_FALSE(archive->containsRoot(dummyVertex))
        << "Archive should not contain the removed root.";
}

TEST_F(MapElitesArchiveTest, RemoveRootOnlyIfNotComplete)
{
    auto selectionMetrics = std::make_shared<Selector::SelectionMetrics>(1);
    auto eval = std::make_shared<Learn::EvaluationResult>(selectionMetrics, 1);
    // We fake number of evaluations using a mockable assumption:
    // Assume default getNbEvaluation() is 0 unless overridden by your class
    archive->setArchiveAt(dummyVertex, eval, {2, 2});

    archive->removeRootFromArchiveIfNotComplete(dummyVertex,
                                                5); // 0 < 5 so it should remove
    EXPECT_FALSE(archive->containsRoot(dummyVertex))
        << "Archive should not contain the removed root.";
}

TEST_F(MapElitesArchiveTest, GetVerticesInArchive)
{
    archive->setArchiveAt(dummyVertex, dummyEval, {0, 0});
    archive->setArchiveAt(dummyVertex2, dummyEval, {1, 1});

    auto verts = archive->getVerticesInArchive();
    EXPECT_EQ(verts.size(), 2) << "Archive should contain 2 vertices.";
    EXPECT_TRUE(verts.count(dummyVertex))
        << "Archive should contain the first vertex.";
    EXPECT_TRUE(verts.count(dummyVertex2))
        << "Archive should contain the second vertex.";
}

TEST_F(MapElitesArchiveTest, LinearIndexConsistency)
{
    for (uint64_t i = 0; i < archive->size(); ++i) {
        auto coords = archive->computeIndices(i);
        auto back = archive->computeLinearIndex(coords);
        EXPECT_EQ(i, back) << "Linear index consistency failed at index " << i;
    }
}

TEST_F(MapElitesArchiveTest, getAllArchive)
{
    ASSERT_NO_THROW(archive->getAllArchive());
}*/