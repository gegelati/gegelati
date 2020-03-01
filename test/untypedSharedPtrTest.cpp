#include <gtest/gtest.h>

#include "data/untypedSharedPtr.h"

class UntypedSharedPtrTest : public ::testing::Test {
public:
	/// Base class for testing
	class Base {
	public:
		/// Default polymorphic destructor.
		virtual ~Base() = default;
	};

	/// Derived class for testing
	class Derived : public Base {};

	virtual void SetUp() {};

	virtual void TearDown() {};

	/// Empty destructor for any type
	template <typename T>
	auto emptyDestructor() { return [](T* ptr) {}; };
};


TEST_F(UntypedSharedPtrTest, ConstructorDestructor) {
	Data::UntypedSharedPtr* usp, * copy;

	// With default destructor
	ASSERT_NO_THROW(usp = new Data::UntypedSharedPtr(new Derived())) << "An exception was thrown when building the UntypedSharedPtr.";
	ASSERT_NE(usp, nullptr) << "Construction of UntypedSharedPtr with default deleter failed.";
	ASSERT_NO_THROW(delete usp) << "Deletion of UntypedSharedPtr failed.";

	// With no destructor
	Derived obj;

	ASSERT_NO_THROW(usp = new Data::UntypedSharedPtr(&obj, emptyDestructor<Derived>())) << "An exception was thrown when building the UntypedSharedPtr with a custom destructor.";
	ASSERT_NE(usp, nullptr) << "Construction of UntypedSharedPtr with empty deleter failed.";
	ASSERT_NO_THROW(delete usp) << "Deletion of UntypedSharedPtr failed.";

	// Copy constructor
	usp = new Data::UntypedSharedPtr(new Derived());
	ASSERT_NO_THROW(copy = new Data::UntypedSharedPtr(*usp)) << "Copy constructor of UntypedSharedPtr failed.";
	ASSERT_NE(copy, nullptr) << "Copy construction of UntypedSharedPtr failed.";
	ASSERT_NO_THROW(delete usp) << "Deletion of copied UntypedSharedPtr failed.";
	ASSERT_NO_THROW(delete copy) << "Deletion of copy UntypedSharedPtr failed.";
}

TEST_F(UntypedSharedPtrTest, GetType) {
	Derived* derivedPtr = new Derived();
	Base* basePtr = new Derived();
	const Derived* cderivedPtr = new Derived();
	const Base* cbasePtr = new Derived();

	// Derived pointer on derived object
	Data::UntypedSharedPtr usp0{ derivedPtr };
	ASSERT_EQ(usp0.getType(), typeid(Derived)) << "getType() method does not return the expected type_info.";
	ASSERT_EQ(usp0.getPtrType(), typeid(Derived*)) << "getPtrType() method does not return the expected type_info.";
	ASSERT_NE(usp0.getPtrType(), typeid(const Derived*)) << "getPtrType() method does not return the expected type_info.";

	// Const Derived pointer on derived object
	Data::UntypedSharedPtr usp1{ cderivedPtr };
	ASSERT_EQ(usp1.getType(), typeid(Derived)) << "getType() method does not return the expected type_info.";
	ASSERT_EQ(usp1.getPtrType(), typeid(const Derived*)) << "getPtrType() method does not return the expected type_info.";

	// Base pointer on derived object
	Data::UntypedSharedPtr usp2{ basePtr };
	ASSERT_EQ(usp2.getType(), typeid(Base)) << "getType() method does not return the expected type_info.";
	ASSERT_EQ(usp2.getPtrType(), typeid(Base*)) << "getPtrType() method does not return the expected type_info.";
	ASSERT_NE(usp2.getPtrType(), typeid(const Base*)) << "getPtrType() method does not return the expected type_info.";

	// Const Base pointer on derived object
	Data::UntypedSharedPtr usp3{ cbasePtr };
	ASSERT_EQ(usp3.getType(), typeid(Base)) << "getType() method does not return the expected type_info.";
	ASSERT_EQ(usp3.getPtrType(), typeid(const Base*)) << "getPtrType() method does not return the expected type_info.";
}

TEST_F(UntypedSharedPtrTest, getSharedPtr) {
	Derived* derivedPtr = new Derived();
	Base* basePtr = new Derived();
	const Derived* cderivedPtr = new Derived();
	const Base* cbasePtr = new Derived();

	std::shared_ptr<Derived> derivedSharedPtr;
	std::shared_ptr<const Derived> constDerivedSharedPtr;
	std::shared_ptr<Base> baseSharedPtr;

	// Derived pointer on derived object
	{ // scope
		Data::UntypedSharedPtr usp{ derivedPtr };
		ASSERT_NO_THROW(derivedSharedPtr = usp.getSharedPointer<Derived>()) << "Getting the shared pointer with the right type failed unexpectedly.";
		ASSERT_EQ(derivedSharedPtr.use_count(), 2) << "Use count of retrieved shared pointer is incorrect.";

		ASSERT_NO_THROW(constDerivedSharedPtr = usp.getSharedPointer<const Derived>()) << "Getting a shared pointer to const type should work.";
		ASSERT_EQ(derivedSharedPtr.use_count(), 3) << "Use count of retrieved shared pointer is incorrect.";
	} // usp disappears
	ASSERT_EQ(derivedSharedPtr.use_count(), 2) << "Retrieved shared pointer should be the only user after deletion of the UntypedSharedPtr";
	constDerivedSharedPtr.reset();
	derivedSharedPtr.reset();

	// Const Derived pointer on derived object
	{
		Data::UntypedSharedPtr usp{ cderivedPtr };
		ASSERT_NO_THROW(constDerivedSharedPtr = usp.getSharedPointer<const Derived>()) << "Getting a shared pointer to const type should work.";
		ASSERT_EQ(constDerivedSharedPtr.use_count(), 2) << "Use count of retrieved shared pointer is incorrect.";

		ASSERT_THROW(derivedSharedPtr = usp.getSharedPointer<Derived>(), std::runtime_error) << "Getting the shared pointer with the const type should failed.";
		ASSERT_EQ(constDerivedSharedPtr.use_count(), 2) << "Use count of retrieved shared pointer is incorrect.";
	}
	ASSERT_EQ(constDerivedSharedPtr.use_count(), 1) << "Retrieved shared pointer should be the only user after deletion of the UntypedSharedPtr";
	constDerivedSharedPtr.reset();
	derivedSharedPtr.reset();

	// Base pointer on derived object
	{
		Data::UntypedSharedPtr usp{ basePtr };
		ASSERT_NO_THROW(baseSharedPtr = usp.getSharedPointer<Base>()) << "Getting the shared pointer with the right type failed unexpectedly.";
		ASSERT_EQ(baseSharedPtr.use_count(), 2) << "Use count of retrieved shared pointer is incorrect.";

		ASSERT_THROW(derivedSharedPtr = usp.getSharedPointer<Derived>(), std::runtime_error) << "Getting the shared pointer with the actual type (Derived) from a UntypedSharedPtr built with a Base pointer should fail.";
		ASSERT_EQ(baseSharedPtr.use_count(), 2) << "Use count of retrieved shared pointer is incorrect.";

		// Conversion of previously retrieved shard pointer works though
		ASSERT_NO_THROW(derivedSharedPtr = std::dynamic_pointer_cast<Derived>(baseSharedPtr)) << "Conversion of previously retrieved Base shared pointer to an object of actual type Derived can be done.";
		ASSERT_EQ(baseSharedPtr.use_count(), 3) << "Use count of retrieved shared pointer is incorrect.";
	}
}