#include "selector/mapElites/cvtMapElitesArchive.h"
#include <gtest/gtest.h>

class CvtMapElitesArchiveTest : public ::testing::Test
{
  protected:
    Mutator::RNG rng{42}; // fixed seed for deterministic behavior
    const size_t centroids = 6;
    const size_t descriptors = 3;
    const double minV = 0.0;
    const double maxV = 10.0;
    const size_t iter = 5;
    const size_t dots = 20;
    const double a1 = 0.5, b1 = 0.5, a2 = 0.5, b2 = 0.5;

    Selector::MapElites::CvtMapElitesArchive* archive;

    const TPG::TPGVertex* v1;
    const TPG::TPGVertex* v2;
    std::shared_ptr<Learn::EvaluationResult> eval1;
    std::shared_ptr<Learn::EvaluationResult> eval2;

    void SetUp() override
    {
        archive = new Selector::MapElites::CvtMapElitesArchive(
            rng, centroids, descriptors, minV, maxV, iter, dots, a1, b1, a2,
            b2);
        eval1 = std::make_shared<Learn::EvaluationResult>(
            std::make_shared<Selector::SelectionMetrics>(0), 1);
        eval2 = std::make_shared<Learn::EvaluationResult>(
            std::make_shared<Selector::SelectionMetrics>(1), 1);
    }

    void TearDown() override
    {
        delete archive;
    }
};

TEST_F(CvtMapElitesArchiveTest, DistSquared)
{
    std::vector<double> a{1, 2, 3}, b{1, 2, 6};
    EXPECT_EQ(archive->dist_squared(a, b), 9.0)
        << "Distance squared calculation is incorrect.";
}

TEST_F(CvtMapElitesArchiveTest, AddVectors)
{
    std::vector<double> a{1, 2, 3}, b{3, 2, 1};
    EXPECT_EQ(archive->add(a, b), (std::vector<double>{4, 4, 4}))
        << "Vector addition is incorrect.";
}

TEST_F(CvtMapElitesArchiveTest, ScalarMult)
{
    std::vector<double> a{2, 4, 6};
    EXPECT_EQ(archive->scalar_mult(a, 0.5), (std::vector<double>{1, 2, 3}))
        << "Scalar multiplication is incorrect.";
}

TEST_F(CvtMapElitesArchiveTest, AveragePoints)
{
    std::vector<std::vector<double>> pts{{0, 0, 0}, {2, 2, 2}};
    EXPECT_EQ(archive->average(pts), (std::vector<double>{1, 1, 1}))
        << "Average calculation is incorrect.";
}

TEST_F(CvtMapElitesArchiveTest, AverageEmpty)
{
    std::vector<std::vector<double>> pts;
    EXPECT_EQ(archive->average(pts), std::vector<double>(descriptors, 0.0))
        << "Average calculation is incorrect.";
}

TEST_F(CvtMapElitesArchiveTest, RandomPointInBounds)
{
    auto p = archive->random_point(rng);
    ASSERT_EQ(p.size(), descriptors) << "Random point dimension mismatch.";
    for (double v : p) {
        EXPECT_GE(v, minV) << "Random point value is out of bounds.";
        EXPECT_LE(v, maxV) << "Random point value is out of bounds.";
    }
}

TEST_F(CvtMapElitesArchiveTest, NearestCentroid)
{
    std::vector<std::vector<double>> cents{{0, 0, 0}, {5, 5, 5}, {10, 10, 10}};
    std::vector<double> p{4.9, 5.1, 5.0};
    EXPECT_EQ(archive->nearest(p, cents), 1u)
        << "Nearest centroid calculation is incorrect.";
}

TEST_F(CvtMapElitesArchiveTest, NearestCentroidThrowsOnEmpty)
{
    EXPECT_THROW(archive->nearest({1, 1, 1}, {}), std::runtime_error)
        << "Expected exception when finding nearest centroid in empty list.";
}

TEST_F(CvtMapElitesArchiveTest, CentroidCountMatchesArchiveSize)
{
    EXPECT_EQ(archive->getCentroids().size(), centroids)
        << "Centroid count does not match archive size.";
    EXPECT_EQ(archive->size(), centroids)
        << "Archive size does not match centroid count.";
}

TEST_F(CvtMapElitesArchiveTest, IndexForDescriptorIsValid)
{
    std::vector<double> desc(descriptors, 5.0);
    size_t idx = archive->getIndexForDescriptor(desc);
    EXPECT_LT(idx, centroids) << "Index for descriptor is out of bounds.";
}

TEST_F(CvtMapElitesArchiveTest, SetAndGetFromDescriptors)
{
    std::vector<double> d{2, 3, 4};
    archive->setArchiveFromDescriptors(v1, eval1, d);
    auto out = archive->getArchiveFromDescriptors(d);
    EXPECT_EQ(out.first, eval1)
        << "Failed to retrieve correct evaluation result.";
    EXPECT_EQ(out.second, v1) << "Failed to retrieve correct vertex.";
}

TEST_F(CvtMapElitesArchiveTest, OverwriteArchiveCell)
{
    std::vector<double> d{1, 1, 1};
    archive->setArchiveFromDescriptors(v1, eval1, d);
    archive->setArchiveFromDescriptors(v2, eval2, d);

    auto result = archive->getArchiveFromDescriptors(d);
    EXPECT_EQ(result.second, v2) << "Failed to retrieve correct vertex.";
    EXPECT_EQ(result.first, eval2)
        << "Failed to retrieve correct evaluation result.";
}

TEST_F(CvtMapElitesArchiveTest, SameDescriptorSameIndex)
{
    std::vector<double> d1{2.4, 7.1, 3.3};
    std::vector<double> d2{2.4, 7.1, 3.3};
    EXPECT_EQ(archive->getIndexForDescriptor(d1),
              archive->getIndexForDescriptor(d2))
        << "Same descriptors did not yield same index.";
}

TEST_F(CvtMapElitesArchiveTest, CentroidsInitializedWithinBounds)
{
    for (const auto& c : archive->getCentroids()) {
        ASSERT_EQ(c.size(), descriptors);
        for (double v : c) {
            EXPECT_GE(v, minV) << "Centroid value is out of bounds.";
            EXPECT_LE(v, maxV) << "Centroid value is out of bounds.";
        }
    }
}

TEST_F(CvtMapElitesArchiveTest, InitializeCvtReinitializesCentroids)
{
    // Use the fixture's archive and rng (remember fixture's constructor already
    // called initialize_cvt once) Capture current centroids
    auto before = archive->getCentroids();

    // Call initialize_cvt again with the same RNG instance (rng state has
    // advanced from constructor)
    archive->initialize_cvt(rng);

    auto after = archive->getCentroids();

    ASSERT_EQ(before.size(), after.size())
        << "Centroid count changed after re-initialization.";
    bool anyDifferent = false;
    for (size_t i = 0; i < before.size(); ++i) {
        for (size_t d = 0; d < before[i].size(); ++d) {
            if (before[i][d] != after[i][d]) {
                anyDifferent = true;
                break;
            }
        }
        if (anyDifferent)
            break;
    }

    // It is extremely likely centroids changed; assert that at least one
    // centroid coordinate differs.
    EXPECT_TRUE(anyDifferent)
        << "initialize_cvt did not change any centroid (unexpected).";
}

TEST_F(CvtMapElitesArchiveTest, InitializeCvtPopulatesWithinBounds)
{
    // Create an archive with 0 iterations to get a minimal initial population
    // via initialize_cvt in constructor
    Mutator::RNG localRng(999);
    Selector::MapElites::CvtMapElitesArchive arch(
        localRng, centroids, descriptors, minV, maxV, 0 /*iter*/, dots, a1, b1,
        a2, b2);

    // Capture centroids after construction
    const auto& initCentroids = arch.getCentroids();
    ASSERT_EQ(initCentroids.size(), centroids)
        << "Centroid count mismatch after initialization.";

    for (const auto& c : initCentroids) {
        ASSERT_EQ(c.size(), descriptors);
        for (double v : c) {
            EXPECT_GE(v, minV) << "Centroid value is out of bounds.";
            EXPECT_LE(v, maxV) << "Centroid value is out of bounds.";
        }
    }

    // Now call initialize_cvt explicitly again and verify bounds are still
    // respected
    arch.initialize_cvt(localRng);
    const auto& newCentroids = arch.getCentroids();
    ASSERT_EQ(newCentroids.size(), centroids)
        << "Centroid count mismatch after re-initialization.";
    for (const auto& c : newCentroids) {
        ASSERT_EQ(c.size(), descriptors);
        for (double v : c) {
            EXPECT_GE(v, minV) << "Centroid value is out of bounds.";
            EXPECT_LE(v, maxV) << "Centroid value is out of bounds.";
        }
    }
}
