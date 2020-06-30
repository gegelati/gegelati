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

#include "data/untypedSharedPtr.h"

class UntypedSharedPtrTest : public ::testing::Test
{
  public:
    /// Base class for testing
    class Base
    {
      public:
        /// Default polymorphic destructor.
        virtual ~Base() = default;
    };

    /// Derived class for testing
    class Derived : public Base
    {
    };

    virtual void SetUp(){};

    virtual void TearDown(){};
};

TEST_F(UntypedSharedPtrTest, ConstructorDestructorClassical)
{
    Data::UntypedSharedPtr *usp, *copy;

    // With default destructor
    ASSERT_NO_THROW(usp = new Data::UntypedSharedPtr(new Derived()))
        << "An exception was thrown when building the UntypedSharedPtr.";
    ASSERT_NE(usp, nullptr)
        << "Construction of UntypedSharedPtr with default deleter failed.";
    ASSERT_NO_THROW(delete usp) << "Deletion of UntypedSharedPtr failed.";

    // With no destructor
    Derived obj;

    ASSERT_NO_THROW(
        usp = new Data::UntypedSharedPtr(
            &obj, Data::UntypedSharedPtr::emptyDestructor<Derived>()))
        << "An exception was thrown when building the UntypedSharedPtr with a "
           "custom destructor.";
    ASSERT_NE(usp, nullptr)
        << "Construction of UntypedSharedPtr with empty deleter failed.";
    ASSERT_NO_THROW(delete usp) << "Deletion of UntypedSharedPtr failed.";

    // Copy constructor
    usp = new Data::UntypedSharedPtr(new Derived());
    ASSERT_NO_THROW(copy = new Data::UntypedSharedPtr(*usp))
        << "Copy constructor of UntypedSharedPtr failed.";
    ASSERT_NE(copy, nullptr) << "Copy construction of UntypedSharedPtr failed.";
    ASSERT_NO_THROW(delete usp)
        << "Deletion of copied UntypedSharedPtr failed.";
    ASSERT_NO_THROW(delete copy) << "Deletion of copy UntypedSharedPtr failed.";
}

TEST_F(UntypedSharedPtrTest, ConstructorDestructorFromModel)
{
    Data::UntypedSharedPtr* usp;

    // Construct a shared_ptr to a Concept
    std::shared_ptr<Data::UntypedSharedPtr::Concept> spConcept;
    ;
    ASSERT_NO_THROW(
        spConcept =
            std::make_shared<Data::UntypedSharedPtr::Model<int[]>>(new int[3]))
        << "Building a shared_pointer to a Model<cstyle array> failed.";
    ASSERT_NE(spConcept, nullptr)
        << "Building a shared_pointer to a Model<cstyle array> failed.";

    // Construct the UntypedSharedPtr
    ASSERT_NO_THROW(usp = new Data::UntypedSharedPtr(spConcept))
        << "Building an UntypedSharedPtr from an existing shared pointer to a "
           "Concept failed.";
    ASSERT_NE(usp, nullptr) << "Building an UntypedSharedPtr from an existing "
                               "shared pointer to a Concept failed.";
    ASSERT_NO_THROW(delete usp)
        << "Deleting the UntypedSharedPtr built from a concept failed.";
}

TEST_F(UntypedSharedPtrTest, GetType)
{
    Derived* derivedPtr = new Derived();
    Base* basePtr = new Derived();
    const Derived* cderivedPtr = new Derived();
    const Base* cbasePtr = new Derived();

    // Derived pointer on derived object
    Data::UntypedSharedPtr usp0{derivedPtr};
    ASSERT_EQ(usp0.getType(), typeid(Derived))
        << "getType() method does not return the expected type_info.";
    ASSERT_EQ(usp0.getPtrType(), typeid(Derived*))
        << "getPtrType() method does not return the expected type_info.";
    ASSERT_NE(usp0.getPtrType(), typeid(const Derived*))
        << "getPtrType() method does not return the expected type_info.";

    // Const Derived pointer on derived object
    Data::UntypedSharedPtr usp1{cderivedPtr};
    ASSERT_EQ(usp1.getType(), typeid(Derived))
        << "getType() method does not return the expected type_info.";
    ASSERT_EQ(usp1.getPtrType(), typeid(const Derived*))
        << "getPtrType() method does not return the expected type_info.";

    // Base pointer on derived object
    Data::UntypedSharedPtr usp2{basePtr};
    ASSERT_EQ(usp2.getType(), typeid(Base))
        << "getType() method does not return the expected type_info.";
    ASSERT_EQ(usp2.getPtrType(), typeid(Base*))
        << "getPtrType() method does not return the expected type_info.";
    ASSERT_NE(usp2.getPtrType(), typeid(const Base*))
        << "getPtrType() method does not return the expected type_info.";

    // Const Base pointer on derived object
    Data::UntypedSharedPtr usp3{cbasePtr};
    ASSERT_EQ(usp3.getType(), typeid(Base))
        << "getType() method does not return the expected type_info.";
    ASSERT_EQ(usp3.getPtrType(), typeid(const Base*))
        << "getPtrType() method does not return the expected type_info.";
}

TEST_F(UntypedSharedPtrTest, getTypesOnBuiltFromConcept)
{
    // getType and getPtrType on UntypedSharedPtr built from a
    // shared_ptr<Concept>
    Data::UntypedSharedPtr usp{
        std::make_shared<Data::UntypedSharedPtr::Model<double[]>>(
            new double[5])};

    ASSERT_EQ(usp.getType(), typeid(double[]));
    ASSERT_EQ(usp.getPtrType(), typeid(double*));

    // For comparison only, the type returned with the classical constructor
    Data::UntypedSharedPtr usp2{new double[5]};
    ASSERT_EQ(usp2.getType(), typeid(double))
        << "getType() for an UntypedSharedPointer built from a Model<double[]> "
           "should return double[]";
    ASSERT_EQ(usp2.getPtrType(), typeid(double*))
        << "getPtrType() for an UntypedSharedPointer built from a "
           "Model<double[]> should return double*";
}

TEST_F(UntypedSharedPtrTest, getSharedPtr)
{
    Derived* derivedPtr = new Derived();
    Base* basePtr = new Derived();
    const Derived* cderivedPtr = new Derived();
    const Base* cbasePtr = new Derived();

    std::shared_ptr<Derived> derivedSharedPtr;
    std::shared_ptr<const Derived> constDerivedSharedPtr;
    std::shared_ptr<Base> baseSharedPtr;

    // Derived pointer on derived object
    { // scope
        Data::UntypedSharedPtr usp{derivedPtr};
        ASSERT_NO_THROW(derivedSharedPtr = usp.getSharedPointer<Derived>())
            << "Getting the shared pointer with the right type failed "
               "unexpectedly.";
        ASSERT_EQ(derivedSharedPtr.use_count(), 2)
            << "Use count of retrieved shared pointer is incorrect.";

        ASSERT_NO_THROW(constDerivedSharedPtr =
                            usp.getSharedPointer<const Derived>())
            << "Getting a shared pointer to const type should work.";
        ASSERT_EQ(derivedSharedPtr.use_count(), 3)
            << "Use count of retrieved shared pointer is incorrect.";
    } // usp disappears
    ASSERT_EQ(derivedSharedPtr.use_count(), 2)
        << "Retrieved shared pointer should be the only user after deletion of "
           "the UntypedSharedPtr";
    constDerivedSharedPtr.reset();
    derivedSharedPtr.reset();

    // Const Derived pointer on derived object
    {
        Data::UntypedSharedPtr usp{cderivedPtr};
        ASSERT_NO_THROW(constDerivedSharedPtr =
                            usp.getSharedPointer<const Derived>())
            << "Getting a shared pointer to const type should work.";
        ASSERT_EQ(constDerivedSharedPtr.use_count(), 2)
            << "Use count of retrieved shared pointer is incorrect.";

        ASSERT_THROW(derivedSharedPtr = usp.getSharedPointer<Derived>(),
                     std::runtime_error)
            << "Getting the shared pointer with the const type should failed.";
        ASSERT_EQ(constDerivedSharedPtr.use_count(), 2)
            << "Use count of retrieved shared pointer is incorrect.";
    }
    ASSERT_EQ(constDerivedSharedPtr.use_count(), 1)
        << "Retrieved shared pointer should be the only user after deletion of "
           "the UntypedSharedPtr";
    constDerivedSharedPtr.reset();
    derivedSharedPtr.reset();

    // Base pointer on derived object
    {
        Data::UntypedSharedPtr usp{basePtr};
        ASSERT_NO_THROW(baseSharedPtr = usp.getSharedPointer<Base>())
            << "Getting the shared pointer with the right type failed "
               "unexpectedly.";
        ASSERT_EQ(baseSharedPtr.use_count(), 2)
            << "Use count of retrieved shared pointer is incorrect.";

        ASSERT_THROW(derivedSharedPtr = usp.getSharedPointer<Derived>(),
                     std::runtime_error)
            << "Getting the shared pointer with the actual type (Derived) from "
               "a UntypedSharedPtr built with a Base pointer should fail.";
        ASSERT_EQ(baseSharedPtr.use_count(), 2)
            << "Use count of retrieved shared pointer is incorrect.";

        // Conversion of previously retrieved shard pointer works though
        ASSERT_NO_THROW(derivedSharedPtr =
                            std::dynamic_pointer_cast<Derived>(baseSharedPtr))
            << "Conversion of previously retrieved Base shared pointer to an "
               "object of actual type Derived can be done.";
        ASSERT_EQ(baseSharedPtr.use_count(), 3)
            << "Use count of retrieved shared pointer is incorrect.";
    }
}

TEST_F(UntypedSharedPtrTest, getSharedPtrOnBuiltFromConcept)
{

    { // non-const tests
        auto msp = std::make_shared<Data::UntypedSharedPtr::Model<double[]>>(
            new double[5]);
        Data::UntypedSharedPtr usp{msp};
        std::shared_ptr<double[]> dataPtr;
        std::shared_ptr<const double[]> cdataPtr;
        ASSERT_NO_THROW(dataPtr = usp.getSharedPointer<double[]>())
            << "Getting the shared pointer to the original template parameter "
               "of the model failed.";
        ASSERT_EQ(dataPtr.use_count(), 2)
            << "Retrieved shared_ptr use count is incorrect.";
        ASSERT_NO_THROW(cdataPtr = usp.getSharedPointer<const double[]>())
            << "Getting the const shared pointer to the original template "
               "parameter of the model failed.";
        ASSERT_EQ(cdataPtr.use_count(), 3)
            << "Retrieved shared_ptr use count is incorrect.";
    }

    // const tests
    {
        auto cmsp =
            std::make_shared<Data::UntypedSharedPtr::Model<const double[]>>(
                new double[5]);
        Data::UntypedSharedPtr cusp{cmsp};
        std::shared_ptr<double[]> dataPtr;
        std::shared_ptr<const double[]> cdataPtr;
        ASSERT_NO_THROW(cdataPtr = cusp.getSharedPointer<const double[]>())
            << "Getting the shared pointer to the original template parameter "
               "of the model failed.";
        ASSERT_EQ(cdataPtr.use_count(), 2)
            << "Retrieved shared_ptr use count is incorrect.";
        ASSERT_THROW(dataPtr = cusp.getSharedPointer<double[]>(),
                     std::runtime_error)
            << "Getting a non-const pointer to orignally const data should "
               "fail.";
    }
}
