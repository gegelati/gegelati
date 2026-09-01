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

#ifndef ARCHIVE_ENVIRONMENT_H
#define ARCHIVE_ENVIRONMENT_H
#if 0

#include <optional>

#include "evaluation/learningEnvironment.h"
#include "evaluation/scoreMetric.h"
#include "evaluation/archiveMetric.h"

#include "evolution/population.h"
#include "evolution/representation.h"

namespace Evaluation {


    /**
     * \brief Environment for the ArchiveEvalAgent
     * 
     * This environment is used to evaluate the member population of a hierarchical structure.
     * 
     * The Environment should contain a const reference of both the member and team populations.
     *  - The non-protected individuals of the team population are used to obtain the input of the archive based on the archiveMetric measured during their evaluation.
     *  - The protected individuals of the member population are used to compute the output of the archive.
     */
    class ArchiveEnvironment : public LearningEnvironment
    {
      protected:

        /// @brief Input used to give the input dimension of the archive.
        std::vector<std::reference_wrapper<const Data::DataHandler>> inputDimensions;

        /// @brief Number of input sampled stored in the archive.
        size_t archiveSize;

        /// @brief Random number generator for selecting archive inputs
        RNG::RNG rng;

        /// map of input and output pair values stored in the archive.
        std::vector<std::pair<std::vector<std::reference_wrapper<const Data::DataHandler>>, std::map<size_t, double>>> archive;

        /// The population in which individuals contains archive metrics used as input for the archive.
        std::optional<std::reference_wrapper<const Evolution::Population>> archiveInputPopulation;

        /// The population used to compute the output values of the archive.
        std::optional<std::reference_wrapper<const Evolution::Population>> archiveOutputPopulation;

        /**
         * \brief set the dimension of data source by creating a copy of the given one
         * 
         * \param[in] dHandler given dataSource.
         */
        void setDimensionDataSource(std::vector<std::reference_wrapper<const Data::DataHandler>> dHandler);

      public:
        /**
         * \brief Constructor for LearningEnviroment.
         *
         * \param[in] inputDimensions example inputs given as an example of dimension.
         * \param[in] archiveSize size of the archive
         */
        ArchiveEnvironment(std::vector<std::reference_wrapper<const Data::DataHandler>> inputDimensions, size_t archiveSize)
            : LearningEnvironment(Output::Output(-1.0, 1.0)), archiveSize{archiveSize} {
              this->setDimensionDataSource(inputDimensions);
            };

        /**
         * \brief inherrit from LearningEnvironment.
         */
        virtual std::vector<std::reference_wrapper<const Data::DataHandler>>
          getDataSources() const;

        /**
         * \brief set the population in which the individual provides the input of the archive.
         * 
         * \param[in] population the archive input propulation
         */
        virtual void setArchiveInputPopulation(const Evolution::Population& population);

        /**
         * Return if the current evaluation contains an archive input population
         */
        virtual bool hasArchiveInputPopulation();

        /**
         * \brief return the ArchiveInputPopulation of the current evaluation
         */
        virtual const std::optional<std::reference_wrapper<const Evolution::Population>>& getArchiveInputPopulation();

        /**
         * \brief set the population in which the individual provides the output of the archive.
         * 
         * \param[in] population the archive output propulation
         */
        virtual void setArchiveOutputPopulation(const Evolution::Population& population);

        /**
         * Return if the current evaluation contains an archive output population
         */
        virtual bool hasArchiveOutputPopulation();

        /**
         * \brief return the ArchiveOutputPopulation of the current evaluation
         */
        virtual const std::optional<std::reference_wrapper<const Evolution::Population>>& getArchiveOutputPopulation();


        /**
         * \brief get the current IDs stored in the archive
         */
        virtual std::set<size_t> getCurrentIDs() const;

        /**
         * \brief get the max size of the archive
         */
        virtual size_t getSize() const;

        /**
         * \brief get the current size of the archive
         */
        virtual size_t getCurrentSize() const;

        /**
         * \brief get the input of the specified index
         * 
         * \param[in] idx Index of the input
         */
        virtual const std::vector<std::reference_wrapper<const Data::DataHandler>>& getInput(size_t idx) const;

        /**
         * \brief get the output of the specified index and output
         * 
         * \param[in] idx Index of the input
         * \param[in] ID ID of the output
         */
        virtual double getOutput(size_t idx, size_t ID) const;

        /**
         * \brief Update the input used in the archive map based on the input population.
         * 
         * The population should belong to the evaluationAgent on which the archiveAgent is connected.
         * Only the EvaluationResults are used, which should contain archiveMetric.
         * 
         * Some archiveMetric are randomly sampled from the list.
         */
        virtual void updateArchiveInputs();

        /**
         * \brief Update the output used in the archive based on the individuals given.
         * 
         * Each individual is executed on each input, the output is saved in the archive.
         * 
         * \param[in] memberRepresentation the representation used to execute the individuals.
         */
        virtual void updateArchiveOutputs(
            const Evolution::Representation& memberRepresentation);

        /**
         * \brief boring override
         */
        virtual double getScore() const override {return 0.0;};
    };
}; // namespace Learn

#endif // ARCHIVE_ENVIRONMENT_H
#endif