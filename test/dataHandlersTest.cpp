/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2021) :
 *
 * Cedric Leonard <cleonard@insa-rennes.fr> (2021)
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2021)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2020)
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

#include "data/dataHandler.h"
#include "data/primitiveTypeArray.h"

TEST(DataHandlersTest, Constructor)
{
    ASSERT_NO_THROW({
        Data::DataHandler* d = new Data::PrimitiveTypeArray<double>();
        delete d;
    }) << "Call to PrimitiveTypeArray constructor failed.";
}

TEST(DataHandlersTest, ID)
{
    Data::PrimitiveTypeArray<double> d0;
    Data::PrimitiveTypeArray<int> d1;

    ASSERT_NE(d0.getId(), d1.getId()) << "Id of two DataHandlers created one "
                                         "after the other should not be equal.";
}

TEST(DataHandlersTest, PrimitiveDataArrayGetDataAtNativeType)
{
    const size_t size{32};
    Data::DataHandler* d = new Data::PrimitiveTypeArray<float>(size);

    d->resetData();
    for (int i = 0; i < size; i++) {
        const float a =
            *(d->getDataAt(typeid(float), i).getSharedPointer<const float>());
        ASSERT_EQ((float)a, 0.0f)
            << "Data at valid address and type can not be accessed.";
    }

#ifndef NDEBUG
    ASSERT_THROW(d->getDataAt(typeid(float), size), std::out_of_range)
        << "Address exceeding the addressSpace should cause an exception.";
#else
    // No alternative test to put here.. out of range access to memory _may_
    // happen without being detected.
#endif

#ifndef NDEBUG
    ASSERT_THROW(d->getDataAt(typeid(double), 0), std::invalid_argument)
        << "Requesting a non-handled type, even at a valid location, should "
           "cause an exception.";
#else
    ASSERT_THROW(
        d->getDataAt(typeid(double), 0).getSharedPointer<const double>(),
        std::out_of_range)
        << "In NDEBUG mode, a pointer with invalid type will be returned when "
           "requesting a non-handled type, even at a valid location.";
#endif

    delete d;
}

TEST(DataHandlersTest, PrimitiveDataArrayGetDataAtArray)
{
    const size_t size{8};
    const size_t sizeArray = 3;
    Data::PrimitiveTypeArray<int>* d = new Data::PrimitiveTypeArray<int>(size);

    // Fill the array
    for (auto idx = 0; idx < size; idx++) {
        d->setDataAt(typeid(int), idx, idx);
    }

    // Get data as arrays
    for (int i = 0; i < size - sizeArray + 1; i++) {
        std::shared_ptr<const int> sptr =
            d->getDataAt(typeid(int[sizeArray]), i)
                .getSharedPointer<const int[]>();
        const int* a = (sptr.get());
        ASSERT_NE(a, nullptr) << "Retrieved data is a null_ptr";
        for (int idx = 0; idx < sizeArray; idx++) {
            ASSERT_EQ(a[idx], i + idx)
                << "Value given in the array do not correspond to the one "
                   "stored in the array.";
        }
    }

#ifndef NDEBUG
    ASSERT_THROW(d->getDataAt(typeid(int[sizeArray]), size - 1),
                 std::out_of_range)
        << "Address exceeding the addressSpace should cause an exception.";
#else
    // No alternative test to put here.. out of range access to memory _may_
    // happen without being detected.
#endif

#ifndef NDEBUG
    ASSERT_THROW(d->getDataAt(typeid(long[sizeArray]), 0),
                 std::invalid_argument)
        << "Requesting a non-handled type, even at a valid location, should "
           "cause an exception.";
#else
    ASSERT_THROW(d->getDataAt(typeid(long[sizeArray]), 0)
                     .getSharedPointer<const long[sizeArray]>(),
                 std::out_of_range)
        << "In NDEBUG mode, a pointer with invalid type will be returned when "
           "requesting a non-handled type, even at a valid location.";
#endif

    delete d;
}

TEST(DataHandlersTest, PrimitiveDataArraySetDataAt)
{
    const size_t size{8};
    const size_t address{3};
    const double doubleValue{42.0};
    Data::PrimitiveTypeArray<double>* d =
        new Data::PrimitiveTypeArray<double>(size);

    d->resetData();
    double value(doubleValue);
    ASSERT_NO_THROW(d->setDataAt(typeid(value), address, value))
        << "Setting data with valid Address and type failed.";

    // Check that data was indeed updated.
    ASSERT_EQ((double)*(d->getDataAt(typeid(double), address)
                            .getSharedPointer<const double>()),
              doubleValue)
        << "Previously set data did not persist.";

    delete d;
}

TEST(DataHandlersTest, PrimitiveDataArrayHash)
{
    // Create a DataHandler
    const size_t size{8};
    const size_t address{3};
    const double doubleValue{42.0};

    Data::PrimitiveTypeArray<double> d(size);

    // Get hash
    size_t hash = 0;
    ASSERT_NO_THROW(hash = d.getHash());
    // change the content of the array
    d.setDataAt(typeid(double), address, doubleValue);
    ASSERT_NE(hash, d.getHash());
}

TEST(DataHandlersTest, PrimitiveDataArrayClone)
{
    // Create a DataHandler
    const size_t size{8};
    const size_t address{3};
    const double doubleValue{42.0};

    // create a first one to increase the DataHandler::count
    Data::PrimitiveTypeArray<int> d0(12);
    Data::PrimitiveTypeArray<double> d(size);
    // change the content of the array
    d.setDataAt(typeid(double), address, doubleValue);
    // Hash was voluntarily not computed before clone.

    // Create a clone
    Data::DataHandler* dClone = NULL;
    ASSERT_NO_THROW(dClone = d.clone();)
        << "Cloning a PrimitiTypeArray<double> failed.";

    // Extra if to remove warnings on further use of dClone.
    if (dClone == NULL)
        FAIL() << "Cloning of DataHandler returned a NULL Pointer.";

    // Check ID
    ASSERT_EQ(dClone->getId(), d.getId())
        << "Cloned and original dataHandler do not have the same ID as "
           "expected.";
    // Check the polymorphic type.
    ASSERT_EQ(typeid(*dClone), typeid(Data::PrimitiveTypeArray<double>))
        << "Type of clone DataHandler differes from the original one.";
    // Compute the hashes
    ASSERT_EQ(dClone->getHash(), d.getHash())
        << "Hash of clone and original DataHandler differ.";

    // Change data in the original to make sure the two dHandlers are decoupled.
    size_t hash = dClone->getHash();
    d.resetData();
    ASSERT_NE(dClone->getHash(), d.getHash())
        << "Hash of clone and original DataHandler should differ after "
           "modification of data in the original.";
    ASSERT_EQ(dClone->getHash(), hash)
        << "Hash of the clone dataHandler should remain unchanged after "
           "modification of data within the original DataHandler.";
}

TEST(DataHandlersTest, PrimitiveDataArrayAssignmentOperator)
{
    // Create a DataHandler
    const size_t size{8};
    Data::PrimitiveTypeArray<int>* d = new Data::PrimitiveTypeArray<int>(size);

    // Fill the array
    d->resetData();
    for (auto idx = 0; idx < size; idx++) {
        d->setDataAt(typeid(int), idx, idx);
    }

    // Create another DataHandler with the same size
    Data::PrimitiveTypeArray<int>* d2 = new Data::PrimitiveTypeArray<int>(size);
    // Create another DataHandler with a different size
    Data::PrimitiveTypeArray<int>* d3 =
        new Data::PrimitiveTypeArray<int>(size - 1);

    // Check that assignment do not throw std::domain_error
    ASSERT_NO_THROW(*d2 = *d)
        << "Assigning PrimitiveTypeArray with valid size and type failed.";

    // Check that data was successfully copied.
    for (auto idx = 0; idx < size; idx++) {
        ASSERT_EQ(
            (int)*(
                d2->getDataAt(typeid(int), idx).getSharedPointer<const int>()),
            idx)
            << "Previously set data did not persist.";
    }

    // Check that a wrong assignment throw std::domain_error
    ASSERT_THROW(*d3 = *d, std::domain_error)
        << "Assigning PrimitiveTypeArray with invalid size did not throw "
           "domain_error.";

    delete d, d2, d3;
}
