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

#if 0


#include "evaluation/archiveMetric.h"
#include "individual.h"
#include "learn/fakeRepresentation.h"

// Create a fake LearningEnvironment for testing purpose.
class FakeLearningEnvironment : public Evaluation::Problem
{
    Data::DataValue dataInt;
    Data::DataValue dataDouble;

    std::vector<Data::DataView> sources;

  public:
    FakeLearningEnvironment() : Evaluation::Problem(
        {Dimensions::Requirement::array1d<int>(5), Dimensions::Requirement::scalar<double>()}, Dimensions::Requirement::scalar<int>()),
        dataInt(Data::DataValue::zeros<int>(5)), dataDouble(Data::DataValue::zeros<double>()) {
            sources.push_back(dataInt.view());
            sources.push_back(dataDouble.view());
        };
    
    std::vector<Data::DataView> getDataSources() const override
    {
        return sources;
    }
    void setDataInt(std::vector<int> values) {
        dataInt.setSubValue(Data::DataValue::array1d<std::vector<int>>(values), 0);
    }
    void setDataDouble(double value) {
        dataDouble.setScalarAt<double>(value, 0);
    }
};


class ArchiveMetricTest : public ::testing::Test
{
  protected:

    Representations::FakeRepresentation rep;
    Individual* indiv;
    FakeLearningEnvironment le;

    virtual void SetUp()
    {
        indiv = new Individual(rep);
    }

    virtual void TearDown()
    {
        delete indiv;
    }
};






TEST_F(ArchiveMetricTest, Constructor)
{
    Evaluation::ArchiveMetric* metric;
    ASSERT_NO_THROW(metric = new Evaluation::ArchiveMetric(0, 1.0))
        << "Default construction of an archiveMetric failed";

    ASSERT_NO_THROW(metric->cloneEmptyUniquePtr(0)) << "Construction with cloning failed";

    ASSERT_NO_THROW(delete metric;) << "Destruction of an empty ArchiveMetric failed.";
}

TEST_F(ArchiveMetricTest, extractMetricForced)
{
    // For these test, force archivingProbability to 1
    Evaluation::ArchiveMetric metric(0, 1.0);

    // Add a fictive recording
    ASSERT_NO_THROW(metric.extractBeforeExecution(*indiv, le))
        << "Adding a recording to the empty archive failed.";

    ASSERT_EQ(metric.getInputsExtracted().size(), 1)
        << "Number or recordings in the archive is incorrect.";

    // Add other recordings with the same DataHandlers
    ASSERT_NO_THROW(metric.extractBeforeExecution(*indiv, le))
        << "Adding a recording to the non-empty archive failed.";
    ASSERT_EQ(metric.getInputsExtracted().size(), 1)
        << "Number or recordings in the archive is incorrect.";

    // Add another recording with a new environment
    // change data in one dataHandler
    le.setDataDouble(3.5);
    ASSERT_NO_THROW(metric.extractBeforeExecution(*indiv, le))
        << "Adding a recording to the non-empty archive failed.";
    ASSERT_EQ(metric.getInputsExtracted().size(), 2)
        << "Number or recordings in the archive is incorrect.";
}

TEST_F(ArchiveMetricTest, extractMetricWithProbability)
{
    // For these test, force archivingProbability to 0.5
    // Use a known seed
    Evaluation::ArchiveMetric metric(0, 0.5);

    // Add a few fictive recording
    for (int i = 0; i < 10; i++) {
        le.setDataInt({i, i, i, i, i});
        ASSERT_NO_THROW(metric.extractBeforeExecution(*indiv, le))
            << "Adding a recording to the archive failed.";
    }
    ASSERT_EQ(metric.getInputsExtracted().size(), 4)
        << "Number or recordings in the archive is incorrect with a known "
           "seed.";
}

TEST_F(ArchiveMetricTest, getInputs)
{
    // extract all
    Evaluation::ArchiveMetric metric(0, 1.0);

    le.setDataDouble(2.0);

    // Add a few fictive recording
    for (int i = 0; i < 5; i++) {
        le.setDataInt({i, i, i, i, i});
        ASSERT_NO_THROW(metric.extractBeforeExecution(*indiv, le))
            << "Adding a recording to the archive failed.";
    }

    const std::map<size_t, std::vector<std::pair<std::unique_ptr<std::byte []>, Data::DataView>>>& inputs = metric.getInputsExtracted();
    ASSERT_EQ(inputs.size(), 5) << "Metric should have extract five inputs";

    // Set learningEnv double value after to confirm copy is done.
    le.setDataDouble(3.0);
    auto it = inputs.begin();
    std::set<int> intValues;
    for (int i = 0; i < 5; i++) {
        ASSERT_EQ(it->second.size(), 2) << "Input should have two datasources";
        const Data::DataView& viewInt = it->second.at(0).second;
        const Data::DataView& viewDouble = it->second.at(1).second;

        int val = viewInt.getScalarAt<int>(0);
        for (int j = 1; j < 5; j++) {
            ASSERT_EQ(viewInt.getScalarAt<int>(j), val) << "Value should be equal to i";
        }
        intValues.insert(val);
        ASSERT_EQ(viewDouble.getScalar<double>(), 2.0) << "Value changed, copy went wrong";
        it++;
    }
    ASSERT_EQ(intValues, std::set<int>({0, 1, 2, 3, 4})) << "Set filling went wrong";
}
#endif