#ifndef GEGELATI_FAKEIMPROVEDCLASSIFICATIONLEARNINGENVIRONMENT_H
#define GEGELATI_FAKEIMPROVEDCLASSIFICATIONLEARNINGENVIRONMENT_H

#include "data/primitiveTypeArray.h"
#include "learn/improvedClassificationLearningEnvironment.h"

/**
 * \brief Classification Learning enviroment for testing purposes
 */
class FakeImprovedClassificationLearningEnvironment
    : public Learn::ImprovedClassificationLearningEnvironment
{
  protected:
    Data::PrimitiveTypeArray<int> data;
    int value;

  public:
    FakeImprovedClassificationLearningEnvironment()
        : ImprovedClassificationLearningEnvironment(3, 81), data(1), value{0} {};
    void doAction(uint64_t actionId) override
    {
        // Increment classificationTable
        ImprovedClassificationLearningEnvironment::doAction(actionId);

        // Update data
        value++;
        this->_currentClass = value % 3;
        data.setDataAt(typeid(int), 0, value);
    }
    void reset(size_t seed, Learn::LearningMode mode) override
    {
        // Call super pure virtual method to reset the pure virtual method.
        ImprovedClassificationLearningEnvironment::reset(seed, mode);

        this->value = 0;
        this->_currentClass = 0;
    };
    std::vector<std::reference_wrapper<const Data::DataHandler>>
    getDataSources() override
    {
        std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
        vect.push_back(data);
        return vect;
    }
    bool isTerminal() const override
    {
        return false;
    }
};

#endif // GEGELATI_FAKEIMPROVEDCLASSIFICATIONLEARNINGENVIRONMENT_H
