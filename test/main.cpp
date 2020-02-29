#include <gtest/gtest.h>
#include <typeinfo>
#include <vector>
#include <memory>
#include <any>
#include <array>
#include <exception>

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
	template <typename T, class Deleter = std::default_delete<T>>
	Object(T* obj, Deleter func = std::default_delete<T>()) : object(std::make_shared<Model<T>>(obj, func)) {};

	const std::type_info& getType() const {
		return object->getType();
	}

	const std::type_info& getPtrType() const {
		return object->getPtrType();
	}

	template<typename T>
	std::shared_ptr<T> getSharedPointer() const {
		const auto& tT = typeid(T*);
		const auto& templateTypeNoConst = typeid(std::remove_const_t<T>*);
		const auto& oT = this->getPtrType();

		// If pointer types are identical (which includes const qualifier), go for it.
		if (oT == tT) {
			std::shared_ptr< const Model<T>> typedPtr = std::dynamic_pointer_cast<const Model<T>> (this->object);
			return typedPtr->object;
		}
		// If pointer types are identical when loosing const qualifier of function template type, go for it.
		else if (oT == templateTypeNoConst) {
			std::shared_ptr< const Model<std::remove_const_t<T>>> typedPtr = std::dynamic_pointer_cast<const Model<std::remove_const_t<T>>> (this->object);
			return typedPtr->object;
		}
		else {
			// Type mismatch
			std::string msg("Cannot convert ");
			msg.append(oT.name());
			msg.append(" into ");
			msg.append(tT.name());
			msg.append(".");

			throw std::runtime_error(msg);
		}
	}

	struct Concept {
		virtual ~Concept() {};
		virtual const std::type_info& getType() const = 0;
		virtual const std::type_info& getPtrType() const = 0;
	};

	template< typename T>
	struct Model : Concept {
	public:
		template <typename Deleter>
		Model(T* t, Deleter func) : object(t, func) {}

		const std::type_info& getType() const override {
			return typeid(*object);
		}

		const std::type_info& getPtrType() const override {
			return typeid(object.get());
		}


		std::shared_ptr<T> object;
	};

	std::shared_ptr<const Concept> object;
};

int main(int argc, char** argv) {
	int a = 2;

	std::function<void(const int*)> del{ [](const int*) {} };

	std::vector<Object> vect;

	std::shared_ptr<int>PTR(new int(5));

	std::shared_ptr<const int>cPTR(PTR);

	std::shared_ptr<A>AAA(new A(1000));
	std::shared_ptr<B>BBB(std::dynamic_pointer_cast<B>(AAA));

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

	Object o((int*)&a, del);
	const int b = 3;
	std::cout << ">>>" << o.getType().name() << " - " << typeid(b).name() << std::endl;
	try {
		auto aptr = o.getSharedPointer<const int>();
		std::cout << "P:" << typeid(aptr).name() << " - " << *aptr << std::endl;
	}
	catch (const std::runtime_error & a) {
		std::cout << a.what();
		return 255;
	}


	//*aptr = 3;

	std::cout << a << "<<<" << std::endl;

	for (auto o : vect) {
		std::cout << o.getType().name() << std::endl;
	}

	vect.at(4).getSharedPointer<B>()->print();
	return 0;
	//testing::InitGoogleTest(&argc, argv);
	//return RUN_ALL_TESTS();
}
