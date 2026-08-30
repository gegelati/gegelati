/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
 * Thomas Bourgoin <tbourgoi@insa-rennes.fr> (2021)
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

#include <gtest/gtest.h>
#include <memory>

#include "data/dataHandler.h"
#include "data/primitiveTypeArray.h"

#include "evaluation/archiveMetric.h"
#include "evolution/individual.h"

// Create a fake LearningEnvironment for testing purpose.
class FakeLearningEnvironment : public Evaluation::LearningEnvironment
{
    Data::PrimitiveTypeArray<int> dataInt;
    Data::PrimitiveTypeArray<double> dataDouble;

  public:
    FakeLearningEnvironment() : LearningEnvironment(1), dataInt(1), dataDouble(1) {};
    
    std::vector<std::reference_wrapper<const Data::DataHandler>> getDataSources() const override
    {
        std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
        vect.push_back(dataInt);
        vect.push_back(dataDouble);
        return vect;
    }
    void setDataDouble(double value) {
        dataDouble.setDataAt(typeid(double), 0, value);
    }
    void setDataInt(int value) {
        dataInt.setDataAt(typeid(int), 0, value);
    }

    double getScore() const override {return 0.0;}
};


class ArchiveMetricTest : public ::testing::Test
{
  protected:

    Evolution::Individual indiv;
    FakeLearningEnvironment le;

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};






TEST_F(ArchiveMetricTest, Constructor)
{
    Evaluation::ArchiveMetric* metric;
    ASSERT_NO_THROW(metric = new Evaluation::ArchiveMetric(1.0))
        << "Default construction of an archiveMetric failed";

    ASSERT_NO_THROW(metric->cloneEmptyUniquePtr()) << "Construction with cloning failed";

    ASSERT_NO_THROW(delete metric;) << "Destruction of an empty ArchiveMetric failed.";
}

TEST_F(ArchiveMetricTest, CombineHash)
{
    size_t hash;

    ASSERT_NO_THROW(hash = Evaluation::ArchiveMetric::getCombinedHash(le.getDataSources()))
        << "Combination of several DataHandler hash failed.";

    // change data in one dataHandler
    le.setDataDouble(2.0);

    // Compare hashes.
    ASSERT_NE(Evaluation::ArchiveMetric::getCombinedHash(le.getDataSources()), hash) << "Hashes should be different";
}

TEST_F(ArchiveMetricTest, extractMetricForced)
{
    // For these test, force archivingProbability to 1
    Evaluation::ArchiveMetric metric(1.0);

    // Initialize the metric with known seed
    ASSERT_NO_THROW(metric.initMetrics(indiv, le, 0)) << "Initialize the metric with seed 0 failed";

    // Add a fictive recording
    ASSERT_NO_THROW(metric.extractMetricsStep(indiv, {}, le))
        << "Adding a recording to the empty archive failed.";

    ASSERT_EQ(metric.getInputsExtracted().size(), 1)
        << "Number or recordings in the archive is incorrect.";

    // Add other recordings with the same DataHandlers
    ASSERT_NO_THROW(metric.extractMetricsStep(indiv, {}, le))
        << "Adding a recording to the non-empty archive failed.";
    ASSERT_EQ(metric.getInputsExtracted().size(), 1)
        << "Number or recordings in the archive is incorrect.";

    // Add another recording with a new environment
    // change data in one dataHandler
    le.setDataDouble(3.5);
    ASSERT_NO_THROW(metric.extractMetricsStep(indiv, {}, le))
        << "Adding a recording to the non-empty archive failed.";
    ASSERT_EQ(metric.getInputsExtracted().size(), 2)
        << "Number or recordings in the archive is incorrect.";
}

TEST_F(ArchiveMetricTest, extractMetricWithProbability)
{
    // For these test, force archivingProbability to 0.5
    // Use a known seed
    Evaluation::ArchiveMetric metric(0.5);

    // Initialize the metric with known seed
    ASSERT_NO_THROW(metric.initMetrics(indiv, le, 0)) << "Initialize the metric with seed 0 failed";

    // Add a few fictive recording
    for (int i = 0; i < 10; i++) {
        le.setDataInt(i);
        ASSERT_NO_THROW(metric.extractMetricsStep(indiv, {}, le))
            << "Adding a recording to the archive failed.";
    }
    ASSERT_EQ(metric.getInputsExtracted().size(), 4)
        << "Number or recordings in the archive is incorrect with a known "
           "seed.";
}

TEST_F(ArchiveMetricTest, getInputs)
{
    // extract all
    Evaluation::ArchiveMetric metric(1.0);

    // Initialize the metric with known seed
    ASSERT_NO_THROW(metric.initMetrics(indiv, le, 0)) << "Initialize the metric with seed 0 failed";

    le.setDataDouble(2.0);

    // Add a few fictive recording
    for (int i = 0; i < 5; i++) {
        le.setDataInt(i);
        ASSERT_NO_THROW(metric.extractMetricsStep(indiv, {}, le))
            << "Adding a recording to the archive failed.";
    }

    const std::map<size_t, std::vector<std::reference_wrapper<const Data::DataHandler>>>& inputs = metric.getInputsExtracted();

    ASSERT_EQ(inputs.size(), 5) << "Metric should have extract five inputs";

    // Set learningEnv double value after to confirm copy is done.
    le.setDataDouble(3.0);
    auto it = inputs.begin();
    for (int i = 0; i < 5; i++) {
        ASSERT_EQ(it->second.size(), 2) << "Input should have two datasources";
        ASSERT_EQ(it->second.at(0).get().getDataAt(typeid(int), 0).getScalar<int>(), i) << "Input should have two datasources";
        ASSERT_EQ(it->second.at(1).get().getDataAt(typeid(double), 0).getScalar<double>(), 2.0) << "Input should have two datasources";
        it++;
    }
}
