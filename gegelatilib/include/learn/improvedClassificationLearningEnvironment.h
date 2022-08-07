#ifndef DICE_PROJECT_IMPROVEDCLASSIFICATIONLEARNINGENVIRONMENT_H
#define DICE_PROJECT_IMPROVEDCLASSIFICATIONLEARNINGENVIRONMENT_H

#include <gegelati.h>
#include <vector>

#include "learn/learningEnvironment.h"

namespace Learn {

    /**
     * \brief This enum manage the different types of algorithm that the classification is able to use
     */
    typedef enum LearningAlgorithm
    {
        DEFAULT, BRSS, FS, BANDIT, LEXICASE
    }LearningAlgorithm;

    /**
     * \brief The DS type is used to manage the dataset
     */
    using DS = std::pair<std::vector<std::vector<double>>,std::vector<double>>;

    /**
     * \brief Specialization of the LearningEnvironment class for classification
     * purposes.
     */
    class ImprovedClassificationLearningEnvironment : public LearningEnvironment
    {
      protected:
        /**
         * \brief 2D array storing for each class the guesses that were made by
         * the LearningAgent.
         *
         * For example classificationTable.at(x).at(y) represents the number of
         * times a LearningAgent guessed class y, for a data from class x since
         * the last reset.
         */
        std::vector<std::vector<uint64_t>> classificationTable;

        /**
         * \brief Class of the current data.
         *
         * This attribute should be updated alongside the data made avalaible
         * to the LearningAgent.
         */
        uint64_t currentClass;

        /**
         * \brief Current learning algorithm used
         */
        LearningAlgorithm currentAlgo;

        /**
         * \brief The dataset is the set of data that will be presented to the
         * agent for learning
         */
        DS * dataset;

        /**
         * \brief The datasubset is the set of data that will be presented to the
         * agent at each generation for learning
         */
        DS * datasubset;

        /**
         * \brief datasubsetSizeRatio is the ratio between the datasubset size
         * and the dataset size
         */
        float datasubsetSizeRatio;

        /**
         * \brief datasubsetRefreshRatio is the ratio between the number of
         * samples that will be refreshed at each generation and the datasubset
         * size
         */
        float datasubsetRefreshRatio;

        /**
         * \brief rng is used to keep predictability
         */
        Mutator::RNG rng;

        /**
         * \brief currentSampleIndex is the index of the current sample in the
         * datasubset
         */
        uint64_t currentSampleIndex;

        /**
          * \brief currentSample is the sample that will be presented to the agent
          * on this generation
         */
        Data::Array2DWrapper<double> currentSample;

      private:
        virtual double getScore_DEFAULT() const;
        virtual double getScore_BRSS() const;

        void refreshDatasubset_BRSS();

      public:
        /**
         * Main constructor of the ClassificationLearningEnvironment.
         *
         * \param[in] nbClass number of classes of the
         * classificationLearningEnvironment, and thus number of action of the
         * underlying LearningEnvironment.
         */
        ImprovedClassificationLearningEnvironment(uint64_t nbClass, LearningAlgorithm algo, uint64_t sampleSize)
            : LearningEnvironment(nbClass),
              classificationTable(nbClass, std::vector<uint64_t>(nbClass, 0)),
              currentClass{0}, currentAlgo(algo), currentSample(sampleSize, sampleSize)
        {
            this->datasubsetSizeRatio = 0.4;
            this->datasubsetRefreshRatio = 0.1;

            this->dataset = new DS();
            this->datasubset = new DS();
        };

        /**
         * \brief Get a const ref to the classification table of the learning
         * environment.
         */
        const std::vector<std::vector<uint64_t>>& getClassificationTable()
            const;

        /**
         * \brief Default implementation for the doAction method.
         *
         * This implementation only increments the classificationTable based on
         * the currentClass attribute. Refresh of the data should be implemented
         * by the child class, hence the pure virtual method.
         */
        virtual void doAction(uint64_t actionID) override = 0;

        /**
         * \brief Default scoring for classificationLearningEnvironment.
         *
         * The default scoring for the classificationLearningEnvironment is
         * computed based on the classificationTable attribute.
         *
         * The score represents the percentage of correct classification.
         */
        virtual double getScore() const override;

        /**
         * \brief Default implementation of the reset.
         *
         * Resets to zero the classificationTable.
         */
        virtual void reset(
            size_t seed = 0,
            LearningMode mode = LearningMode::TRAINING) override = 0;

        /**
         * \brief This method will refresh the datasubset according to the current
         * learning algorithm
         */
        void refreshDatasubset();

        /**
         * \brief This implementation will select the next current sample according
         * to the current learning mode
         */
        void changeCurrentSample(LearningMode mode);

        /**
         * \brief This implementation is used to modify the dataset (and will set
         * the datasubset equal to the dataset attribute)
         */
        void setDataset(DS * newDataset);

        /**
         * \brief This implementation is used to modify the current learning algorithm
         * that change the way the score is computed and the way the datasubset is
         * refreshed
         */
        void setAlgorithm(LearningAlgorithm algo);

        /**
         * \brief This implementation modify the datasubsetSizeRatio attributes
         *
         * It needs to be between 0 and 1
         */
        void setDatasubsetSizeRatio(float ratio);

        /**
         * \brief This implementation modify the datasubsetRefreshRatio attributes
         *
         * It needs to be between 0 and 1
         */
        void setDatasubsetRefreshRatio(float ratio);
    };
}; // namespace Learn

#endif //DICE_PROJECT_IMPROVEDCLASSIFICATIONLEARNINGENVIRONMENT_H
