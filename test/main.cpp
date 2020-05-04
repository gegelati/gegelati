#include <gtest/gtest.h>

#include <functional>

#include "data/untypedSharedPtr.h"
#include "parameter.h"
#include "instructions/instruction.h"

template< typename First, typename... Rest>
class LambdaInstructionTests : public Instructions::Instruction {
private:

protected:

	/**
	* \brief Function executed for this Instruction.
	*/
	const std::function<double(First, Rest...)> func;

public:
	LambdaInstructionTests(std::function<double(First, Rest...)> function) : func{ function } {

		this->operandTypes.push_back(typeid(First));
		// Fold expression to push all other types
		(this->operandTypes.push_back(typeid(Rest)), ...);
	};

	/// Inherited from Instruction
	virtual bool checkOperandTypes(const std::vector<Data::UntypedSharedPtr>& arguments) const override {
		if (arguments.size() != this->operandTypes.size()) {
			return false;
		}

		// List of expected types
		const std::vector<std::reference_wrapper<const std::type_info>> expectedTypes{
			// First
			(!std::is_array<First>::value) ?
				typeid(First) :
				typeid(std::remove_all_extents_t<First>[]),
			(!std::is_array<Rest>::value) ?
				typeid(Rest) :
				typeid(std::remove_all_extents_t<Rest>[])... };

		for (auto idx = 0; idx < arguments.size(); idx++) {
			// Argument Type
			const std::type_info& argType = arguments.at(idx).getType();
			if (argType != expectedTypes.at(idx).get()) {
				return false;
			}
		}

		return true;
	};

	double execute(
		const std::vector<std::reference_wrapper<const Parameter>>& params,
		const std::vector<Data::UntypedSharedPtr>& args) const override {

		if (Instruction::execute(params, args) != 1.0) {
			return 0.0;
		}

		// const evaluated lambda expression are needed because type of arg will
		// not be the same if First is an array, and if it is not. 
		// Fort this reason, ternary operator can not be used.
		const auto& arg1 = [&]() {
			if constexpr (!std::is_array<First>::value) {
				return *(args.at(0).getSharedPointer<const First>());
			}
			else {
				return (args.at(0).getSharedPointer<const std::remove_all_extents_t<First>[]>()).get();
			};
		}();

		size_t i = args.size() - 1;
		// Using i-- as expansion seems to happen with parameters evaluated from right to left.
		double result = this->func(arg1,
			[&]() {
				if constexpr (!std::is_array<Rest>::value) {
					return *(args.at(i--).getSharedPointer<const Rest>());
				}
				else {
					return (args.at(i--).getSharedPointer<const std::remove_all_extents_t<Rest>[]>()).get();
				};
			}()...);
		return result;
	};

};

int main(int argc, char** argv) {
	std::cout << "Hello variadic world." << std::endl;
	std::function<double(double, double)> func = [](double d, double e) {return d; };
	std::function<double(double, double, int)> func2 = [](double d, double e, int i) {return (d + e) * i; };
	std::function<double(const double[2], double, const int[1])> func3 = [](const double d[2], double e, const int i[1]) {return (d[1] + d[0] + e) * *i; };

	LambdaInstructionTests<double, double> t(func);
	LambdaInstructionTests<double, double, int> t2(func2);
	LambdaInstructionTests<const double[2], double, const int[]> t3(func3);

	std::vector<Data::UntypedSharedPtr> vect;
	double a = 1.0;
	double b = 1.1;
	int c = 2;

	vect.emplace_back(&a, Data::UntypedSharedPtr::emptyDestructor<double>());
	vect.emplace_back(&b, Data::UntypedSharedPtr::emptyDestructor<double>());
	vect.emplace_back(&c, Data::UntypedSharedPtr::emptyDestructor<int>());
	std::cout << t2.execute({}, vect) << std::endl;

	//double tab[2] = { 1.0, 2.0 };
	std::vector<Data::UntypedSharedPtr> vect2;
	vect2.emplace_back(std::make_shared<Data::UntypedSharedPtr::Model<const double[]>>(new double[2]{ 1.0, 2.0 }));
	vect2.emplace_back(&b, Data::UntypedSharedPtr::emptyDestructor<double>());
	vect2.emplace_back(std::make_shared<Data::UntypedSharedPtr::Model<const int[]>>(new int[1]{ 2 }));
	std::cout << t3.execute({}, vect2) << std::endl;
	std::cout << "The end" << std::endl;
	//testing::InitGoogleTest(&argc, argv);
	//return RUN_ALL_TESTS();
}
