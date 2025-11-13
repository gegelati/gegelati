#include <gtest/gtest.h>
#include "instructions/addPrimitiveType.h"
#include "instructions/lambdaInstruction.h"
#include "selector/mapElites/mapElitesSelector.h"
#include "learn/fakeMultiContinuousLearningEnvironment.h"

#include "selector/mapElites/mapElitesDefaultDescriptors.h"


class MapElitesSelectorTest : public ::testing::Test {
protected:
    const size_t size1{24};
    const size_t size2{32};
    Mutator::RNG rng{42};
    Learn::LearningParameters params;
    std::shared_ptr<TPG::TPGGraph> graph;
    Environment* e = NULL;
    Instructions::Set set;
    std::vector<std::reference_wrapper<const Data::DataHandler>> vect;

    Selector::MapElites::MapElitesSelector* selector;
    std::shared_ptr<Selector::MapElites::DefaultDescriptors::ActionValues> descriptor;

    void SetUp() override {
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
        selector = new Selector::MapElites::MapElitesSelector(graph, params);
        descriptor = std::make_shared<Selector::MapElites::DefaultDescriptors::ActionValues>();

        // Minimal init for descriptor
        FakeMultiContinuousLearningEnvironment dummyEnv;
        descriptor->initDescriptor(*graph, dummyEnv);
    }

    void TearDown() override {
        delete selector;
    }
};

TEST_F(MapElitesSelectorTest, AddGridArchiveAndRetrieve) {
    FakeMultiContinuousLearningEnvironment le;
    selector->addArchiveFromDescriptor(10, descriptor, le);

    auto archive = selector->getMapElitesArchiveAt(descriptor);
    ASSERT_NE(archive, nullptr);
}

TEST_F(MapElitesSelectorTest, AddCvtArchiveAndRetrieve) {
    FakeMultiContinuousLearningEnvironment le;
    selector->addCvtArchiveFromDescriptor(15, descriptor, le, rng, 5, 20);

    auto archive = selector->getMapElitesArchiveAt(descriptor);
    ASSERT_NE(archive, nullptr);
}

TEST_F(MapElitesSelectorTest, GetArchiveThrowsIfMissing) {
    EXPECT_THROW(selector->getMapElitesArchiveAt(descriptor), std::runtime_error);
}

TEST_F(MapElitesSelectorTest, GetMapElitesArchivesInitiallyEmpty) {
    EXPECT_TRUE(selector->getMapElitesArchives().empty());
}

TEST_F(MapElitesSelectorTest, GetMapElitesArchivesAfterInsert) {
    FakeMultiContinuousLearningEnvironment le;
    selector->addArchiveFromDescriptor(8, descriptor, le);

    const auto& archives = selector->getMapElitesArchives();
    ASSERT_EQ(archives.size(), 1u);
    EXPECT_EQ(archives.begin()->first, descriptor);
}


TEST_F(MapElitesSelectorTest, CreateSelectionMetricsThrows) {
    EXPECT_THROW(selector->createSelectionMetrics(), std::runtime_error);
}

TEST_F(MapElitesSelectorTest, CreateSelectionMetrics) {
    FakeMultiContinuousLearningEnvironment le;
    auto d2 = std::make_shared<Selector::MapElites::DefaultDescriptors::ActionValues>(); // NOT initialized
    selector->addArchiveFromDescriptor(5, d2, le);

    EXPECT_THROW(selector->createSelectionMetrics(), std::runtime_error);
}

TEST_F(MapElitesSelectorTest, CreateSelectionMetricsReturnsValidObject) {
    FakeMultiContinuousLearningEnvironment le;
    selector->addArchiveFromDescriptor(5, descriptor, le);

    auto metrics = selector->createSelectionMetrics();
    ASSERT_NE(metrics, nullptr);

    auto casted = std::dynamic_pointer_cast<Selector::MapElites::MapElitesSelectionMetrics>(metrics);
    ASSERT_NE(casted, nullptr);
}

TEST_F(MapElitesSelectorTest, DoSelectionNoCrashOnEmptyResults) {
    FakeMultiContinuousLearningEnvironment le;
    selector->addArchiveFromDescriptor(5, descriptor, le);

    std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*> results;
    EXPECT_NO_THROW(selector->doSelection(results, rng));
}

TEST_F(MapElitesSelectorTest, DoSelectionCrashWrongMetrics) {
    FakeMultiContinuousLearningEnvironment le;
    selector->addArchiveFromDescriptor(5, descriptor, le);

    std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*> results;
    
    auto metrics = std::make_shared<Selector::SelectionMetrics>(100.0);
    auto eval = std::make_shared<Learn::EvaluationResult>(metrics, 1u);
    const TPG::TPGVertex* root = &graph->addNewTeam();
    results.insert({eval, root});
    EXPECT_THROW(selector->doSelection(results, rng), std::runtime_error);
}


TEST_F(MapElitesSelectorTest, DoSelectionInserts) {
    // Prepare a learning env and add a grid archive for our descriptor
    FakeMultiContinuousLearningEnvironment le;
    selector->addArchiveFromDescriptor(10, descriptor, le);

    // Retrieve the archive (grid)
    auto archive = selector->getMapElitesArchiveAt(descriptor);
    ASSERT_NE(archive, nullptr);

    // Build two MapElitesSelectionMetrics with different scores.
    // Use the constructor taking scores to set the selection score deterministically.
    auto metrics_high = std::make_shared<Selector::MapElites::MapElitesSelectionMetrics>(100.0);
    auto metrics_low  = std::make_shared<Selector::MapElites::MapElitesSelectionMetrics>(1.0);

    // Prepare two descriptor vectors that will map to two *different* archive cells.
    // Use descriptor->getNbDescriptors() to get right size.
    size_t nbDesc = descriptor->getNbDescriptors();
    ASSERT_GT(nbDesc, 0u);

    std::vector<double> desc_vec_low(nbDesc, 0.1);  // will fall in low bins
    std::vector<double> desc_vec_high(nbDesc, 0.9); // will fall in high bins (different cell)

    // Inject mapDescriptors entries into metrics (test-only: use const_cast to mutate).
    // This ensures getMapDescriptors().at(descriptor) will return the vector we want.
    {
        auto &map_low = const_cast<std::map<std::shared_ptr<const Selector::MapElites::MapElitesDescriptor>, std::vector<double>>&>(
            metrics_low->getMapDescriptors());
        map_low[descriptor] = desc_vec_low;
    }
    {
        auto &map_high = const_cast<std::map<std::shared_ptr<const Selector::MapElites::MapElitesDescriptor>, std::vector<double>>&>(
            metrics_high->getMapDescriptors());
        map_high[descriptor] = desc_vec_high;
    }

    // Build EvaluationResult objects wrapping those metrics.
    // Use constructor (metrics, nbEvaluation) like in your other tests.
    auto eval_high = std::make_shared<Learn::EvaluationResult>(metrics_high, 1u);
    auto eval_low  = std::make_shared<Learn::EvaluationResult>(metrics_low,  1u);

    // Fake (but validly non-null) root pointers. We won't dereference them.
    const TPG::TPGVertex* root_high = &graph->addNewTeam();
    const TPG::TPGVertex* root_low  = &graph->addNewTeam();

    // Build the results multimap.
    // (doSelection processes results in reverse iteration order; for grid cells being different
    // both results will be inserted irrespective of ordering)
    std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*> results;
    results.insert({eval_high, root_high});
    results.insert({eval_low,  root_low});

    // Run the selection — should insert both results into two distinct archive cells.
    EXPECT_NO_THROW(selector->doSelection(results, rng));

    // After selection, check archive stored the correct evaluation+root for each descriptor vector
    const auto &stored_high = archive->getArchiveFromDescriptors(desc_vec_high);
    const auto &stored_low  = archive->getArchiveFromDescriptors(desc_vec_low);

    // They should match the ones we passed in.
    EXPECT_EQ(stored_high.first, eval_high);
    EXPECT_EQ(stored_high.second, root_high);

    EXPECT_EQ(stored_low.first, eval_low);
    EXPECT_EQ(stored_low.second, root_low);


    EXPECT_NO_THROW(const auto &ctx = selector->updateContext());

    // Check that clonable teams are correctly identified
    EXPECT_EQ(selector->updateContext().teamsClonable.size(), 2);
}

TEST_F(MapElitesSelectorTest, DoSelectionReplaces) {
    // Test the replacement logic on same descriptor cell:
    FakeMultiContinuousLearningEnvironment le;
    selector->addArchiveFromDescriptor(10, descriptor, le);
    auto archive = selector->getMapElitesArchiveAt(descriptor);
    ASSERT_NE(archive, nullptr);

    size_t nbDesc = descriptor->getNbDescriptors();
    std::vector<double> desc_vec(nbDesc, 0.5); // same cell for both results

    // First insert a low-scoring evaluation directly in the archive
    auto metrics_existing = std::make_shared<Selector::MapElites::MapElitesSelectionMetrics>(1.0);
    {
        auto &map_existing = const_cast<std::map<std::shared_ptr<const Selector::MapElites::MapElitesDescriptor>, std::vector<double>>&>(
            metrics_existing->getMapDescriptors());
        map_existing[descriptor] = desc_vec;
    }
    auto eval_existing = std::make_shared<Learn::EvaluationResult>(metrics_existing, 1u);
    const TPG::TPGVertex* root_existing = &graph->addNewTeam();

    // Put the existing (low) value into the archive (simulating prior generation).
    // Note: archive is returned as shared_ptr<const MapElitesArchive>, but our addArchiveFromDescriptor constructed a non-const MapElitesArchive
    // and getMapElitesArchiveAt returns shared_ptr<const MapElitesArchive>. We need a non-const pointer to set the archive.
    // We re-fetch the non-const archive from selector's internal map (safe test-only access via getMapElitesArchives()).
    auto &archives_map = selector->getMapElitesArchives();
    auto it = archives_map.find(descriptor);
    ASSERT_NE(it, archives_map.end());
    auto nonConstArchive = it->second; // shared_ptr<MapElitesArchive>
    nonConstArchive->setArchiveFromDescriptors(root_existing, eval_existing, desc_vec);

    // Now build a new, higher-scoring evaluation that should replace the existing one
    auto metrics_new = std::make_shared<Selector::MapElites::MapElitesSelectionMetrics>(200.0);
    {
        auto &map_new = const_cast<std::map<std::shared_ptr<const Selector::MapElites::MapElitesDescriptor>, std::vector<double>>&>(
            metrics_new->getMapDescriptors());
        map_new[descriptor] = desc_vec;
    }
    auto eval_new = std::make_shared<Learn::EvaluationResult>(metrics_new, 1u);
    const TPG::TPGVertex* root_new = &graph->addNewTeam();

    std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*> results;
    results.insert({eval_new,  root_new});
    results.insert({eval_existing, root_existing}); // include the old eval as well (it will not survive)

    // Run selection: since both map to the same descriptor, the new higher-score should replace the old one.
    EXPECT_NO_THROW(selector->doSelection(results, rng));

    // Verify that the archive contains the new evaluation (and vertex)
    const auto stored = nonConstArchive->getArchiveFromDescriptors(desc_vec);
    EXPECT_EQ(stored.first, eval_new);
    EXPECT_EQ(stored.second, root_new);

    EXPECT_NO_THROW(const auto &ctx = selector->updateContext());

    // Check that clonable team is correctly identified
    EXPECT_EQ(selector->updateContext().teamsClonable.size(), 1);
}

TEST_F(MapElitesSelectorTest, TwoArchives) {
    FakeMultiContinuousLearningEnvironment le;

    // We need a second descriptor instance for the second archive
    auto descriptorB = std::make_shared<Selector::MapElites::DefaultDescriptors::ActionValues>();
    descriptorB->initDescriptor(*graph, le);

    // Create two archives
    selector->addArchiveFromDescriptor(10, descriptor,  le); // Archive A
    selector->addArchiveFromDescriptor(10, descriptorB, le); // Archive B

    auto archiveA = selector->getMapElitesArchiveAt(descriptor);
    auto archiveB = selector->getMapElitesArchiveAt(descriptorB);
    ASSERT_NE(archiveA, nullptr);
    ASSERT_NE(archiveB, nullptr);

    size_t nA = descriptor->getNbDescriptors();
    size_t nB = descriptorB->getNbDescriptors();
    ASSERT_GT(nA, 0u);
    ASSERT_GT(nB, 0u);

    // --- Descriptor vectors per "intended cell" ---
    std::vector<double> cell1_A(nA, 0.0); 
    std::vector<double> cell1_B(nB, 1.0);  
    std::vector<double> cell2_A(nA, 1.0);  
    std::vector<double> cell2_B(nB, 0.0);  
    std::vector<double> cellNoneA(nA, 0.0);
    std::vector<double> cellNoneB(nB, 0.0);

    std::vector<double> descriptorUsed;

    // --- Build metrics for 4 maps ---
    std::map<std::shared_ptr<const Selector::MapElites::MapElitesDescriptor>, std::vector<double>> mapA = {
        {descriptor,  cell1_A}, {descriptorB, cell1_B}
    };
    std::map<std::shared_ptr<const Selector::MapElites::MapElitesDescriptor>, std::vector<double>> mapB = {
        {descriptor,  cell2_A}, {descriptorB, cell2_B}
    };
    std::map<std::shared_ptr<const Selector::MapElites::MapElitesDescriptor>, std::vector<double>> mapAB = {
        {descriptor,  cell2_A}, {descriptorB, cell1_B}
    };
    std::map<std::shared_ptr<const Selector::MapElites::MapElitesDescriptor>, std::vector<double>> mapNone = {
        {descriptor,  cellNoneA}, {descriptorB, cellNoneB}
    };


    const TPG::TPGVertex* vA    = &graph->addNewAction(0);
    const TPG::TPGVertex* vB    = &graph->addNewAction(1);
    const TPG::TPGVertex* vAB   = &graph->addNewAction(2);
    const TPG::TPGVertex* vNone = &graph->addNewAction(3);

    auto mA    = std::make_shared<Selector::MapElites::MapElitesSelectionMetrics>(50.0, mapA);
    auto mB    = std::make_shared<Selector::MapElites::MapElitesSelectionMetrics>(50.0, mapB);
    auto mAB    = std::make_shared<Selector::MapElites::MapElitesSelectionMetrics>(100.0, mapAB);
    auto mNone    = std::make_shared<Selector::MapElites::MapElitesSelectionMetrics>(5.0, mapNone);

    auto eA    = std::make_shared<Learn::EvaluationResult>(mA, 1u);
    auto eB    = std::make_shared<Learn::EvaluationResult>(mB, 1u);
    auto eAB   = std::make_shared<Learn::EvaluationResult>(mAB, 1u);
    auto eNone = std::make_shared<Learn::EvaluationResult>(mNone, 1u);

    // Insert in results
    std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*> results;
    results.insert({eA,    vA});
    results.insert({eB,    vB});
    results.insert({eAB,   vAB});
    results.insert({eNone, vNone});

    // Run selection
    ASSERT_NO_THROW(selector->doSelection(results, rng));

    // --- Verify archive contents ---

    // A should contain vA and vAB
    EXPECT_EQ(archiveA->getArchiveFromDescriptors(cell1_A).second, vA);
    EXPECT_EQ(archiveA->getArchiveFromDescriptors(cell2_A).second, vAB);

    // B should contain vB and vAB
    EXPECT_EQ(archiveB->getArchiveFromDescriptors(cell1_B).second, vAB);
    EXPECT_EQ(archiveB->getArchiveFromDescriptors(cell2_B).second, vB);

    // --- Verify archive sizes (each archive has exactly 2 entries) ---
    EXPECT_EQ(archiveA->getVerticesInArchive().size(), 2u);
    EXPECT_EQ(archiveB->getVerticesInArchive().size(), 2u);

    // --- Verify clonable count ---
    auto ctx = selector->updateContext();
    EXPECT_EQ(ctx.teamsClonable.size(), 3u); // vA, vB, vAB

    EXPECT_EQ(graph->getNbRootVertices(), 3u); // vA, vB, vAB
}
