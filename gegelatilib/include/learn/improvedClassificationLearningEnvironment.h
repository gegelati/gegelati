#ifndef GEGELATI_IMPROVEDCLASSIFICATIONLEARNINGENVIRONMENT_H
#define GEGELATI_IMPROVEDCLASSIFICATIONLEARNINGENVIRONMENT_H

#include "gegelati.h"
#include "learn/learningEnvironment.h"

namespace Learn
{
    typedef enum LearningAlgorithm
    {
        DEFAULT, BRSS, FS, BANDIT, LEXICASE
    }LearningAlgorithm;

    using DS = std::pair<std::vector<std::vector<double>>,std::vector<double>>;

    class ImprovedClassificationLearningEnvironment : public LearningEnvironment
    {
      protected:
        std::vector<std::vector<uint64_t>> _classificationTable;
        uint64_t _currentClass, _currentSampleIndex;
        mutable DS * _dataset, * _datasubset;
        Learn::LearningMode _currentMode;
        Mutator::RNG _rng;
        Data::Array2DWrapper<double> _currentSample;
        float _datasubsetRefreshRatio;

        void changeCurrentSample();

      private:
        void refreshDatasubset_BRSS(size_t seed);
        static void refreshDatasubset_BANDIT(size_t seed);

      public:
        /**
         * \brief Constructor for the Learning Environment.
         *
         * @param nbClass The amount of classes the classification will use
         */
        explicit ImprovedClassificationLearningEnvironment(uint64_t nbClass);

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
        [[nodiscard]] double getScore() const override;

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
        void setDatasubset(DS * datasubset);
        void setDataset(DS * dataset);
        void setRefreshRatio(float ratio);

        [[nodiscard]] const std::vector<std::vector<uint64_t>>& getClassificationTable() const;
        std::vector<std::reference_wrapper<const Data::DataHandler>> getDataSources() override;
    };
}

#endif // GEGELATI_IMPROVEDCLASSIFICATIONLEARNINGENVIRONMENT_H
