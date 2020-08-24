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

#include "learn/adversarialJob.h"
#include "learn/job.h"
#include "learn/learningAgent.h"
#include "learn/learningEnvironment.h"

TEST(JobTest, Constructor)
{
    Learn::Job* job = nullptr;
    Learn::Job* job2 = nullptr;
    Learn::AdversarialJob* job3 = nullptr;
    Learn::AdversarialJob* job4 = nullptr;

    TPG::TPGVertex* tpg = nullptr;

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

    ASSERT_NO_THROW(job3 = new Learn::AdversarialJob({tpg}, 3, 2,4))
        << "Construction of the AdversarialJob failed.";
    ASSERT_EQ(1, job3->getSize()) << "Size did not have expected value after "
                                     "calling adversarialJob constructor.";
    ASSERT_EQ(4, job3->getPosOfStudiedRoot()) << "posOfStudiedRoot did not have "
                                                 "expected value after "
                                     "calling adversarialJob constructor.";

    ASSERT_NO_THROW(job4 = new Learn::AdversarialJob({tpg}))
                                << "Construction of the AdversarialJob failed.";
    ASSERT_EQ(-1, job4->getPosOfStudiedRoot()) << "posOfStudiedRoot did not have "
                                                  "expected default value after "
                                                 "calling adversarialJob constructor.";

    ASSERT_NO_THROW(delete job) << "Destruction of the Job failed.";
    ASSERT_NO_THROW(delete job2) << "Destruction of the Job failed.";
    ASSERT_NO_THROW(delete job3) << "Destruction of the AdversarialJob failed.";
    ASSERT_NO_THROW(delete job4) << "Destruction of the AdversarialJob failed.";
}

TEST(JobTest, addRoot)
{
    Learn::AdversarialJob* job = nullptr;

    TPG::TPGVertex* tpg = nullptr;
    TPG::TPGVertex* tpg2 = nullptr;

    job = new Learn::AdversarialJob({tpg});

    ASSERT_EQ(1, job->getSize()) << "The job doesn't have the right size.";

    job->addRoot(tpg2);

    ASSERT_EQ(2, job->getSize())
        << "The job doesn't have the right size after a root add.";

    delete job;
}

TEST(JobTest, getRootsAndRoot)
{
    Learn::AdversarialJob* job = nullptr;
    Learn::Job* job2 = nullptr;

    TPG::TPGVertex* tpg = nullptr;
    // we define a non-null tpg to check the content of the job later
    TPG::TPGVertex* tpg2 = new TPG::TPGAction(0);

    job = new Learn::AdversarialJob({tpg});
    job2 = new Learn::Job(tpg);

    auto roots = job->getRoots();
    ASSERT_EQ(job->getSize(), roots.size())
        << "The job doesn't contain as many roots as its size attribute";
    ASSERT_EQ(tpg, roots[0]);

    job->addRoot(tpg2);

    roots = job->getRoots();
    ASSERT_EQ(job->getSize(), roots.size())
        << "The job doesn't contain as many roots as its size attribute";

    ASSERT_EQ(tpg, roots[0]) << "The first root is not the good one";
    ASSERT_EQ(tpg2, roots[1]) << "The second root is not the good one";
    ASSERT_EQ(tpg, (*job).getRoot())
        << "The root from getRoot is not the good one";
    ASSERT_EQ(tpg, (*job2).getRoot())
        << "The root from getRoot is not the good one";

    delete tpg2;
    delete job;
    delete job2;
}

TEST(JobTest, operatorGet)
{
    Learn::AdversarialJob* job = nullptr;

    TPG::TPGVertex* tpg = nullptr;
    // we define a non-null tpg to check the content of the job later
    TPG::TPGVertex* tpg2 = new TPG::TPGAction(0);

    job = new Learn::AdversarialJob({tpg, tpg2});

    ASSERT_EQ(tpg, (*job)[0]) << "The first root is not the good one";
    ASSERT_EQ(tpg2, (*job)[1]) << "The second root is not the good one";

    delete tpg2;
    delete job;
}
