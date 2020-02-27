#include "data/primitiveType.h"

/**
* \brief Classification Learning enviroment for testing purposes
*/
class FakeClassificationLearningEnvironment : public Learn::ClassificationLearningEnvironment {
protected:
	Data::PrimitiveTypeArray<int> data;
	int value;

public:
	FakeClassificationLearningEnvironment() : ClassificationLearningEnvironment(3), data(1), value{ 0 } {};
	void doAction(uint64_t actionId) override {
		// Increment classificationTable
		ClassificationLearningEnvironment::doAction(actionId);

		// Update data
		value++;
		this->currentClass = value % 3;
		data.setDataAt(typeid(Data::PrimitiveType<int>), 0, value);
	}
	void reset(size_t seed, Learn::LearningMode mode) {
		// Call super pure virtual method to reset the pure virtual method.
		ClassificationLearningEnvironment::reset(seed, mode);

		this->value = 0;
		this->currentClass = 0;
	};
	std::vector<std::reference_wrapper<const Data::DataHandler>> getDataSources() {
		std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
		vect.push_back(data);
		return vect;
	}
	bool isTerminal() const { return false; }
};