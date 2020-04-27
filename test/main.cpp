#include <gtest/gtest.h>

#include <functional>

template< typename First, typename... Rest>
class LambdaInstructionTests {
private:

protected:
	/// TO BE REMOVED: Inherited
	std::vector<std::reference_wrapper<const std::type_info>> operandTypes;

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

};

int main(int argc, char** argv) {
	std::cout << "Hello variadic world." << std::endl;
	std::function<double(double, double)> func = [](double d, double e) {return d; };
	std::function<double(double, double, int)> func2 = [](double d, double e, int i) {return d; };
	LambdaInstructionTests<double, double> t(func);
	LambdaInstructionTests<double, double, int> t2(func2);

	// testing::InitGoogleTest(&argc, argv);
	// return RUN_ALL_TESTS();
}
