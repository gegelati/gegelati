/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2025)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2024 - 2025)
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

#ifndef LEARNING_ENVIRONMENT_H
#define LEARNING_ENVIRONMENT_H

#include "newData/dataValue.h"
#include "newData/dataRequirement.h"
#include <cstdint>
#include <vector>

namespace Evaluation {

    /**
     * \brief Different modes in which the LearningEnvironment can be reset.
     *
     * Each of the following mode corresponds to a classical phase of a learning
     * process. These mode usually refer to different parts of the data set used
     * throughout the learning process. Classically, the TRAINING mode is used
     * to effectively train an agent. The VALIDATION mode is used to evaluate
     * the efficiency of the learning process during the training phase, but on
     * data differring from the one used for training, in order to avoid biased
     * evaluation. TESTING mode is used at the end of all training activity to
     * evaluate the efficiency of the agent on completely new data.
     */
    enum class LearningMode
    {
        TRAINING,
        VALIDATION,
        TESTING
    };


    /**
     * \brief Interface for creating a Learning Environment.
     *
     * This class defines all the method that should be implemented for a
     * Learner to interact with an learning environment and learn to interact
     * with it.
     *
     * Interaction with a learning environment are made through a discrete set
     * of actions. As a result of these actions, the learning environment may
     * update its state, accessible through the data sources it provides. The
     * learning environment also provides a score resulting from the past
     * actions, and a termination boolean indicating that the
     * learningEnvironment has reached a final state, that no action will
     * affect.
     */
    class LearningEnvironment
    {
      protected:

        /// Input dimensions
        std::vector<Data::DataRequirement> inputDimensions;

        /// Output dimension
        Data::DataRequirement outputDimension;

        /// Make the default copy constructor protected.
        LearningEnvironment(const LearningEnvironment& other) = default;

      public:
        /**
         * \brief Delete the default constructor of a LearningEnvironment.
         */
        LearningEnvironment() = delete;

        /// Default virtual destructor
        virtual ~LearningEnvironment() = default;

        /**
         * \brief Constructor for LearningEnviroment.
         * 
         * \param[in] inputDimensions the dimensions of the input sources.
         * \param[in] outputDimension the dimensions of the output source.
         */
        LearningEnvironment(const std::vector<Data::DataRequirement>& inputDimensions, const Data::DataRequirement& outputDimension) : inputDimensions(inputDimensions), outputDimension(outputDimension) {};

        /**
         * \brief Get a copy of the LearningEnvironment.
         *
         * Default implementation returns a null pointer.
         *
         * \return a copy of the LearningEnvironment if it is copyable,
         * otherwise this method returns a NULL pointer.
         */
        virtual LearningEnvironment* clone() const;

        /**
         * \brief Can the LearningEnvironment be copy constructed to evaluate
         * several LearningAgent in parallel.
         *
         * \return true if the LearningEnvironment can be copied and run in
         * parallel. Default implementation returns false.
         */
        virtual bool isCopyable() const;


        /**
         * \brief get the input dimensions of the LearningEnvironment.
         */
        virtual const std::vector<Data::DataRequirement>& getInputDimensions() const;

        /**
         * \brief get the output dimension of the LearningEnvironment.
         */
        virtual const Data::DataRequirement& getOutputDimension() const;

        /**
         * \brief Get the data sources for this LearningEnvironment.
         *
         * This method returns a vector of reference to the DataHandler that
         * will be given to the LearningAgent, and to its Program to learn how
         * to interact with the LearningEnvironment. Throughout the existence
         * of the LearningEnvironment, data contained in the data will be
         * modified, but never the number, nature or size of the dataHandlers.
         * Since this methods return references to the DataHandler, the
         * LearningAgent will assume that the referenced dataHandler are
         * automatically updated each time the doAction, or reset methods
         * are called on the LearningEnvironment.
         *
         * \return a vector of references to the DataHandler.
         */
        virtual std::vector<Data::DataView>  getDataSources() const = 0;

        /**
         * \brief Returns the current score of the Environment.
         *
         * The returned score will be used as a reward during the learning
         * phase of a LearningAgent.
         *
         * \return the current score for the LearningEnvironment.
         */
        virtual double getScore() const = 0;
    };
}; // namespace Learn

#endif
