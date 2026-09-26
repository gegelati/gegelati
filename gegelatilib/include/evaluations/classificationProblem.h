/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2025) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2022)
 * Nicolas Sourbier <nsourbie@insa-rennes.fr> (2019 - 2020)
 * Pierre-Yves Le Rolland-Raumer <plerolla@insa-rennes.fr> (2020)
 * Quentin Vacher <qvacher@insa-rennes.fr> (2023 - 2025)
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

#ifndef CLASSIFICATION_PROBLEM_H
#define CLASSIFICATION_PROBLEM_H

#include "evaluations/problem.h"

namespace Evaluations {

    /**
     * \brief Class used to control the learning steps of a Graph within
     * a given Problem.
     */
    class ClassificationProblem : public Problem
    {
      protected:
        /// Number of class in the dataset
        size_t nbClass;
      
        /// Dataset used for the classification learning task.
        std::vector<std::pair<std::vector<Data::DataValue>, size_t>> dataset; 
      public:
        /**
         * \brief Constructor for Problem.
         * 
         * \param[in] inputDimensions the dimensions of the input sources.
         * \param[in] nbClass the number of class in the dataset.
         * \param[in] dataset The dataset.
         * \param[in] problemSeed the seed of the problem.
         */
        ClassificationProblem(
            const std::vector<Dimensions::Requirement>& inputDimensions, size_t nbClass, 
            std::vector<std::pair<std::vector<Data::DataValue>, size_t>> dataset, uint64_t problemSeed = 0) 
            : Problem(inputDimensions, Dimensions::Requirement::scalar<size_t>(Dimensions::NumericRange<size_t>(0, nbClass-1)), problemSeed), 
              nbClass{nbClass}, dataset{std::move(dataset)} {};

        /// Default destructor for polymorphism
        virtual ~ClassificationProblem() = default;

        /**
         * \brief Return the dataView at the specified index of the dataset.
         */
        std::vector<Data::DataView> getDataViewAt(size_t index) const;
        
        /**
         * \brief Return the target at the specified index of the dataset.
         */
        const size_t& getTargetAt(size_t index) const;
        
        /**
         * \brief Override of Problem method to set the maximum hash to the size of the dataset.
         */
        virtual uint64_t maxHash() const override;

        /**
         * \brief TODO
         */
        virtual void extractFeatures(
            const Individual& individual,
            const std::map<size_t, std::unique_ptr<Feature>>& features,
            const std::set<uint64_t>& hashes) const override;
    };
}; // namespace Learn

#endif // CLASSIFICATION_PROBLEM_H
