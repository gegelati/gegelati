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

#ifndef FAKE_CLASSIFICATION_LEARNING_ENVIRONMENT_H
#define FAKE_CLASSIFICATION_LEARNING_ENVIRONMENT_H

#include "data/primitiveTypeArray.h"

/**
 * \brief Classification Learning enviroment for testing purposes
 */
class FakeClassificationLearningEnvironment
    : public Learn::ClassificationLearningEnvironment
{
  protected:
    Data::PrimitiveTypeArray<int> data;
    int value;

  public:
    FakeClassificationLearningEnvironment()
        : ClassificationLearningEnvironment(3), data(1), value{0} {};
    void doAction(uint64_t actionId) override
    {
        // Increment classificationTable
        ClassificationLearningEnvironment::doAction(actionId);

        // Update data
        value++;
        this->currentClass = value % 3;
        data.setDataAt(typeid(int), 0, value);
    }
    void reset(size_t seed, Learn::LearningMode mode)
    {
        // Call super pure virtual method to reset the pure virtual method.
        ClassificationLearningEnvironment::reset(seed, mode);

        this->value = 0;
        this->currentClass = 0;
    };
    std::vector<std::reference_wrapper<const Data::DataHandler>>
    getDataSources()
    {
        std::vector<std::reference_wrapper<const Data::DataHandler>> vect;
        vect.push_back(data);
        return vect;
    }
    bool isTerminal() const
    {
        return false;
    }
};

#endif // !FAKE_CLASSIFICATION_LEARNING_ENVIRONMENT_H
