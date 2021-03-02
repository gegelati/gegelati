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

#include "data/primitiveTypeArray2D.h"

TEST(PrimitiveTypeArray2DTest, Constructors)
{
    Data::PrimitiveTypeArray2D<double>*array, *array2;
    ASSERT_NE(array = new Data::PrimitiveTypeArray2D<double>(3, 4), nullptr)
        << "A PrimitiveTypeArray2D<double> could not be built successfully.";

    // Clone test
    ASSERT_NE(array2 = (Data::PrimitiveTypeArray2D<double>*)array->clone(),
              nullptr)
        << "Cloning an existing array failed unexpectedly.";

    // ID equality
    ASSERT_EQ(array->getId(), array2->getId())
        << "Clone array ID differs from the cloned one.";

    ASSERT_NO_THROW(delete array)
        << "PrimitiveTypeArray2D could not be deleted.";

    ASSERT_NO_THROW(delete array2)
        << "PrimitiveTypeArray2D could not be deleted.";
}

TEST(PrimitiveTypeArray2DTest, getDataAt)
{
    const size_t h = 3;
    const size_t w = 5;
    Data::PrimitiveTypeArray2D<int> a(w, h);

    // Fill the array
    for (auto idx = 0; idx < h * w; idx++) {
        a.setDataAt(typeid(int), idx, idx);
    }

    // Check primitive type provided by 1D array
    for (auto idx = 0; idx < h * w; idx++) {
        const int val =
            *((a.getDataAt(typeid(int), idx)).getSharedPointer<const int>());
        ASSERT_EQ(val, idx) << "Value with primitive type is not as expected.";
    }

    // Check 1D array
    for (auto idx = 0; idx < a.getAddressSpace(typeid(int[3])); idx++) {
        std::shared_ptr<const int> valSPtr =
            (a.getDataAt(typeid(int[3]), idx)).getSharedPointer<const int[]>();
        const int* valPtr = valSPtr.get();
        for (auto subIdx = 0; subIdx < 3; subIdx++) {
            const int val = valPtr[subIdx];
            ASSERT_EQ(val, (idx / (w - 3 + 1) * w + idx % (w - 3 + 1)) + subIdx)
                << "Value with primitive type is not as expected.";
        }
    }

    // Check 2D array (returned as a 1D array)
    for (auto idx = 0; idx < a.getAddressSpace(typeid(int[3][2])); idx++) {
        std::shared_ptr<const int> valSPtr =
            (a.getDataAt(typeid(int[2][3]), idx))
                .getSharedPointer<const int[]>();
        const int(*valPtr)[3] = (int(*)[3])valSPtr.get();
        size_t srcIdx = idx / (w - 3 + 1) * w + idx % (w - 3 + 1);
        for (auto subH = 0; subH < 2; subH++) {
            for (auto subW = 0; subW < 3; subW++) {
                const int val = valPtr[subH][subW];
                ASSERT_EQ(val, srcIdx + (subH * w) + subW)
                    << "Value with primitive type is not as expected.";
            }
        }
    }

#ifndef NDEBUG
    ASSERT_THROW(a.getDataAt(typeid(int[h * w]), 1), std::invalid_argument)
        << "Address exceeding the addressSpace should cause an exception.";

    ASSERT_THROW(a.getDataAt(typeid(int[w - 1]), h * (w - 1) + 1),
                 std::out_of_range)
        << "Address exceeding the addressSpace should cause an exception.";
#else
    // No alternative test to put here.. out of range access to memory _may_
    // happen without being detected.
#endif
}
