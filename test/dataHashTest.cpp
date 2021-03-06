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

#include "data/hash.h"

TEST(DataHashTest, HashInt)
{

    ASSERT_EQ(Data::Hash<int>()(1337), 10617510125527791323u);
    ASSERT_EQ(Data::Hash<unsigned int>()(666), 11188042650382073989u);
    ASSERT_EQ(Data::Hash<uint64_t>()(UINT64_MAX), 10157053723145373757u);
}

TEST(DataHashTest, HashDouble)
{
    ASSERT_EQ(Data::Hash<double>()(12.00), 12123769577132206109u);
    ASSERT_EQ(Data::Hash<float>()((float)3.14), 2246715419936742613u);
}

TEST(DataHashTest, HashNullptr)
{
    std::nullptr_t t = NULL;
    ASSERT_EQ(Data::Hash<std::nullptr_t>()(t), 12161962213042174405u);
}
