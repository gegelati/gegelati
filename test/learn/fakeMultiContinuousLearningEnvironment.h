/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2020 - 2024) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2020 - 2021)
 * Pierre-Yves Le Rolland-Raumer <plerolla@insa-rennes.fr> (2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2024)
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

#ifndef FAKE_MULTI_CONTINUOUS_LEARNING_ENVIRONMENT_H
#define FAKE_MULTI_CONTINUOUS_LEARNING_ENVIRONMENT_H

#include "data/primitiveTypeArray.h"
#include "learn/learningEnvironment.h"

/**
 * \brief Multi Continuous Learning Environment for testing purpose
 */
class FakeMultiContinuousLearningEnvironment : public Learn::LearningEnvironment
{
  protected:
    Data::PrimitiveTypeArray<int> data;
    int nbPositivesActions;
    int nbTurns;

  public:
    FakeMultiContinuousLearningEnvironment()
        : LearningEnvironment(3, false), data(3), nbTurns(0)
    {
        reset();
        this->data.setDataAt(typeid(int), 0, 1);
        this->data.setDataAt(typeid(int), 1, 2);
        this->data.setDataAt(typeid(int), 2, 3);
    };

    ~FakeMultiContinuousLearningEnvironment(){};

    void doActions(std::vector<double> actions) override
    {

        for (double act : actions) {
            if (act != 0.0) {
                nbPositivesActions++;
            }
        }

        nbTurns++;
    }

    void reset(size_t seed = 0,
               Learn::LearningMode mode = Learn::LearningMode::TRAINING,
               uint16_t iterationNumber = 0,
               uint64_t generationNumber = 0) override
    {
        nbTurns = 0;
        nbPositivesActions = 0;
    };

    std::vector<std::reference_wrapper<const Data::DataHandler>>
    getDataSources() override
    {
        std::vector<std::reference_wrapper<const Data::DataHandler>> res = {
            this->data};

        return res;
    }
    bool isTerminal() const override
    {
        // stop after a custom number of turns
        return nbTurns == 10;
    }

    double getScore() const override
    {
        return (double)nbPositivesActions;
    }

    bool isCopyable() const override
    {
        return true;
    }
};

#endif
