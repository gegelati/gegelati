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


	double execute(
		const std::vector<std::reference_wrapper<const Parameter>>& params,
		const std::vector<Data::UntypedSharedPtr>& args) const override {

		if (Instruction::execute(params, args) != 1.0) {
			return 0.0;
		}

		const First& arg1 = *(args.at(0).getSharedPointer<const First>());

		size_t i = args.size() - 1;
		// Using i-- as expansion seems to happen with parameters evaluated from right to left.
		double result = this->func(arg1, (*(args.at(i--).getSharedPointer<const Rest>().get()))...);
		return result;
	};

};

int main(int argc, char** argv) {
	std::cout << "Hello variadic world." << std::endl;
	std::function<double(double, double)> func = [](double d, double e) {return d; };
	std::function<double(double, double, int)> func2 = [](double d, double e, int i) {return (d + e) * i; };
	LambdaInstructionTests<double, double> t(func);
	LambdaInstructionTests<double, double, int> t2(func2);

	std::vector<Data::UntypedSharedPtr> vect;
	double a = 1.0;
	double b = 1.1;
	int c = 2;
	vect.emplace_back(&a, Data::UntypedSharedPtr::emptyDestructor<double>());
	vect.emplace_back(&b, Data::UntypedSharedPtr::emptyDestructor<double>());
	vect.emplace_back(&c, Data::UntypedSharedPtr::emptyDestructor<int>());
	t2.execute({}, vect);

	//testing::InitGoogleTest(&argc, argv);
	//return RUN_ALL_TESTS();
}
