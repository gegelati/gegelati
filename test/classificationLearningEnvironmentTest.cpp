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

#include "learn/learningEnvironment.h"
#include "learn/stickGameWithOpponent.h"
#include "learn/classificationLearningEnvironment.h"
#include "learn/fakeClassificationLearningEnvironment.h"


TEST(ClassificationLearningEnvironmentTest, Constructor) {
	Learn::ClassificationLearningEnvironment* le = NULL;

	ASSERT_NO_THROW(le = new FakeClassificationLearningEnvironment()) << "Construction of the classification Learning Environment failed";

	ASSERT_NO_THROW(delete le) << "Destruction of the Learning Environment failed";
}

TEST(ClassificationLearningEnvironmentTest, resetDoActionGetClassificationTableGetScore) {
	FakeClassificationLearningEnvironment fle;

	auto actions = { 0, 2, 2, 1, 0, 1, 2, 0, 0, 1, 1, 1, 2, 2, 2, 1, 1, 2 };

	// reset
	ASSERT_NO_THROW(fle.reset(0, Learn::LearningMode::TRAINING)) << "resetting the ClassificationLearningEnvironment failed";

	// doActions
	for (auto action : actions) {
		ASSERT_NO_THROW(fle.doAction(action)) << "doAction failed within a known ClassificationLearningEnvironment.";
	}

	// getClassif table
	const std::vector<std::vector<uint64_t>>* classifTable = NULL;
	ASSERT_NO_THROW(classifTable = &fle.getClassificationTable()) << "Getting the classificationTable failed.";

	// check its content with known actions
	int table[3][3] = {
		{1, 3, 2},
		{2, 2, 2},
		{1, 2, 3}
	};

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			ASSERT_EQ(classifTable->at(i).at(j), table[i][j]) << "Classification table contains unexpected values with known actions.";
		}
	}

	// Get average f1 score
	ASSERT_NEAR(fle.getScore(), 0.323077, 0.000001) << "Score of the ClassificationLearningEnvironment is not as expected with known actions.";
}
