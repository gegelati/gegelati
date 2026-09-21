/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2020)
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
#include <stdexcept>

#include "selector/classificationSelectionMetrics.h"

#include "learn/evaluationResult.h"
#include "evaluation/evaluationResult.h"
#include "evaluation/scoreMetric.h"

class FakeMetric : public Evaluation::EvaluationMetric {
    public:
        FakeMetric(): Evaluation::EvaluationMetric(0) {};
        std::unique_ptr<EvaluationMetric> cloneEmptyUniquePtr(size_t seed) const {
            return std::make_unique<FakeMetric>();
        }
        std::string toString(std::string prefix = "") const {return prefix + "Fake"; }

        virtual std::type_index typeId() const noexcept {return typeid(FakeMetric); }
};

TEST(EvaluationResultTest, Constructor)
{
    Evaluation::EvaluationResult* eval1;


    ASSERT_NO_THROW(eval1 = new Evaluation::EvaluationResult())
        << "Building an EvaluationRun failed unexpectedly.";
    
        
    ASSERT_NO_THROW(delete eval1);
}

TEST(EvaluationResultTest, addEvaluationRun)
{
    std::unique_ptr<Evaluation::EvaluationMetric> scoreMetric1 = std::make_unique<Evaluation::ScoreMetric>(0, 1.0);
    std::unique_ptr<Evaluation::EvaluationMetric> scoreMetric2 = std::make_unique<Evaluation::ScoreMetric>(1, 2.0);
    std::unique_ptr<Evaluation::EvaluationMetric> scoreMetric3 = std::make_unique<Evaluation::ScoreMetric>(2, 3.0);

    std::unique_ptr<Evaluation::EvaluationMetric> fake1 = std::make_unique<FakeMetric>();
    std::unique_ptr<Evaluation::EvaluationMetric> fake2 = std::make_unique<FakeMetric>();

    Evaluation::EvaluationResult result;

    
    ASSERT_EQ(result.getSize(), 0) << "Wrong size";
    ASSERT_FALSE(result.hasTypeIndex(typeid(Evaluation::ScoreMetric))) << "Should be false";
    ASSERT_FALSE(result.hasTypeIndex(typeid(FakeMetric))) << "Should be false";

    ASSERT_NO_THROW(result.addEvaluationMetric(std::move(scoreMetric1)));
    ASSERT_EQ(result.getSize(), 1) << "Wrong size";
    ASSERT_TRUE(result.hasTypeIndex(typeid(Evaluation::ScoreMetric))) << "Should be true";
    ASSERT_FALSE(result.hasTypeIndex(typeid(FakeMetric))) << "Should be false";

    ASSERT_NO_THROW(result.addEvaluationMetric(std::move(scoreMetric2)));
    ASSERT_NO_THROW(result.addEvaluationMetric(std::move(scoreMetric3)));
    ASSERT_EQ(result.getSize(), 1) << "Wrong size";
    ASSERT_TRUE(result.hasTypeIndex(typeid(Evaluation::ScoreMetric))) << "Should be true";
    ASSERT_FALSE(result.hasTypeIndex(typeid(FakeMetric))) << "Should be false";

    ASSERT_THROW(result.getEvaluationMetricsAt(typeid(FakeMetric)), std::runtime_error);
    ASSERT_NO_THROW(result.getEvaluationMetricsAt(typeid(Evaluation::ScoreMetric)));
    
    ASSERT_NO_THROW(result.addEvaluationMetric(std::move(fake1)));
    ASSERT_NO_THROW(result.addEvaluationMetric(std::move(fake2)));
    ASSERT_TRUE(result.hasTypeIndex(typeid(Evaluation::ScoreMetric))) << "Should be true";
    ASSERT_TRUE(result.hasTypeIndex(typeid(FakeMetric))) << "Should be true";
    ASSERT_EQ(result.getSize(), 2) << "Wrong size";

    const std::vector<std::unique_ptr<Evaluation::EvaluationMetric>>& scoreMetrics = result.getEvaluationMetricsAt(typeid(Evaluation::ScoreMetric));
    ASSERT_EQ(scoreMetrics.size(), 3);
    for(size_t idx = 0; idx < 3; idx++) {

        ASSERT_EQ(scoreMetrics.at(idx)->getSeed(), idx);
        const Evaluation::ScoreMetric* scoreMetric = dynamic_cast<const Evaluation::ScoreMetric*>(scoreMetrics.at(idx).get());
        ASSERT_FALSE(scoreMetric == nullptr);
        ASSERT_EQ(scoreMetric->getScore(), double(idx + 1));
    }

    const std::vector<std::unique_ptr<Evaluation::EvaluationMetric>>& fakeMetrics = result.getEvaluationMetricsAt(typeid(FakeMetric));
    ASSERT_EQ(fakeMetrics.size(), 2);

    ASSERT_NO_THROW(result.toString()) << "For coverage";

    std::cout<<result.toString()<<std::endl;
}