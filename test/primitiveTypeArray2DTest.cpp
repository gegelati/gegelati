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
    Data::PrimitiveTypeArray2D<double>* array;
    ASSERT_NE(array = new Data::PrimitiveTypeArray2D<double>(3, 4), nullptr)
        << "A PrimitiveTypeArray2D<double> could not be built successfully.";

    ASSERT_NO_THROW(delete array)
        << "PrimitiveTypeArray2D could not be deleted.";
}

TEST(PrimitiveTypeArray2DTest, Clone)
{
    // Create a DataHandler
    const size_t width{8}, height{3};
    const size_t address{3};
    const double doubleValue{42.0};
    std::vector<double> values(width * height);

    // create a first one to increase the DataHandler::count
    Data::PrimitiveTypeArray2D<int> d0(12, 10);
    Data::PrimitiveTypeArray2D<double> d(width, height);
    // change the content of the array
    d.setDataAt(typeid(double), address, doubleValue);
    // Hash was voluntarily not computed before clone.

    // Create a clone
    Data::DataHandler* dClone = NULL;
    ASSERT_NO_THROW(dClone = d.clone();)
        << "Cloning a PrimitiveTypeArray2D<double> failed.";

    // Extra if to remove warnings on further use of dClone.
    if (dClone == NULL)
        FAIL() << "Cloning of PrimitiveTypeArray2D returned a NULL Pointer.";

    // Check ID
    ASSERT_EQ(dClone->getId(), d.getId())
        << "Cloned and original PrimitiveTypeArray2D do not have the same ID "
           "as "
           "expected.";
    // Check the polymorphic type.
    ASSERT_EQ(typeid(*dClone), typeid(Data::PrimitiveTypeArray2D<double>))
        << "Type of clone PrimitiveTypeArray2D is not a PrimitiveTypeArray2D "
           "as "
           "expected.";
    // Compute the hashes
    ASSERT_EQ(dClone->getHash(), d.getHash())
        << "Hash of clone and original DataHandler differ.";

    // Change data in the original to make sure the two dHandlers are decoupled.
    size_t hash = dClone->getHash();
    d.setDataAt(typeid(double), address + 1, doubleValue + 1.0);
    ((Data::PrimitiveTypeArray2D<double>*)dClone)->invalidateCachedHash();
    ASSERT_NE(dClone->getHash(), d.getHash())
        << "Hash of clone and original DataHandler should differ after "
           "modification of data in the original.";
    ASSERT_EQ(dClone->getHash(), hash)
        << "Hash of the clone dataHandler should remain unchanged after "
           "modification of data within the original DataHandler.";
    delete dClone;

    // Check nullptr clone also
    d.setPointer(nullptr);
    ASSERT_NO_THROW(dClone = d.clone())
        << "Cloning an Array2DWrapper pointing to a nullptr should not fail.";
    if (dClone == NULL)
        FAIL() << "Cloning of Array2DWrapper returned a NULL Pointer.";
    delete dClone;
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

TEST(PrimitiveTypeArray2DTest, PrimitiveDataArray2DAssignmentOperator)
{
    // Create a DataHandler
    const size_t size{4};
    auto d = new Data::PrimitiveTypeArray2D<int>(size, size); // 4x4

    // Fill the array
    d->resetData();
    for (auto idx = 0; idx < size * size; idx++) {
        d->setDataAt(typeid(int), idx, idx);
    }

    // Create another DataHandler with the same size
    auto d2 = new Data::PrimitiveTypeArray2D<int>(size, size);
    // Create another DataHandler with a different size
    auto d3 = new Data::PrimitiveTypeArray2D<int>(size - 1, size - 1);

    // Check that assignment do not throw std::domain_error
    ASSERT_NO_THROW(*d2 = *d)
        << "Assigning PrimitiveTypeArray2D with valid size and type failed.";

    // Check that data was successfully copied.
    for (auto idx = 0; idx < size * size; idx++) {
        ASSERT_EQ(
            (int)*(
                d2->getDataAt(typeid(int), idx).getSharedPointer<const int>()),
            idx)
            << "Previously set data did not persist.";
    }

    // Check that a wrong assignment throw std::domain_error
    ASSERT_THROW(*d3 = *d, std::domain_error)
        << "Assigning PrimitiveTypeArray2D with invalid size did not throw "
           "domain_error.";

    delete d, d2, d3;
}
