#include <gtest/gtest.h>
#include <vector>
#include <memory>
#include <any>
#include <array>

class A {
public:
	A(int i) :att{ i } {

	}

	virtual void print()const {
		std::cout << "A" << (this->att) << std::endl;
	}
	virtual ~A() {
		std::cout << "~A" << this->att << std::endl;
	}
	int att;
};

class B : public A {
public:
	B(int i) : A(i) {};
	virtual void print() const {
		std::cout << "B" << (this->att) << std::endl;
	}
	virtual ~B() {
		std::cout << "~B" << A::att << std::endl;
	}
};

class Object {

public:
	template <typename T, class Deleter = std::default_delete<const T>>
	Object(const T* obj, Deleter func = std::default_delete<const T>()) : object(std::make_shared<Model<T>>(obj, func)) {};

	const std::type_info& getType() const {
		return object->getType();
	}

	template<typename T>
	std::shared_ptr<const T> getSharedPointer() const {
		std::shared_ptr< const Model<T>> typedPtr = std::dynamic_pointer_cast<const Model<T>> (this->object);

		return typedPtr->object;
	}

	struct Concept {
		virtual ~Concept() {};
		virtual const std::type_info& getType() const = 0;
	};

	template< typename T>
	struct Model : Concept {
	public:
		template <typename Deleter>
		Model(const T* t, Deleter func) : object(t, func) {}

		const std::type_info& getType() const override {
			return typeid(*object);
		}

		std::shared_ptr<const T> object;
	};

	std::shared_ptr<const Concept> object;
};

int main(int argc, char** argv) {
	int a = 2;

	std::function<void(const int*)> del{ [](const int*) {} };

	std::vector<Object> vect;

	vect.emplace_back(&a, del);
	vect.emplace_back(new int(2));
	vect.emplace_back(new A(1));
	vect.emplace_back(new B(2));
	vect.emplace_back(new B(3), [](const B*) {});
	vect.emplace_back(new std::array<B, 2>{ 4, 5 });

	{
		Object ooo(new B(6));
		vect.push_back(ooo); // survive in vect.
	}

	Object o(&a, del);
	auto aptr = o.getSharedPointer<int>();
	std::cout << "P:" << typeid(aptr).name() << " - " << *aptr << std::endl;

	for (auto o : vect) {
		std::cout << o.getType().name() << std::endl;
	}

	vect.at(4).getSharedPointer<B>()->print();

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
