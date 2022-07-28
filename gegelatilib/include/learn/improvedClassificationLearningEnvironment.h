#ifndef GEGELATI_IMPROVEDCLASSIFICATIONLEARNINGENVIRONMENT_H
#define GEGELATI_IMPROVEDCLASSIFICATIONLEARNINGENVIRONMENT_H

#include "gegelati.h"
#include "learn/learningEnvironment.h"

namespace Learn
{
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

    class ImprovedClassificationLearningEnvironment : public LearningEnvironment
    {
      protected:
        /// _classificationTable is the result of a training
        std::vector<std::vector<uint64_t>> _classificationTable;
        /// _currentClass is the current class that the environment is providing
        uint64_t _currentClass;
        /// _currentSampleIndex is the index in the datasubset of the curent sample
        uint64_t _currentSampleIndex;
        /// _sampleSize is the size of one data sample
        uint64_t _sampleSize;

        /// _dataset is the dataset
        mutable DS * _dataset;
        /// _datasubset is the datasubset
        mutable DS * _datasubset;

        /// _curentMode is the agent's learning mode
        Learn::LearningMode _currentMode;

        /// _rng is used to manage rng with predictability
        Mutator::RNG _rng;

        /// _currentSample is the current sample that is provided to the agent
        Data::Array2DWrapper<double> _currentSample;

        /// _datasubsetRefreshRatio is the ammount of the datasubset's samples wich will be refresh between each generations
        float _datasubsetRefreshRatio;
        /// _datasubsetSizeRatio is the size of the datasubset relative to the dataset's one
        float _datasubsetSizeRatio;

        /**
         * \brief This method is used to change the current datasubset's sample
         */
        void changeCurrentSample();

      private:
        void refreshDatasubset_BRSS(size_t seed);
        void refreshDatasubset_BANDIT(size_t seed);
        double getScore_DEFAULT() const;
        double getScore_BRSS() const;

      public:
        /**
         * \brief Constructor for the Learning Environment.
         *
         * @param nbClass The amount of classes the classification will use
         * @param sampleSize The size of one data sample
         */
        explicit ImprovedClassificationLearningEnvironment(uint64_t nbClass, uint64_t sampleSize);

        /**
         * \brief Default implementation for the doAction method.
         *
         * This implementation increments the classificationTable based on
         * the currentClass attribute and refresh the dataSample within the dataSet attribute.
         */
        void doAction(uint64_t actionID) override;

        /**
         * \brief This implementation returns the score based on the appropriate learning algorithm.
         *
         *  This learning algorithm is indicated in the class attributes.
         */
        virtual double getScore() const override;

        /**
         * \brief Default implementation of the reset.
         *
         * Resets to zero the classificationTable.
         */
        virtual void reset(size_t seed = 0, LearningMode mode = LearningMode::TRAINING) override;
        bool isTerminal() const override;

        /**
         * \brief This implementation refreshes the datasubset.
         *
         * @param algo is the type of LearningAlgorithm
         * @param seed is useful to keep control on randomness
         */
        void refreshDatasubset(LearningAlgorithm algo, size_t seed);

        // Getters and setters

        /**
         * \brief This method is used to change the datasubset, for it initialization for example
         * @param datasubset
         */
        void setDatasubset(DS * datasubset);
        /**
         * \brief This method is used to change the dataset, for it initialization for example
         * @param dataset
         */
        void setDataset(DS * dataset);
        /**
         * \brief This method is used to set the value of the RefreshRatio wich describes the ammount of datasubset's samples will be refresh between each generations
         * @param ratio
         */
        void setRefreshRatio(float ratio);

        /**
         * \brief Return the classification table
         */
        [[nodiscard]] const std::vector<std::vector<uint64_t>>& getClassificationTable() const;
        /**
         * Return the data source
         */
        std::vector<std::reference_wrapper<const Data::DataHandler>> getDataSources() override;
    };
}

#endif // GEGELATI_IMPROVEDCLASSIFICATIONLEARNINGENVIRONMENT_H
