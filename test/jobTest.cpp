/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2020) :
 *
 * Pierre-Yves Le Rolland-Raumer <plerolla@insa-rennes.fr> (2020)
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

#include "learn/learningAgent.h"
#include "learn/learningEnvironment.h"

/*
TEST(JobTest, Constructor)
{
    Learn::Job* job = nullptr;
    Learn::Job* job2 = nullptr;

    EvoGraph::Vertex* tpg = nullptr;

    ASSERT_NO_THROW(job = new Learn::Job(tpg))
        << "Construction of the Job failed.";

    ASSERT_EQ(0, job->getIdx()) << "Parameter idx did not have expected value "
                                   "after calling job constructor.";
    ASSERT_EQ(0, job->getArchiveSeed())
        << "Parameter archiveSeed did not have expected value after calling "
           "job constructor.";

    ASSERT_NO_THROW(job2 = new Learn::Job(tpg, 3, 2))
        << "Construction of the Job failed.";

    ASSERT_EQ(2, job2->getIdx()) << "Parameter idx did not have expected value "
                                    "after calling job constructor.";
    ASSERT_EQ(3, job2->getArchiveSeed())
        << "Parameter archiveSeed did not have expected value after calling "
           "job constructor.";

    ASSERT_NO_THROW(delete job) << "Destruction of the Job failed.";
    ASSERT_NO_THROW(delete job2) << "Destruction of the Job failed.";
}

TEST(JobTest, getRoot)
{
    Learn::Job* job2 = nullptr;

    EvoGraph::Vertex* tpg = nullptr;

    job2 = new Learn::Job(tpg);

    ASSERT_EQ(tpg, (*job2).getRoot())
        << "The root from getRoot is not the good one";

    delete job2;
}
*/